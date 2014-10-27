/*******************************************************************************
 * somagic-extract-firmware.c                                                  *
 *                                                                             *
 * Extract the EasyCAP Somagic firmware from a Windows driver file.            *
 * *****************************************************************************
 *
 * Copyright 2011-2013 Jeffry Johnston
 *
 * This file is part of somagic_easycap
 * http://code.google.com/p/easycap-somagic-linux/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <errno.h>
#include <gcrypt.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

/* Constants */
#define PROGRAM_NAME "somagic-extract-firmware"
#define VERSION "1.1"
#define SOMAGIC_FIRMWARE_PATH "/lib/firmware/somagic_firmware.bin"

/*
 * Index  Firmware
 * -----  --------
 *     0  SmiUsbGrabber3C.sys, EasyCAP DC60
 *     1  SmiUsbGrabber3E.sys, EasyCAP002
 *     2  SmiUsbGrabber3F.sys, EasyCAP002
 */

/*
 *                               File    Initialized  62-byte  Firmware                                   Offset
 * Firmware Filename             Size    Device       blocks   Length    Firmware Offsets                 Difference
 * ----------------------------  ------  -----------  -------  --------  -------------------------------  ----------
 * vista/SmiUsbGrabber3C.sys     805888  1c88:003c    121      7502      0xbb0d8 0xbce28 0xbeb78 0xc08c8  7504
 * vista64/SmiUsbGrabber3C.sys   821888  1c88:003c    121      7502      0x12aa0 0x147f0 0x16540 0x18290  7504
 * wind7_32/SmiUsbGrabber3C.sys  805888  1c88:003c    121      7502      0xbb0d8 0xbce28 0xbeb78 0xc08c8  7504
 * wind7_64/SmiUsbGrabber3C.sys  821888  1c88:003c    121      7502      0x12aa0 0x147f0 0x16540 0x18290  7504
 * xp/SmiUsbGrabber3C.sys        805632  1c88:003c    121      7502      0xbafd8 0xbcd28 0xbea78 0xc07c8  7504
 * xp/SmiUsbGrabber3E.sys        805376  1c88:003e    107      6634      0xbac48 0xbc628 0xbe008 0xbf9e8  6624
 * vista/SmiUsbGrabber3F.sys     109568  1c88:003f    107      6634      0x10b28 0x12508 0x13ee8 0x158c8  6624
 * vista64/SmiUsbGrabber3F.sys   127872  1c88:003f    107      6634      0x14270 0x15c50 0x17630 0x19010  6624
 * wind7_32/SmiUsbGrabber3F.sys  109568  1c88:003f    107      6634      0x10b28 0x12508 0x13ee8 0x158c8  6624
 * wind7_64/SmiUsbGrabber3F.sys  127872  1c88:003f    107      6634      0x14270 0x15c50 0x17630 0x19010  6624
 * xp/SmiUsbGrabber3F.sys        109568  1c88:003f    107      6634      0x10b28 0x12508 0x13ee8 0x158c8  6624
 * xp/SmiUsbGrabber3F.sys        147328  1c88:003f    107      6634      0x18570 0x19f50 0x1b930 0x1d310  6624
 */
#define PRODUCT_COUNT 3
static const int SOMAGIC_FIRMWARE_LENGTH[PRODUCT_COUNT] = {
	7502,
	6634,
	6634
};
static const unsigned char SOMAGIC_FIRMWARE_MAGIC[PRODUCT_COUNT][4] = {
	{'\x0c', '\x94', '\xce', '\x00'},
	{'\x0c', '\x94', '\xcc', '\x00'},
	{'\x0c', '\x94', '\xcc', '\x00'}
};
static const unsigned char SOMAGIC_FIRMWARE_CRC32[PRODUCT_COUNT][4] = {
	{'\x34', '\x89', '\xf7', '\x7b'},
	{'\x1f', '\xfe', '\xde', '\xbb'},
	{'\x60', '\x1d', '\x37', '\x5f'}
};

static void version()
{
	fprintf(stderr, PROGRAM_NAME" "VERSION"\n");
	fprintf(stderr, "Copyright 2011-2013 Jeffry Johnston\n");
	fprintf(stderr, "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.\n");
	fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
	fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
}

static void usage()
{
	fprintf(stderr, "Usage: "PROGRAM_NAME" [options] DRIVER_FILENAME\n");
	fprintf(stderr, "  -f, --firmware=FILENAME  Write to firmware file FILENAME\n");
	fprintf(stderr, "                           (default: "SOMAGIC_FIRMWARE_PATH")\n");
	fprintf(stderr, "  -v, --verbose            Verbose output\n");
	fprintf(stderr, "      --help               Display usage\n");
	fprintf(stderr, "      --version            Display version information\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Example (run as root):\n");
	fprintf(stderr, PROGRAM_NAME" SmiUsbGrabber3C.sys\n");
}

