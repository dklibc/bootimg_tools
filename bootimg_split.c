/*
 * Split Android boot.img/recovery.img into
 * * boothdr.img,
 * * kernel.img,
 * * ramdisk.img,
 * * dtb.img (Device Tree. Only for Samsung images)
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bootimg.h"

static int do_part(const char *name, int sz, int page_sz, FILE *fp)
{
	FILE *fp2;
	int left, r = -1, n;
	char buf[2048];

	if (sz <= 0)
		return sz;

	fp2 = fopen(name, "w");
	if (!fp2) {
		fprintf(stderr, "Failed to create '%s': %s\n", name,
			strerror(errno));
		return -1;
	}

	left = ((sz + page_sz - 1) / page_sz) * page_sz;
	while (left > 0) {
		n = (sizeof(buf) < left) ? sizeof(buf) : left;
		if (fread(buf, n, 1, fp) != 1) {
			fprintf(stderr, "Failed to read '%s': %s\n",
				name, strerror(errno));
			goto fin;
		}

		left -= n;

		if (left <= 0) {
			n = sz % sizeof(buf);
			if (!n)
				n = sizeof(buf);
		}

		if (fwrite(buf, n, 1, fp2) != 1) {
			fprintf(stderr, "Failed to write '%s': %s\n",
				name, strerror(errno));
			goto fin;
		}
	}

	printf("Written %d bytes to '%s'\n", sz, name);

	r = 0;

fin:
	fclose(fp2);
	return r;
}

static int do_hdr(struct boot_img_hdr *hdr, FILE *fp)
{
	int n;

	n = fread(hdr, sizeof(*hdr), 1, fp);
	if (n != 1) {
		fprintf(stderr, "Failed to read boot header: %s\n",
			strerror(errno));
		return -1;
	}

	if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
		fprintf(stderr, "Missing boot header magic\n");
		return -1;
	}

	if (!hdr->kernel_size) {
		fprintf(stderr, "Kernel size = 0\n");
		return -1;
	}

	if (!hdr->ramdisk_size) {
		fprintf(stderr, "Ramdisk size = 0\n");
		return -1;
	}

	if (fseek(fp, 0, SEEK_SET)) {
		fprintf(stderr, "Failed to skip the beginning of file: %s\n",
			strerror(errno));
		return -1;
	}

	/*
	 * Save to file the whole boot hdr page since there are
	 * several versions of the header, so there can be additional
	 * fields we don't know about, but they can be useful for
	 * combiner.
	 */
	return do_part("boothdr.img", hdr->page_size, hdr->page_size,
		       fp);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	struct boot_img_hdr hdr;
	int r = -1, samsung = 0;
	const char *name;

	if (argc < 2 || argc > 3) {
inval_args:
		fprintf(stderr, "Invalid args. Usage: $ bootimg_split [-samsung] image-file\n");
		return -1;
	}

	name = argv[1];

	if (argc == 3) {
		if (strcmp(argv[1], "-samsung"))
			goto inval_args;
		samsung = 1;
		name = argv[2];
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "Failed to open '%s': %s\n", name,
			strerror(errno));
		return -1;
	}

	if (do_hdr(&hdr, fp))
		goto fin;

	if (do_part("kernel.img", hdr.kernel_size, hdr.page_size, fp))
		goto fin;

	if (do_part("ramdisk.img", hdr.ramdisk_size, hdr.page_size, fp))
		goto fin;

	if (hdr.second_size)
		fprintf(stderr, "WARNING: Second size != 0\n");

	if (samsung && !hdr.unused) {
		fprintf(stderr, "WARNING: Device tree size = 0\n");
	} else {
		if (do_part("dtb.img", hdr.unused, hdr.page_size, fp))
			goto fin;
	}

	printf("Done!\n");

	r = 0;
fin:
	fclose(fp);
	return r;
}
