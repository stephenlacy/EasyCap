/*******************************************************************************
 * somagic-audio-capture.c                                                     *
 *                                                                             *
 * USB Driver for Somagic EasyCAP DC60                                         *
 * USB ID 1c88:0007                                                            *
 *                                                                             *
 * Initializes the Somagic EasyCAP DC60 registers and performs audio capture.  *
 * *****************************************************************************
 *
 * Copyright 2011 Tony Brown, Jeffry Johnston, Jon Arne Jørgensen
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

/*
 * I have not implemented any buffer, so you will need to start the capture into a file.
 * And then start to play from that file a bit later, so the file acts as a buffer!
 *
 * Usage:
 * init
 * audio-capture > output.raw &
 * mplayer output.raw -demuxer +rawaudio -rawaudio "channels=2:samplesize=4:rate=48000"
 *
 */

/* This file was originally generated with usbsnoop2libusb.pl from a usbsnoop log file. */
/* Latest version of the script should be in http://iki.fi/lindi/usb/usbsnoop2libusb.pl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <libusb-1.0/libusb.h>
#ifdef DEBUG
#include <execinfo.h>
#endif
#include <unistd.h>

#define VENDOR 0x1c88
#define PRODUCT 0x003c
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

struct libusb_device_handle *devh;

void release_usb_device(int ret)
{
	fprintf(stderr, "Emergency exit\n");
	ret = libusb_release_interface(devh, 0);
	if (!ret) {
		perror("Failed to release interface");
	}
	libusb_close(devh);
	libusb_exit(NULL);
	exit(1);
}

struct libusb_device *find_device(int vendor, int product)
{
	struct libusb_device **list;
	struct libusb_device *dev = NULL;
	struct libusb_device_descriptor descriptor;
	int i;
	ssize_t count;
	count = libusb_get_device_list(NULL, &list);
	for (i = 0; i < count; i++) {
		struct libusb_device *item = list[i];
		libusb_get_device_descriptor(item, &descriptor);
		if (descriptor.idVendor == vendor && descriptor.idProduct == product) {
			dev = item;
		} else {
			libusb_unref_device(item);
		}
	}
	libusb_free_device_list(list, 0);
	return dev;
}

void print_bytes(unsigned char *bytes, int len)
{
	int i;
	if (len > 0) {
		for (i = 0; i < len; i++) {
			fprintf(stderr, "%02x ", (int)bytes[i]);
		}
		fprintf(stderr, "\"");
		for (i = 0; i < len; i++) {
			fprintf(stderr, "%c", isprint(bytes[i]) ? bytes[i] : '.');
		}
		fprintf(stderr, "\"");
	}
}

int pcount = 0;
const int FCOUNT = 800000;

void gotdata(struct libusb_transfer *tfr)
{
	int ret;
	int num = tfr->num_iso_packets;
	int i;

	for (i = 0; i < num; i++) {
		write(1, libusb_get_iso_packet_buffer_simple(tfr, i), tfr->iso_packet_desc[i].actual_length);
	}

	//	fprintf(stderr, "id %d got %d pkts of length %d. calc=%d, total=%d (%04x)\n", pcount, num, length, num*length, total, total);
	pcount++;

	if (pcount >= 0) {
		/* find_sync(data, num * length); */
		//init_buffer(data, num * length);
		/* init_buffer(data, total); */
		//process_data();
		/*
		fprintf(stderr, "write\n");
		write(1, data, total);
		write(1,"----------------",16);
		*/
	}

	if (pcount <= FCOUNT - 4) {
		/* fprintf(stderr, "resubmit id %d\n", pcount - 1); */
		ret = libusb_submit_transfer(tfr);
		if (ret != 0) {
			fprintf(stderr, "libusb_submit_transfer failed with error %d\n", ret);
			exit(1);
		}
	}
}

