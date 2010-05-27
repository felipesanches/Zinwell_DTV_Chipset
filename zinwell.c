/**
 * IBayo ISDB-T 1Seg DTV USB device driver
 *
 * Copyright (c) 2010, Lucas C. Villa Real <lucasvr@gobolinux.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Released under the GNU GPL version 
 *
 * For more information on this project, please visit the following URL:
 * http://groups.fsf.org/wiki/LinuxLibre:ISDB_USB_ZINWELL
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <libusb.h>

#include "zinwell.h"

#define ZINWELL_VENDOR_ID 0x5a57
#define IB200_PRODUCT_ID  0x4210   /* ISDB-T DTV UB-10 */

#define debug_printf(msg...) do { \
	fprintf(stderr, "%s:%d: ", __FUNCTION__, __LINE__); \
	fprintf(stderr, msg); \
	fprintf(stderr, "\n"); \
} while(0)

static void 
hexdump(unsigned char *buf, int len)
{
	bool shown_last_ascii = false;
	int x, y, counter = 1;

	/* Header */
	printf("\n");
	for (x=0; x<46; ++x)
		printf("*");
	printf(" Hexdump of %d bytes ", len);
	for (x=0; x<46; ++x)
		printf("*");
	printf("\n");

	/* Dump */
	printf("00000000    ");
	for (x=1; x<=len; ++x) {
		shown_last_ascii = false;
		printf("%02x ", buf[x-1] & 0xff);
		
		if ((x % 16) == 0) {
			printf(" ");
			for (y=x-16; y<=x; ++y)
				printf("%c", isprint(buf[y]) ? buf[y] & 0xff : '.');
			printf("\n%08x    ", counter++ * 16);
			shown_last_ascii = true;
		} else if ((x % 4) == 0)
			printf(" ");
	}

	if (! shown_last_ascii) {
		/* Dump ASCII for last line */
		int spaces = 16 - (len % 16);
		for (x=0; x<spaces; ++x) {
			printf("   ");
			if ((x % 4) == 0)
				printf(" ");
		}
		for (y=len-(len%16); y<len; ++y)
			printf("%c", isprint(buf[y]) ? buf[y] & 0xff : '.');
	}

	/* Footer */
	printf("\n");
	for (x=0; x<113; ++x)
		printf("*");
	printf("\n");
}


libusb_device_handle *
ib200_open_device(libusb_device **devlist, size_t n)
{
	int ret;
	ssize_t i;
	libusb_device_handle *devh;

	printf("--> %s\n", __FUNCTION__);

	for (i=0; i<n; ++i) {
		libusb_device *dev = devlist[i];
		struct libusb_device_descriptor desc;

		ret = libusb_get_device_descriptor(dev, &desc);
		if (ret < 0) {
			debug_printf("libusb_get_device_descriptor: failed with error %d", ret);
			return NULL;
		}

		if (desc.idVendor == ZINWELL_VENDOR_ID && desc.idProduct == IB200_PRODUCT_ID) {
			ret = libusb_open(dev, &devh);
			if (ret < 0) {
				debug_printf("libusb_open: failed with error %d", ret);
				return NULL;
			}
			return devh;
		}
	}

	return NULL;
}

void
ib200_close_device(libusb_device_handle *devh)
{
	printf("--> %s\n", __FUNCTION__);
	libusb_close(devh);
}

int
ib200_init(libusb_device_handle *devh)
{
	uint8_t request_type, request;
	uint16_t value, index;
	unsigned int timeout = 1000;
	unsigned char buf[65535];
	int i, ret;

	printf("--> %s\n", __FUNCTION__);


	ret = libusb_get_descriptor(devh, LIBUSB_DT_DEVICE, 0, buf, 0x12);
	if (ret < 0) {
		debug_printf("libusb_get_descriptor: failed with error %d", ret);
		return 1;
	}
	hexdump(buf, 0x12);
	ret = libusb_get_descriptor(devh, LIBUSB_DT_CONFIG, 0, buf, 0x09);
	if (ret < 0) {
		debug_printf("libusb_get_descriptor: failed with error %d", ret);
		return 1;
	}
	hexdump(buf, 0x09);
	ret = libusb_get_descriptor(devh, LIBUSB_DT_CONFIG, 0, buf, 0x42);
	if (ret < 0) {
		debug_printf("libusb_get_descriptor: failed with error %d", ret);
		return 1;
	}
	hexdump(buf, 0x42);
//	ret = libusb_release_interface(devh, 0);
//	if (ret < 0) {
//		debug_printf("libusb_release_interface: failed with error %d", ret);
//		return 1;
//	}

	ret = libusb_set_configuration(devh, 1);
	if (ret < 0) {
		debug_printf("libusb_set_configuration: failed with error %d", ret);
		return 1;
	}
	ret = libusb_claim_interface(devh, 0);
	if (ret < 0) {
		debug_printf("libusb_claim_interface: failed with error %d", ret);
		return 1;
	}
	ret = libusb_set_interface_alt_setting(devh, 0, 0);
	if (ret < 0) {
		debug_printf("libusb_set_interface_alt_setting: failed with error %d", ret);
		return 1;
	}

	/* Configure first endpoint */
	request = 1; value = 0x01; index = 0;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
	ret = libusb_control_transfer(devh, request_type, request, value, index, buf, 2, timeout);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return 1;
	}
	hexdump(buf, 0x2);

	request = 1; value = 0x0b; index = 0;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;
	for (i=0; i<sizeof(ib200_init_data_ep1)/sizeof(char*); ++i) {
		ret = libusb_control_transfer(devh, request_type, request, value, index, 
				(unsigned char *) &ib200_init_data_ep1[i], 13, timeout);
		if (ret < 0) {
			debug_printf("libusb_control_transfer: failed with error %d", ret);
			return 1;
		}
		usleep(ib200_init_delays_ep1[i] * 1000);
	}

	/* Configure second endpoint */
	request = 1; value = 0x01; index = 0;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
	ret = libusb_control_transfer(devh, request_type, request, value, index, buf, 2, timeout);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return 1;
	}
	hexdump(buf, 0x2);

	request = 1; value = 0x0b; index = 0;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE;
	for (i=0; i<sizeof(ib200_init_data_ep2)/sizeof(char*); ++i) {
		ret = libusb_control_transfer(devh, request_type, request, value, index, 
				(unsigned char *) ib200_init_data_ep2[i], 13, timeout);
		if (ret < 0) {
			debug_printf("libusb_control_transfer: failed with error %d", ret);
			return 1;
		}
		usleep(ib200_init_delays_ep2[i] * 1000);
	}

	/* Configuration descriptor */
	ret = libusb_get_descriptor(devh, LIBUSB_DT_CONFIG, 0x0000000, buf, 0x0000109);
	if (ret < 0) {
		debug_printf("libusb_get_descriptor: failed with error %d", ret);
		return 1;
	}
	ret = libusb_set_interface_alt_setting(devh, 0, 1);
	if (ret < 0) {
		debug_printf("libusb_set_interface_alt_setting: failed with error %d", ret);
		return 1;
	}
	usleep(20*1000);

	return 0;
}