int main(int argc, char **argv)
{
	FILE *infile;
	int ret;
	char *firmware_path = SOMAGIC_FIRMWARE_PATH;
	int verbose = 0;
	unsigned char last4[4] = {'\0', '\0', '\0', '\0'};
	int firmware_found = 0;
	long pos;
	char firmware[SOMAGIC_FIRMWARE_LENGTH[0]];
	unsigned char digest[4];
	FILE *outfile;
	int i;

	/* Parsing */
	int c;
	int option_index = 0;
	static struct option long_options[] = {
		{"help", 0, 0, 0},     /* index 0 */
		{"verbose", 0, 0, 'v'},/* index 1 */
		{"version", 0, 0, 0},  /* index 2 */
		{"firmware", 1, 0, 'f'},
		{0, 0, 0, 0}
	};

	/* Parse command line arguments */
	while (1) {
		c = getopt_long(argc, argv, "f:v", long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 0:
			switch (option_index) {
			case 0: /* --help */
				usage();
				return 0;
			case 1: /* --verbose */
				verbose = 1;
				break;
			case 2: /* --version */
				version();
				return 0;
			default:
				usage();
				return 1;
			}
			break;
		case 'f':
			firmware_path = optarg;
			break;
		default:
			usage();
			return 1;
		}
	}
	if (optind + 1 != argc) {
		usage();
		return 1;
	}

	infile = fopen(argv[optind], "r");
	if (infile == NULL) {
		fprintf(stderr, "%s: Failed to open driver file '%s': %s\n", argv[0], argv[optind], strerror(errno));
		return 1;
	}

	/* Search for the firmware magic */
	while (!firmware_found) {
		/* Read next byte from file */
		c = fgetc(infile);
		if (c == EOF) {
			/* Either at EOF or a read error occurred */
			if (!feof(infile)) {
				perror("Failed to read driver file");
				return 1;
			}
			break;
		}

		/* Roll new character into array */
		memmove(last4, last4 + 1, 3);
		last4[3] = c;

		/* Check firmware magic */
		for (i = 0; i < PRODUCT_COUNT; i++) {
			if (memcmp(last4, SOMAGIC_FIRMWARE_MAGIC[i], 4) == 0) {
				/* Found, save file position */
				pos = ftell(infile);

				/* Read rest of firmware */
				memcpy(firmware, last4, 4);
				ret = fread(firmware + 4, 1, SOMAGIC_FIRMWARE_LENGTH[i] - 4, infile);
				if (ret != SOMAGIC_FIRMWARE_LENGTH[i] - 4) {
					perror("Failed to read driver file");
					return 1;
				}

				/* Check CRC32 */
				gcry_md_hash_buffer(GCRY_MD_CRC32, digest, firmware, SOMAGIC_FIRMWARE_LENGTH[i]);
				if (verbose) {
					fprintf(stderr, "Product: %i, Expected: %02x %02x %02x %02x, Found: %02x %02x %02x %02x, Offset: %lx\n", i, SOMAGIC_FIRMWARE_CRC32[i][0], SOMAGIC_FIRMWARE_CRC32[i][1], SOMAGIC_FIRMWARE_CRC32[i][2], SOMAGIC_FIRMWARE_CRC32[i][3], digest[0], digest[1], digest[2], digest[3], (pos - 4));
				}
				if (memcmp(digest, SOMAGIC_FIRMWARE_CRC32[i], 4) == 0) {
					/* CRC32 matched */
					firmware_found = 1;

					/* Write firmware file */
					outfile = fopen(firmware_path, "w+");
					if (outfile == NULL) {
						fprintf(stderr, "%s: Failed to open firmware file '%s': %s\n", argv[0], firmware_path, strerror(errno));
						return 1;
					}
					ret = fwrite(firmware, 1, SOMAGIC_FIRMWARE_LENGTH[i], outfile);
					if (ret != SOMAGIC_FIRMWARE_LENGTH[i]) {
						perror("Failed to write firmware file");
						return 1;
					}
					ret = fclose(outfile);
					if (ret) {
						perror("Failed to close firmware file");
						return 1;
					}
			  		fprintf(stderr, "Firmware written to '%s'.\n", firmware_path);
					break;
				} else {
					/* False positive, return to previous file position and keep looking */
					ret = fseek(infile, pos, SEEK_SET);
					if (ret) {
						perror("Failed to seek in driver file");
						return 1;
					}
				}
			}
		}
	}

	ret = fclose(infile);
	if (ret) {
		perror("Failed to close driver file");
		return 1;
	}

	if (!firmware_found) {
  		fprintf(stderr, "Somagic firmware was not found in driver file.\n");
		return 1;
	}

	return 0;
}