int main()
{
	int ret;
	struct libusb_device *dev;
	unsigned char buf[65535];
	struct libusb_transfer *tfr0;
	struct libusb_transfer *tfr1;
	unsigned char isobuf0[32 * 1008];
	unsigned char isobuf1[32 * 1008];

	libusb_init(NULL);
	libusb_set_debug(NULL, 0);

	dev = find_device(VENDOR, PRODUCT);
	assert(dev);

	ret = libusb_open(dev, &devh);
	libusb_unref_device(dev);
	assert(ret == 0);
	
	signal(SIGTERM, release_usb_device);
	ret = libusb_claim_interface(devh, 0);
	if (ret != 0) {
		fprintf(stderr, "claim failed with error %d\n", ret);
		exit(1);
	}
	
	ret = libusb_set_interface_alt_setting(devh, 0, 0);
	if (ret != 0) {
		fprintf(stderr, "set_interface_alt_setting failed with error %d\n", ret);
		exit(1);
	}

	ret = libusb_get_descriptor(devh, 0x0000001, 0x0000000, buf, 0x0000012);
	fprintf(stderr, "1 get descriptor returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	ret = libusb_get_descriptor(devh, 0x0000002, 0x0000000, buf, 0x0000009);
	fprintf(stderr, "2 get descriptor returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	ret = libusb_get_descriptor(devh, 0x0000002, 0x0000000, buf, 0x0000042);
	fprintf(stderr, "3 get descriptor returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");

	ret = libusb_release_interface(devh, 0);
	if (ret != 0) {
		fprintf(stderr, "failed to release interface before set_configuration: %d\n", ret);
	}
	ret = libusb_set_configuration(devh, 0x0000001);
	fprintf(stderr, "4 set configuration returned %d\n", ret);
	ret = libusb_claim_interface(devh, 0);
	if (ret != 0) {
		fprintf(stderr, "claim after set_configuration failed with error %d\n", ret);
	}
	ret = libusb_set_interface_alt_setting(devh, 0, 0);
	fprintf(stderr, "4 set alternate setting returned %d\n", ret);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, 0x0000001, 0x0000001, 0x0000000, buf, 0x0000002, 1000);
	fprintf(stderr, "5 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");

	// Need to set 0x3a => 0x80 for stereo audio output
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x3a\x80", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "6 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x3b\x00", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "7 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");

/*
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x34\x01", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "8 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x35\x00", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "9 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
*/
/*
	memcpy(buf, "\x0b\x00\x20\x82\x01\x30\x80\x80", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "10 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "11 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
*/
/*
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x34\xff", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "12 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x35\xff", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "13 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
*/
/*
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x3a\x80", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "14 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x3b\x80", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "15 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
*/
/*
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x3a\xff", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "16 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x00\x82\x01\x00\x3b\x80", 0x0000008); // 1101
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "17 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
*/

/*
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x01\x08\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "96 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x02\xc7\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "97 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x03\x33\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "98 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x04\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "99 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x05\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "100 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x06\xe9\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "101 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x07\x0d\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "102 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 08h: d8 or 98 = auto NTSC/PAL, 58 = force NTSC, 18 = force PAL
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x08\x98\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "103 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x09\x01\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "104 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0a\x80\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "105 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0b\x40\xff", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "106 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0c\x40\xff", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "107 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0d\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "108 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "109 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "110 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "111 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "112 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "113 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "114 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "115 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "116 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "117 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x81\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "118 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0f\x2a\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "119 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x10\x40\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "120 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x11\x0c\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "121 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x12\x01\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "122 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x13\x80\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "123 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x14\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "124 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x15\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "125 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x16\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "126 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x17\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "127 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x40\x02\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "128 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x58\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "129 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 59h: HOFF - horizontal offset 
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x59\x54\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "130 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 5Ah: VOFF - vertical offset 
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x5a\x07\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "131 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 5Bh: HVOFF - horizontal and vertical offset bits 
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x5b\x03\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "132 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 5Eh: SDID codes. 0 = sets SDID5 to SDID0 active 
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x5e\x00\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "133 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x02\xc0\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "134 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0e\x01\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "141 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 40h: 00 for PAL, 80 for NTSC 
	if (tv_standard == PAL) {
		memcpy(buf, "\x0b\x4a\x84\x00\x01\x40\x00\x00", 0x0000008);
	} else {
		memcpy(buf, "\x0b\x4a\x84\x00\x01\x40\x80\xf4", 0x0000008);
	}
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "142 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xa0\x00\x01\x86\x30\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "143 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xa0\x00\x01\x00\x30\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "144 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\x84\x00\x01\x5b\x30\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "145 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xa0\x00\x01\xf3\x30\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "146 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xa0\x00\x01\x00\x30\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "147 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\x84\x00\x01\x10\x30\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "148 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xa0\x00\x01\xc4\x30\xf4", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "149 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 40h: 00 for PAL, 80 for NTSC
	if (tv_standard == NTSC) {
		memcpy(buf, "\x0b\x4a\xa0\x00\x01\x40\x80\xf4", 0x0000008);
	}
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "150 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	// Subaddress 5Ah: 07 for PAL, 0a for NTSC
	if (tv_standard == PAL) {
		memcpy(buf, "\x0b\x4a\xc0\x01\x01\x5a\x07\x00", 0x0000008);
	} else {
		memcpy(buf, "\x0b\x4a\xc0\x01\x01\x5a\x0a\x86", 0x0000008);
	}
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "151 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x59\x54\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "152 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x5b\x83\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "153 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x10\x40\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "154 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x55\xff\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "154a control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x41\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "155 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x42\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "156 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x43\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "157 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x44\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "158 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x45\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "159 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x46\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "160 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x47\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "161 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x48\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "162 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x49\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "163 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x4a\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "164 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x4b\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "165 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x4c\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "166 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x4d\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "167 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x4e\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "168 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x4f\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "169 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x50\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "170 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x51\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "171 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x52\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "172 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x53\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "173 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x54\x77\x86", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "174 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0a\x80\x01", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "176 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0b\x40\x01", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "177 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0d\x00\x01", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "178 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x0c\x40\x01", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "179 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x4a\xc0\x01\x01\x09\x01\x00", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "180 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
*/
	memcpy(buf, "\x0b\x00\x00\x82\x01\x17\x40\x00", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "183 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x20\x82\x01\x30\x80\x1b", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "184 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "185 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x00\x82\x01\x17\x40\x00", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "186 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");

	usleep(250 * 1000);
	memcpy(buf, "\x0b\x00\x00\x82\x01\x17\x40\x00", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "187 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	memcpy(buf, "\x0b\x00\x20\x82\x01\x30\x80\x10", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "188 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "189 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");

	memcpy(buf, "\x01\x02", 0x0000002);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x0000001, 0x0000000, buf, 0x0000002, 1000);
	fprintf(stderr, "190 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	ret = libusb_get_descriptor(devh, 0x0000002, 0x0000000, buf, 0x0000109);
	fprintf(stderr, "191 get descriptor returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");
	ret = libusb_set_interface_alt_setting(devh, 0, 1);
	fprintf(stderr, "192 set alternate setting returned %d\n", ret);

 	// 0x1740 Controls audio!
 	//
 	// Bit 1 Toggles left/right channel
 	// Bit 0 & 2 Must be on for audio!
 	//
 	// 0001 1101 (0x1d) = 32Bit Little Endian @ 48,000 hZ 
 	// 0001 1111 (0x1f) = 32Bit Little Endian @ 48,000 hz - Need to remove the 2 first bytes to get any sensible playback!
 	// 0001 0101 (0x15) = 32Bit Little Endian @ 48,000 hz - Need to remove the 2 first bytes to get any sensible playback!
 	// 0000 1101 (0x0d) = 32Bit Little Endian @ 48,000 hz - Need to remove the 2 first bytes to get any sensible playback!
	memcpy(buf, "\x0b\x00\x00\x82\x01\x17\x40\x1d", 0x0000008);
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "193 control msg returned %d, bytes: ", ret);
	print_bytes(buf, ret);
	fprintf(stderr, "\n");

	usleep(30 * 1000);

	tfr0 = libusb_alloc_transfer(32);
	assert(tfr0 != NULL);
	libusb_fill_iso_transfer(tfr0, devh, 0x00000082, isobuf0, 32 * 1008, 32, gotdata, NULL, 2000);
	libusb_set_iso_packet_lengths(tfr0, 1008);

	ret = libusb_submit_transfer(tfr0);
	if (ret != 0) {
		fprintf(stderr, "libusb_submit_transfer failed with error %d\n", ret);
		exit(1);
	}

	tfr1 = libusb_alloc_transfer(32);
	assert(tfr1 != NULL);
	libusb_fill_iso_transfer(tfr1, devh, 0x00000082, isobuf1, 32 * 1008, 32, gotdata, NULL, 2000);
	libusb_set_iso_packet_lengths(tfr1, 1008);

	ret = libusb_submit_transfer(tfr1);
	if (ret != 0) {
		fprintf(stderr, "libusb_submit_transfer failed with error %d\n", ret);
		exit(1);
	}

/*
	memcpy(buf, "\x0b\x00\x00\x82\x01\x18\x00\x09", 0x0000008); // 0x0d = 0000 1101 // Without Bit0 Set, we get no data! We also need Bit3, Don't know what Bit2 Does?
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x0000008, 1000);
	fprintf(stderr, "242 control msg returned %d, bytes: ", ret);
		print_bytes(buf, ret);
	fprintf(stderr, "\n");
*/

	while (pcount < FCOUNT) {
		libusb_handle_events(NULL);
	}


	libusb_free_transfer(tfr0);
	libusb_free_transfer(tfr1);

	ret = libusb_release_interface(devh, 0);
	assert(ret == 0);
	libusb_close(devh);
	libusb_exit(NULL);
	return 0;
}
