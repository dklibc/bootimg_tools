/*
 * Combine Android boot.img/recovery.img from boothdr.img, kernel.img,
 * recovery.img and optional dtb.img. Writes to result.img.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bootimg.h"

/* Return size of file */
static int do_part(const char *name, int page_sz, FILE *ofp)
{
	FILE *ifp;
	int total, left, r = -1, n;
	char buf[4096];
	long p;

	ifp = fopen(name, "r");
	if (!ifp) {
		fprintf(stderr, "Failed to open '%s': %s\n", name,
			strerror(errno));
		return -1;
	}

	total = 0;
	while (1) {
		n = fread(buf, 1, sizeof(buf), ifp);
		if (n < 0) {
			fprintf(stderr, "Failed to read '%s': %s\n",
				name, strerror(errno));
			goto fin;
		}

		if (n == 0)
			break;

		if (fwrite(buf, n, 1, ofp) != 1) {
			fprintf(stderr, "Failed to write '%s': %s\n",
				name, strerror(errno));
			goto fin;
		}

		total += n;
	}


	n = total % page_sz;
	left = n ? page_sz - n : 0;
	memset(buf, 0, sizeof(buf));
	while (left > 0) {
		n = fwrite(buf, 1, (left < sizeof(buf)) ? left :
			   sizeof(buf), ofp);
		if (n < 0) {
			fprintf(stderr, "Failed to write '%s': %s\n",
				name, strerror(errno));
			goto fin;
		}
		left -= n;
	}

	printf("Read %-14s: %d bytes\n", name, total);

	r = total;

fin:
	fclose(ifp);
	return r;
}

static int do_hdr(struct boot_img_hdr *hdr, FILE *ofp)
{
	const char *name = "boothdr.img";
	FILE *ifp;

	ifp = fopen(name, "r");
	if (!ifp) {
		fprintf(stderr, "Failed to open %s: %s\n", name,
			strerror(errno));
		return -1;
	}

	if (fread(hdr, sizeof(*hdr), 1, ifp) != 1) {
		fprintf(stderr, "Failed to read %s: %s\n",
			name, strerror(errno));
		fclose(ifp);
		return -1;
	}

	fclose(ifp);

	if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
		fprintf(stderr, "Missing boot header magic\n");
		return -1;
	}

	if (hdr->second_size) {
		fprintf(stderr, "Second size != 0\n");
		return -1;
	}

	return do_part(name, hdr->page_size, ofp);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	struct boot_img_hdr hdr;
	int r = -1, samsung = 0;
	const char *name = "result.img";

	if (argc == 2) {
		if (strcmp(argv[1], "-samsung")) {
inval_args:
			fprintf(stderr, "Usage: $ bootimg_combine [-samsung]\n");
			return -1;
		}

		samsung = 1;
	} else if (argc != 1){
		goto inval_args;
	}

	fp = fopen(name, "w");
	if (!fp) {
		fprintf(stderr, "Failed to create %s: %s\n", name,
			strerror(errno));
		return -1;
	}

	if (do_hdr(&hdr, fp) <= 0)
		goto fin;

	hdr.kernel_size = do_part("kernel.img", hdr.page_size, fp);
	if (hdr.kernel_size <= 0)
		goto fin;

	hdr.ramdisk_size = do_part("ramdisk.img", hdr.page_size, fp);
	if (hdr.ramdisk_size <= 0)
		goto fin;

	if (samsung) {
		hdr.unused = do_part("dtb.img", hdr.page_size, fp);
		if (hdr.unused <= 0)
			goto fin;
	}

	if (fseek(fp, 0, SEEK_SET)) {
		fprintf(stderr, "Failed to seek to the beginning of %s: %s\n",
			name, strerror(errno));
		goto fin;
	}

	if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1) {
		fprintf(stderr, "Failed to write %s: %s\n", name, strerror(errno));
		goto fin;
	}

	printf("Done! Result is %s\n", name);

	r = 0;
fin:
	fclose(fp);
	return r;
}
