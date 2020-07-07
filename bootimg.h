#ifndef __BOOTIMG_H
#define __BOOTIMG_H

/*
Thanks to https://forum.xda-developers.com/showthread.php?t=2319018
*/

/*
boot.img/recovery.img format:

BOOT HEADER -- 1 page
KERNEL IMAGE -- N pages
RAMDISK IMAGE -- M pages
SECONDARY BOOTLOADER -- X pages
Binary Device Tree -- Y pages (Samsung devices only?)
*/

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512

/* In little endian??? */
struct boot_img_hdr
{
	/* "ANDROID!" */
	unsigned char magic[BOOT_MAGIC_SIZE];

	/* Kernel image. zImage? */
	unsigned kernel_size;	/* Size in bytes */
	unsigned kernel_addr;	/* Physical load addr */

	/* Ramdisk image. Cpio archive, zipped by gzip or
	lzma4-zip. Use file util to determine which one is used.*/
	unsigned ramdisk_size;	/* Size in bytes */
	unsigned ramdisk_addr;	/* Physical load addr */

	/* Secondary bootloader. Never used? */
	unsigned second_size;	/* Size in bytes */
	unsigned second_addr;	/* Physical load addr */

	/* Physical addr for kernel tags. We don't know what is it. */
	unsigned tags_addr;

	/* Flash page size we assume. Usually 2048. */
	unsigned page_size;

	unsigned unused;	/* Samsung only: Device tree size */

	unsigned os_version;	/* ??? */

	/* ASCIIZ product name. Usually empty. */
	unsigned char name[BOOT_NAME_SIZE];

	/* Whitespace separated kernel cmdline args. Null terminated. */
	unsigned char cmdline[BOOT_ARGS_SIZE];

	/* Here timestamp or SHA1 checksum of kernel, ramdisk, second
	sections (and possible dtb section) is written. Bootloader seems
	not to check this filed. */
	unsigned id[8];
} __attribute__ (( packed ));

#endif
