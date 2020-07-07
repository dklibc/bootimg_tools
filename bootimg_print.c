/*
 * Print Android boot.img/recovery.img boot header.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bootimg.h"

int main(int argc, char *argv[])
{
	FILE *fp;
	struct boot_img_hdr hdr;
	int r = -1;

	if (argc != 2) {
		fprintf(stderr, "Invalid args. Usage: $ bootimg_print image-file\n");
		return -1;
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "Failed to open '%s': %s\n", argv[1],
			strerror(errno));
		return -1;
	}

	if (fread(&hdr, sizeof(hdr), 1, fp) != 1) {
		fprintf(stderr, "Failed to read boot header: %s\n",
			strerror(errno));
		return -1;
	}

	if (memcmp(hdr.magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
		fprintf(stderr, "Missing boot header magic\n");
		return -1;
	}

	if (!hdr.kernel_size)
		fprintf(stderr, "Kernel size = 0\n");

	if (!hdr.ramdisk_size)
		fprintf(stderr, "Ramdisk size = 0\n");

	printf(
		"\n***Boot header" \
		"\nkernel-size : %d" \
		"\nkernel-addr : 0x%08x" \
		"\nramdisk-size: %d" \
		"\nramdisk-addr: 0x%08x" \
		"\nsecond-size : %d" \
		"\nsecond-addr : 0x%08x" \
		"\ntags-addr   : 0x%02x" \
		"\npage-size   : %d" \
		"\nunused      : 0x%08x" \
		"\nos-version  : 0x%08x" \
		"\nname        : %s" \
		"\ncmdline     : %s" \
		"\nid          : 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x" \
		"\n\n", hdr.kernel_size, hdr.kernel_addr,
		hdr.ramdisk_size, hdr.ramdisk_addr,
		hdr.second_size, hdr.second_addr,
		hdr.tags_addr, hdr.page_size,
		hdr.unused, hdr.os_version,
		hdr.name, hdr.cmdline,
		hdr.id[0], hdr.id[1], hdr.id[2], hdr.id[3],
		hdr.id[4], hdr.id[5], hdr.id[6], hdr.id[7]
	);

	fclose(fp);
	return 0;
}
