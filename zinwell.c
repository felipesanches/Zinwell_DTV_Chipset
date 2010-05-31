/**
 * IBayo ISDB-T 1Seg DTV USB device driver
 * zinwell.c - main operations
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
#include "max2163.h"
#include "debug.h"

#define ZINWELL_VENDOR_ID      0x5a57
#define IB200_PRODUCT_ID       0x4210   /* ISDB-T DTV UB-10 */

libusb_device_handle *
ib200_open_device(libusb_device **devlist, size_t n)
{
	int ret;
	ssize_t i;
	libusb_device_handle *devh;

	debug_printf("<--");

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

	fprintf(stderr, "Failed to find USB device ID %04x:%04x\n", ZINWELL_VENDOR_ID, IB200_PRODUCT_ID);
	return NULL;
}

void
ib200_close_device(libusb_device_handle *devh)
{
	debug_printf("<--");
	libusb_close(devh);
}

static int
ib200_i2c_write(libusb_device_handle *devh, uint16_t wValue, uint16_t wIndex, unsigned char reg, unsigned char val)
{
	int ret;
	unsigned int timeout;
	uint8_t bmRequestType, bRequest;
	unsigned char addr = MAX2163_I2C_WRITE_ADDR;
	unsigned char cmd[13] = { wValue, addr, addr, 0x01, 0x01, reg, val, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	timeout = 1000;
	bRequest = 1;
	bmRequestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
	ret = libusb_control_transfer(devh, bmRequestType, bRequest, wValue, wIndex, cmd, sizeof(cmd), timeout);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return ret;
	}
	usleep(10000);
	return 0;
}

int
ib200_max2163_init(libusb_device_handle *devh)
{
	int ret;

	/* Initialize the IF Filter Register */
	/* \x0b\xc0\xc0\x01\x01\x00\x6f\x00\x00\x00\x00\x00\x33 */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_IF_FILTER_REG, 
						  /* 0x6f */
						 _IF_FILTER_13MHZ_BANDWIDTH | _IF_FILTER_BIAS_CURRENT | _IF_FILTER_CENTER_FREQUENCY_1_00);
	if (ret < 0) {
		debug_printf("Failed to configure the IF Filter Register");
		return ret;
	}

	/* Initialize the VAS Register */
	/* \x0b\xc0\xc0\x01\x01\x01\xb7\x00\x01\x00\x00\x00\x6e */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_VAS_REG, 
						  /* 0xb7 */
						 _VAS_REG_AUTOSELECT_45056_WAIT_TIME | _VAS_REG_CPS_AUTOMATIC | _VAS_REG_START_AT_CURR_USED_REGS);
	if (ret < 0) {
		debug_printf("Failed to configure the VAS Register");
		return ret;
	}

	/* Initialize the VCO Register */
	/* \x0b\xc0\xc0\x01\x01\x02\x29\x00\x02\x00\x00\x00\xaa" */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_VCO_REG, 
						  /* 0x29 */
						 _VCO_REG_VCOB_LOW_POWER | _VCO_REG_SUB_BAND_3 | _VCO_REG_VCO_1);
	if (ret < 0) {
		debug_printf("Failed to configure the VCO Register");
		return ret;
	}

	/* Initialize the PDET/RF-FILT Register */
	/* \x0b\xc0\xc0\x01\x01\x03\xc7\x00\x03\x00\x00\x00\xe6 */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_RF_FILTER_REG, 
						  /* 0xc7 */
						 _RF_FILTER_UHF_RANGE_710_806MHZ | _RF_FILTER_PWRDET_BUF_ON_GC1);
	if (ret < 0) {
		debug_printf("Failed to configure the PDET/RF-FILT Register");
		return ret;
	}

	/* Initialize the MODE Register */
	/* \x0b\xc0\xc0\x01\x01\x04\x00\x00\x04\x00\x00\x00\x22 */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_MODE_REG, 
						  /* 0x00 */
						 _MODE_REG_HIGH_SIDE_INJECTION | _MODE_REG_ENABLE_RF_FILTER | 
						 _MODE_REG_ENABLE_3RD_STAGE_RFVGA);
	if (ret < 0) {
		debug_printf("Failed to configure the MODE Register");
		return ret;
	}

	/* Initialize the R-Divider MSB Register */
	/* \x0b\xc0\xc0\x01\x01\x05\x38\x00\x05\x00\x00\x00\x5d */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_RDIVIDER_MSB_REG, 
						  /* 0x38, 56 decimal */
						 _RDIVIDER_MSB_REG_PLL_DIVIDER(56));
	if (ret < 0) {
		debug_printf("Failed to configure the R-Divider MSB Register");
		return ret;
	}
	
	/* Initialize the R-Divider LSB/CP Register */
	/* \x0b\xc0\xc0\x01\x01\x06\x00\x00\x06\x00\x00\x00\x99 */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_RDIVIDER_LSB_REG, 
						  /* 0x00 */
						 _RDIVIDER_LSB_REG_RFDA_37DB | _RDIVIDER_LSB_REG_ENABLE_RF_DETECTOR |
						 _RDIVIDER_LSB_REG_CHARGE_PUMP_1_5MA);
	if (ret < 0) {
		debug_printf("Failed to configure the R-Divider LSB/CP Register");
		return ret;
	}

	/* Initialize the N-Divider to 1658 decimal (0x067a) */

	/* Initialize the N-Divider MSB Register */
	/* \x0b\xc0\xc0\x01\x01\x07\x67\x00\x07\x00\x00\x00\xd5 */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_NDIVIDER_MSB_REG, 
						  /* 0x67 */
						 _NDIVIDER_MSB_REG_PLL_MOST_DIV(0x67));
	if (ret < 0) {
		debug_printf("Failed to configure the N-Divider MSB Register");
		return ret;
	}
	
	/* Initialize the N-Divider LSB/LIN Register */
	/* \x0b\xc0\xc0\x01\x01\x08\xa0\x00\x08\x00\x00\x00\x11 */
	ret = ib200_i2c_write(devh, 0x0b, 0x00, MAX2163_I2C_NDIVIDER_LSB_REG, 
						  /* 0xa0 */
						 _NDIVIDER_LSB_REG_STBY_NORMAL | _NDIVIDER_LSB_REG_RFVGA_NORMAL |
						 _NDIVIDER_LSB_REG_MIX_NORMAL | 
						 _NDIVIDER_LSB_REG_PLL_LEAST_DIV(0xa));
	if (ret < 0) {
		debug_printf("Failed to configure the N-Divider LSB/LIN Register");
		return ret;
	}

	return 0;
}