int 
ib200_set_frequency(libusb_device_handle *devh, int frequency)
{
	bool is_valid_frequency = false;
	int i, valid_frequencies[] = {
		473, 479, 485, 491, 497, 503, 509, 515, 521, 527, 
		533, 539, 545, 551, 557, 563, 569, 575, 581, 587, 
		593, 599, 605, 611, 617, 623, 629, 635, 641, 647, 
		653, 659, 665, 671, 677, 683, 689, 695, 701, 707,
		713, 719, 725, 731, 737, 743, 749, 755, 761, 767, 
		773, 779, 785, 791, 797, 803
	};

	printf("--> %s\n", __FUNCTION__);

	for (i=0; i<sizeof(valid_frequencies)/sizeof(int); ++i)
		if (valid_frequencies[i] == frequency) {
			is_valid_frequency = true;
			break;
		}

	if (! is_valid_frequency)
		debug_printf("Warning: there are no known broadcasters on frequency %d.", frequency);

	return 0;
}
	
bool
ib200_has_signal(libusb_device_handle *devh)
{
	printf("--> %s\n", __FUNCTION__);
	return true;
}

ssize_t
ib200_read(libusb_device_handle *devh, void *buf, size_t count)
{
	printf("--> %s\n", __FUNCTION__);
	return 0;
}


struct user_options {
	int frequency;
	bool initialize;
	bool check_signal;
};

void
show_usage(char *appname)
{
	printf("Usage: %s <options>\n\n"
		   "Available options are:\n"
		   "  -i, --init                Initialize tuner\n"
		   "  -f, --frequency <freq>    Tune to frequency <freq>\n"
		   "  -s, --check-signal        Check signal\n"
		   "  -h, --help                This help\n"
		   , appname);

}

struct user_options *
parse_args(int argc, char **argv)
{
	struct user_options *opts, zeroed_opts;
	const char *short_options = "if:sh";
	struct option long_options[] = {
		{ "init", 0, 0, 0 },
		{ "frequency", 1, 0, 'f' },
		{ "check-signal", 0, 0, 0 },
		{ "help", 0, 0, 0 },
	};

	opts = calloc(1, sizeof(struct user_options));
	if (! opts) {
		perror("calloc");
		exit(1);
	}

	memset(&zeroed_opts, 0, sizeof(zeroed_opts));

	while (true) {
		int c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
			case 'i':
				opts->initialize = true;
				break;
			case 'f':
				opts->frequency = atoi(optarg);
				break;
			case 's':
				opts->check_signal = true;
				break;
			case 'h':
				show_usage(argv[0]);
				exit(0);
			case '?':
			default:
				break;
		}
	}

	if (memcmp(opts, &zeroed_opts, sizeof(*opts)) == 0) {
		show_usage(argv[0]);
		free(opts);
		exit(1);
	}

	return opts;
}

int
main(int argc, char **argv)
{
	int ret;
	size_t n;
	bool has_signal;
	libusb_context *ctx;
	libusb_device **dev_list;
	libusb_device_handle *devh = NULL;
	struct user_options *user_options;

	user_options = parse_args(argc, argv);

	ret = libusb_init(&ctx);
	if (ret) {
		debug_printf("libusb_init: failed with error %d", ret);
		return 1;
	}

	libusb_set_debug(ctx, 3);

	n = libusb_get_device_list(ctx, &dev_list);
	if (n < 0) {
		debug_printf("libusb_get_device_list: failed with error %d", n);
		goto out_exit;
	}

	devh = ib200_open_device(dev_list, n);
	if (! devh)
		goto out_free;

	if (user_options->initialize) {
		ret = ib200_init(devh);
		if (ret < 0)
			goto out_close;
	}

	if (user_options->frequency) {
		ret = ib200_set_frequency(devh, user_options->frequency);
		if (ret < 0)
			goto out_close;
	}

	if (user_options->check_signal) {
		has_signal = ib200_has_signal(devh);
		printf("ib200_has_signal: %d\n", has_signal);
	}

	sleep(3);

out_close:
	ib200_close_device(devh);
out_free:
	libusb_free_device_list(dev_list, 1);
out_exit:
	libusb_exit(ctx);
	free(user_options);
	return ret;
}