int
ib200_init(libusb_device_handle *devh)
{
	uint8_t request_type, request;
	uint16_t value, index;
	unsigned int timeout = 1000;
	unsigned char buf[65535];
	int i, ret;
	int bConfiguration, bInterfaceNumber, bAlternateSetting;

	debug_printf("<--");

	/* Check if any kernel driver already claimed this device */
	bInterfaceNumber = 0;
	ret = libusb_kernel_driver_active(devh, bInterfaceNumber);
	if (ret) {
		debug_printf("Error: the kernel is already handling this device");
		return 1;
	}
	
	/* Specify which bConfiguration to use. This device has only one. */
	bConfiguration = 1;
	ret = libusb_set_configuration(devh, bConfiguration);
	if (ret < 0) {
		debug_printf("libusb_set_configuration: failed with error %d", ret);
		return 1;
	}

	/* Specify which bInterfaceNumber to use. This device has only one. */
	ret = libusb_claim_interface(devh, bInterfaceNumber);
	if (ret < 0) {
		debug_printf("libusb_claim_interface: failed with error %d", ret);
		return 1;
	}

	/* 
	 * Specify which bAlternateSetting to use:
	 * 0: contains no endpoint descriptors
	 * 1: EP 2 IN @ 0x82 - wMaxPacketSize=0x03ff  (1 x 1023 bytes)
	 * 2: EP 2 IN @ 0x82 - wMaxPacketSize=0x1400  (3 x 1024 bytes)
	 */
	bAlternateSetting = 0;
	ret = libusb_set_interface_alt_setting(devh, bInterfaceNumber, bAlternateSetting);
	if (ret < 0) {
		debug_printf("libusb_set_interface_alt_setting: failed with error %d", ret);
		return 1;
	}

	/* Get status from first endpoint */
	/*                                          ------------- request_type
	 *                                         /  ---------------- request
	 *                                        /  /  ---------------- value
	 *                                       /  /  /    ------------ index
	 *                                      /  /  /    /     -- buffer_len
	 *                                     /  /  /    /     /   /
	 * edbd5e00 2053267881 S Ci:1:017:0 s c0 01 0001 0000 0002 2 <
	 * edbd5e00 2053268007 C Ci:1:017:0 0 2 = 0103
	 */
	request = 1; value = 0x01; index = 0;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
	ret = libusb_control_transfer(devh, request_type, request, value, index, buf, 2, timeout);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return 1;
	}
	hexdump(buf, 0x2);

	/* Initialize the MAX2163 registers */
	ret = ib200_max2163_init(devh);
	if (ret < 0)
		return 1;

	/* Get status from second endpoint */
	request = 1; value = 0x01; index = 0;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
	ret = libusb_control_transfer(devh, request_type, request, value, index, buf, 2, timeout);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return 1;
	}
	hexdump(buf, 0x2);

	request = 1; value = 0x0b; index = 0;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
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

	debug_printf("<--");

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
	debug_printf("<--");
	return true;
}

ssize_t
ib200_read(libusb_device_handle *devh, void *buf, size_t count)
{
	debug_printf("<--");
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
