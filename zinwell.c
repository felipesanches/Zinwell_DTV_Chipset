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
#include <errno.h>
#include <pthread.h>
#include <libusb.h>

#include "max2163.h"
#include "debug.h"

#define ZINWELL_VENDOR_ID      0x5a57
#define IB200_PRODUCT_ID       0x4210   /* ISDB-T DTV UB-10 */
#define IB200_CONFIG_ENDPOINT  0x82
#define IB200_DEFAULT_RDIVIDER 0x38     /* default PLL divider */

/**
 * The following addresses are used when communicating with the USB device:
 *  \x0b\x00\x00\x82 -> Direct communication with the SMI-2020CBE (endpoint 0x82) (most likely)
 *  \x0b\x00\x20\x82 -> Direct communication with the SMI-2020CBE (endpoint 0x82) (most likely)
 *  \x0b\xc0\xc0\x01 -> MAX2163 I2C registers
 *  \x0b\xee\xc0\x01 -> Firmware code upload address prefix
 *  \x0b\xee\xc4\x01 -> Unknown, currently calling it a "shadow register"
 *  \x0b\xee\xe0\x01 -> Unknown, perhaps this maps to MegaChips' MA50159 registers?
 *
 * The following commands were seen being written to \x0b\x00\x20\x82 in preparation 
 * to submitting an USB ISOCH transfer:
 *
 *  <- \x0b\x00\x20\x82\x01\x30\x80\x89\x01\xf5\x31\x89\xa7
 *  -> 0b 00 20 82 01 30 80 89 01 f5 31 89 a7 ".. ..0....1.."
 *  
 *  <- \x0b\x00\x20\x82\x01\x30\x80\x89\x01\x00\x00\x00\xbc
 *  -> 0b 00 20 82 01 30 80 89 01 00 00 00 bc ".. ..0......."
 *
 *  <- \x0b\x00\x20\x82\x01\x30\x80\x89\x00\x37\xe8\x89\xf4
 *  -> 0b 00 20 82 01 30 80 89 00 37 e8 89 f4 ".. ..0...7..."
 *
 * After each of these individual commands, writes to \x0b\x00\x00\x82 are issued.
 */

struct ib200_handle {
	libusb_device_handle *devh;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	bool packet_arrived;
	bool device_closed;
	void *read_buffer;
	size_t read_count;
};

struct ib200_handle *
ib200_open_device(libusb_device **devlist, size_t n)
{
	int ret;
	ssize_t i;
	struct ib200_handle *handle;
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
			handle = calloc(1, sizeof(struct ib200_handle));
			if (! handle) {
				libusb_close(devh);
				perror("malloc");
				return NULL;
			}
			handle->devh = devh;
			handle->packet_arrived = false;
			handle->device_closed = false;
			pthread_mutex_init(&handle->lock, NULL);
			pthread_cond_init(&handle->cond, NULL);
			return handle;
		}
	}

	fprintf(stderr, "Failed to find USB device ID %04x:%04x\n", ZINWELL_VENDOR_ID, IB200_PRODUCT_ID);
	return NULL;
}

void
ib200_close_device(struct ib200_handle *handle)
{
	debug_printf("<--");
	if (handle) {
		pthread_mutex_lock(&handle->lock);
		handle->device_closed = true;
		pthread_cond_signal(&handle->cond);
		pthread_mutex_unlock(&handle->lock);

		libusb_close(handle->devh);
		pthread_mutex_destroy(&handle->lock);
		pthread_cond_destroy(&handle->cond);
		free(handle);
	}
}

/**
 * Read from the I2C
 * @param devh LibUSB device handle
 * @param wValue wValue, as specified by the USB Specification 2.0
 * @param wIndex wIndex, as specified by the USB Specification 2.0
 * @param reg I2C register to read from
 * @param buf output buffer
 * @param size buffer size
 * @return the number of bytes read on success or a negative value on error.
 */
static int
ib200_i2c_read(libusb_device_handle *devh, uint16_t wValue, uint16_t wIndex, 
	unsigned char *buf, size_t size)
{
	int ret;
	uint8_t bmRequestType, bRequest;

	bRequest = 1;
	bmRequestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
	ret = libusb_control_transfer(devh, bmRequestType, bRequest, wValue, wIndex, buf, size, 1000);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return ret;
	}
	usleep(10000);
	return 0;
}

/**
 * Write to the I2C configuration registers
 * @param devh LibUSB device handle
 * @param addr address to write to. The MSB is sent as 2nd argument of the command and the LSB as 3rd.
 * @param wValue wValue, as specified by the USB Specification 2.0
 * @param wIndex wIndex, as specified by the USB Specification 2.0
 * @param reg I2C register to write to
 * @param val value to write to the I2C register
 * @param last value to write in the last byte of the command
 * @return 0 on success or a negative value on error.
 */
static int
ib200_i2c_write(libusb_device_handle *devh, uint16_t addr, uint16_t wValue, uint16_t wIndex, unsigned char reg, 
	unsigned char val, unsigned char last)
{
	int ret;
	uint8_t bmRequestType, bRequest;
	unsigned char stub = 0x00;
	unsigned char addr_high = (addr >> 8) & 0xff;
	unsigned char addr_low = addr & 0xff;
	unsigned char cmd[13] = { wValue, addr_high, addr_low, 0x01, 0x01, reg, val, stub, reg, stub, stub, stub, last };

	bRequest = 1;
	bmRequestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
	ret = libusb_control_transfer(devh, bmRequestType, bRequest, wValue, wIndex, cmd, sizeof(cmd), 1000);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return ret;
	}
	usleep(10000);
	return 0;
}

/**
 * Write to the unknown 0beec401 address. I'm calling this a "shadow" register, as the last byte written to the I2C
 * (which seems to be a random value) gets also written to this "shadow" register at some points.
 * @param reg 4-byte I2C register. MSB is mapped to the 9th byte, LSB to the 12th byte.
 * @param last value to write in the last byte of the command
 * @return 0 on success or a negative value on error.
 */
static int
ib200_shadow_write(libusb_device_handle *devh, uint32_t reg, unsigned char last)
{
	int ret;
	uint16_t wValue, wIndex;
	uint8_t bRequestType, bRequest;
	unsigned char cmd[13] = { 
		0x0b, 0xee, 0xc4, 0x01, 
		0x01, 0x02, 0x01, 0x00, 
		((reg >> 24) & 0xff), 
		((reg >> 16) & 0xff), 
		((reg >>  8) & 0xff), 
		((reg)       & 0xff), 
		last 
	};
	
	bRequest = 1;
	wValue = 0x0b;
	wIndex = 0x00;
	bRequestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
	ret = libusb_control_transfer(devh, bRequestType, bRequest, wValue, wIndex, cmd, sizeof(cmd), 1000);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return ret;
	}
	usleep(10000);
	return 0;
}

/**
 * Write to the USB configuration endpoint
 * @param devh LibUSB device handle
 * @param addr address to write to. The MSB is sent as 2nd argument of the command and the LSB as 3rd.
 * @param wValue wValue, as specified by the USB Specification 2.0
 * @param wIndex wIndex, as specified by the USB Specification 2.0
 * @param data: 8-byte array holding the data to be transferred to the USB configuration endpoint
 * @return 0 on success or a negative value on error.
 */
static int
ib200_endpoint_write(libusb_device_handle *devh, uint16_t addr, uint16_t wValue, uint16_t wIndex, unsigned char *data)
{
	int ret;
	uint8_t bmRequestType, bRequest;
	unsigned char stub = 0x00;
	unsigned char addr_high = (addr >> 8) & 0xff;
	unsigned char addr_low = addr & 0xff;
	unsigned char cmd[13] = { wValue, addr_high, addr_low, IB200_CONFIG_ENDPOINT, 0x01, stub, stub, stub, stub, stub, stub, stub, stub };

	memcpy(&cmd[5], data, 8);
	bRequest = 1;
	bmRequestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
	ret = libusb_control_transfer(devh, bmRequestType, bRequest, wValue, wIndex, cmd, sizeof(cmd), 1000);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return ret;
	}
	usleep(10000);
	return 0;
}

/**
 * Initialize the configuration descriptor
 * It's still unclear which features are being configured at this stage.
 * It looks like the commands are 0x34, 0x35, 0x3a and 0x3b.
 */
int
ib200_init_configuration_descriptor(libusb_device_handle *devh)
{
	int ret;
	unsigned char buf[65535], data[8];
	uint8_t request_type, request;
	uint16_t value, index, addr = 0x0000;

	/* Get status from the configuration descriptor */
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
	ret = libusb_control_transfer(devh, request_type, request, value, index, buf, 2, 1000);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return 1;
	}
	/* TODO: interpret the 2 bytes returned (0x01 0x03) */
	hexdump(buf, 0x2);

	/* 
	 * TODO: interpret the commands below.
	 * Based on some other logs it seems like only the first
	 * 3 bytes are meaningful. The remaning ones looks like garbage.
	 */
	/* \x0b\x00\x00\x82\x01\x00\x34\x21\xf0\xbc\x33\x89\x00 */
	memcpy(data, "\x00\x34\x21\xf0\xbc\x33\x89\x00", 8);
	ret = ib200_endpoint_write(devh, addr, 0x0b, 0x00, data);
	if (ret < 0)
		return ret;

	/* \x0b\x00\x00\x82\x01\x00\x35\x21\xf0\xbc\x33\x89\x00 */
	memcpy(data, "\x00\x35\x21\xf0\xbc\x33\x89\x00", 8);
	ret = ib200_endpoint_write(devh, addr, 0x0b, 0x00, data);
	if (ret < 0)
		return ret;
	
	/* \x0b\x00\x00\x82\x01\x00\x35\x01\x00\x00\x00\x00\xea */
	memcpy(data, "\x00\x35\x01\x00\x00\x00\x00\xea", 8);
	ret = ib200_endpoint_write(devh, addr, 0x0b, 0x00, data);
	if (ret < 0)
		return ret;

	/* \x0b\x00\x00\x82\x01\x00\x3a\x80\x00\x00\x00\x00\xea */
	memcpy(data, "\x00\x3a\x80\x00\x00\x00\x00\xea", 8);
	ret = ib200_endpoint_write(devh, addr, 0x0b, 0x00, data);
	if (ret < 0)
		return ret;

	/* \x0b\x00\x00\x82\x01\x00\x3b\x00\x00\x00\x00\x00\xea */
	memcpy(data, "\x00\x3b\x00\x00\x00\x00\x00\xea", 8);
	ret = ib200_endpoint_write(devh, addr, 0x0b, 0x00, data);
	if (ret < 0)
		return ret;

	/* XXX: in another log I noticed 0xf9 instead of 0x39 */
	/* \x0b\xee\xc4\x01\x01\x02\x01\x00\x58\x39\x52\xba\x0d */
	ret = ib200_shadow_write(devh, 0x583952ba, 0x0d);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * Initialize the MAX2163 to a reasonable configuration.
 */
int
ib200_max2163_init(libusb_device_handle *devh)
{
	int i, ret;
	unsigned char random_val;
	uint16_t addr = (MAX2163_I2C_WRITE_ADDR << 8) | MAX2163_I2C_WRITE_ADDR;

	/* Initialize the IF Filter Register */
	random_val = 0x33;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_IF_FILTER_REG, 
			/* 0x6f */ _IF_FILTER_13MHZ_BANDWIDTH | _IF_FILTER_BIAS_CURRENT | 
			_IF_FILTER_CENTER_FREQUENCY_1_00,
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_IF_FILTER_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the IF Filter Register");
		return ret;
	}


	/* Initialize the VAS Register */
	random_val = 0x6e;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_VAS_REG, 
			/* 0xb7 */ _VAS_REG_AUTOSELECT_45056_WAIT_TIME | _VAS_REG_CPS_AUTOMATIC | 
			_VAS_REG_START_AT_CURR_USED_REGS,
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_VAS_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the VAS Register");
		return ret;
	}

	/* Initialize the VCO Register */
	random_val = 0xaa;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_VCO_REG, 
			/* 0x29 */ _VCO_REG_VCOB_LOW_POWER | _VCO_REG_SUB_BAND_3 | _VCO_REG_VCO_1,
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_VCO_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the VCO Register");
		return ret;
	}

	/* Initialize the PDET/RF-FILT Register */
	random_val = 0xe6;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_RF_FILTER_REG, 
			/* 0xc7 */ _RF_FILTER_UHF_RANGE_710_806MHZ | _RF_FILTER_PWRDET_BUF_ON_GC1,
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_RF_FILTER_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the PDET/RF-FILT Register");
		return ret;
	}

	/* Initialize the MODE Register */
	random_val = 0x22;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_MODE_REG, 
			/* 0x00 */ _MODE_REG_HIGH_SIDE_INJECTION | _MODE_REG_ENABLE_RF_FILTER | 
			_MODE_REG_ENABLE_3RD_STAGE_RFVGA,
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_MODE_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the MODE Register");
		return ret;
	}

	/* Initialize the R-Divider MSB Register */
	random_val = 0x5d;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_RDIVIDER_MSB_REG, 
			_RDIVIDER_MSB_REG_PLL_DIVIDER(IB200_DEFAULT_RDIVIDER),
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_RDIVIDER_MSB_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the R-Divider MSB Register");
		return ret;
	}
	
	/* Initialize the R-Divider LSB/CP Register */
	random_val = 0x99;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_RDIVIDER_LSB_REG, 
			/* 0x00 */ _RDIVIDER_LSB_REG_RFDA_37DB | _RDIVIDER_LSB_REG_ENABLE_RF_DETECTOR | 
			_RDIVIDER_LSB_REG_CHARGE_PUMP_1_5MA,
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_RDIVIDER_LSB_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the R-Divider LSB/CP Register");
		return ret;
	}

	/* Initialize the N-Divider MSB Register */
	random_val = 0xd5;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_NDIVIDER_MSB_REG, 
			/* 0x67 */ _NDIVIDER_MSB_REG_PLL_MOST_DIV(0x67),
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_NDIVIDER_MSB_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the N-Divider MSB Register");
		return ret;
	}
	
	/* Initialize the N-Divider LSB/LIN Register */
	random_val = 0x11;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_NDIVIDER_LSB_REG, 
			/* 0xa0 */ _NDIVIDER_LSB_REG_STBY_NORMAL | _NDIVIDER_LSB_REG_RFVGA_NORMAL |
			_NDIVIDER_LSB_REG_MIX_NORMAL | _NDIVIDER_LSB_REG_PLL_LEAST_DIV(0xa0),
			random_val);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_NDIVIDER_LSB_REG << 24, random_val);
	if (ret < 0) {
		debug_printf("Failed to configure the N-Divider LSB/LIN Register");
		return ret;
	}

	/* Initialize non-documented registers */
	for (i=0x11; i<=0x16; ++i) {
		unsigned char cmd[13] = { 0x0b, (addr>>8) & 0xff, addr & 0xff, 0x01, 0x01, i, 0, 0, i-0x11, 0, 0, 0, 0x11 };
		libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 
			1, 0x0b, 0x00, cmd, sizeof(cmd), 1000);
		if (i < 0x16)
			ib200_shadow_write(devh, (i-0x11) << 24, 0x00);
	}

	return 0;
}

int
ib200_upload_firmware(libusb_device_handle *devh)
{
	int i, ret;
	uint16_t wValue, wIndex;
	uint8_t bmRequestType, bRequest;
	unsigned char firmware[56] = {
		0x01, 0x01, 0x04, 0x08, 0x05, 0x01, 0x06, 0x00, 
		0x15, 0xf8, 0x19, 0xcc, 0x4d, 0x08, 0x70, 0x02, 
		0x08, 0x28, 0x09, 0x10, 0x40, 0x0d, 0x43, 0xf8, 
		0x47, 0xc0, 0x48, 0x7f, 0xa1, 0x40, 0xa5, 0x4c, 
		0xb9, 0xd9, 0xac, 0x01, 0xad, 0x80, 0xae, 0x43, 
		0xaf, 0x01, 0xb0, 0x22, 0xbd, 0xe9, 0xc0, 0x1c, 
		0xc4, 0x20, 0xc5, 0x04, 0xca, 0x33, 0x01, 0x02
	};
	unsigned char cmd[13] = { 0x0b, 0xee, 0xc0, 0x01, 0x01, 0x00, 0x00, 0xba, 0xe6, 0x44, 0x9f, 0x3e, 0x58 };

	bRequest = 1;
	wValue = 0x0b;
	wIndex = 0x00;
	bmRequestType = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
	for (i=0; i<sizeof(firmware); i+=2) {
		cmd[5] = firmware[i];
		cmd[6] = firmware[i+1];
		ret = libusb_control_transfer(devh, bmRequestType, bRequest, wValue, wIndex, cmd, sizeof(cmd), 1000);
		if (ret < 0) {
			debug_printf("libusb_control_transfer: failed with error %d", ret);
			return ret;
		}
		usleep(10000);
	}
	return 0;
}
	
int
ib200_init_ep82(libusb_device_handle *devh)
{
	unsigned char data[8], buf[13];
	uint8_t request_type, request;
	uint16_t value, index, addr;
	int ret;

	/* \x0b\x00\x20\x82\x01\x15\x80\x00\x18\x01\x00\x00\x74 */
	addr = 0x0020;
	memcpy(data, "\x15\x80\x00\x18\x01\x00\x00\x74", 8);
	ret = ib200_endpoint_write(devh, addr, 0x0b, 0x00, data);
	if (ret < 0)
		return ret;

	value = 0x0b;
	index = 0x00;
	request = 0x01;
	request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
	ret = libusb_control_transfer(devh, request_type, request, value, index, buf, sizeof(buf), 1000);
	if (ret < 0) {
		debug_printf("libusb_control_transfer: failed with error %d", ret);
		return ret;
	}
	hexdump(buf, ret);
	usleep(1000);

	/* \x0b\x00\x00\x82\x01\x15\x80\xc3\x18\x01\x00\x00\x74 */
	addr = 0x0000;
	memcpy(data, "\x15\x80\xc3\x18\x01\x00\x00\x74", 8);
	ret = ib200_endpoint_write(devh, addr, 0x0b, 0x00, data);
	if (ret < 0)
		return ret;

	return 0;
}

int
ib200_init(struct ib200_handle *handle)
{
	libusb_device_handle *devh = handle->devh;
	int ret, bConfiguration, bInterfaceNumber, bAlternateSetting;

	/* Check if any kernel driver already claimed this device */
	bInterfaceNumber = 0;
	ret = libusb_kernel_driver_active(devh, bInterfaceNumber);
	if (ret) {
		debug_printf("Error: the kernel is already handling this device");
		return 1;
	}
	
	/* Enable LibUSB debug messages */
	libusb_set_debug(NULL, 3);

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
	 * 0: Configuration Descriptor
	 * 1: EP 2 IN @ 0x82 - wMaxPacketSize=0x03ff  (1 x 1023 bytes)
	 * 2: EP 2 IN @ 0x82 - wMaxPacketSize=0x1400  (3 x 1024 bytes)
	 */
	bAlternateSetting = 0;
	ret = libusb_set_interface_alt_setting(devh, bInterfaceNumber, bAlternateSetting);
	if (ret < 0) {
		debug_printf("libusb_set_interface_alt_setting: failed with error %d", ret);
		return 1;
	}

	/* Initialize the Configuration Descriptor */
	ret = ib200_init_configuration_descriptor(devh);
	if (ret < 0)
		return 1;

	/* Initialize the MAX2163 registers */
	ret = ib200_max2163_init(devh);
	if (ret < 0)
		return 1;

	/* Upload firmware */
	ret = ib200_upload_firmware(devh);
	if (ret < 0)
		return 1;

	/* 
	 * Specify which bAlternateSetting to use:
	 * 0: Configuration Descriptor
	 * 1: EP 2 IN @ 0x82 - wMaxPacketSize=0x03ff  (1 x 1023 bytes)
	 * 2: EP 2 IN @ 0x82 - wMaxPacketSize=0x1400  (3 x 1024 bytes)
	 */
	bAlternateSetting = 1;
	ret = libusb_set_interface_alt_setting(devh, bInterfaceNumber, bAlternateSetting);
	if (ret < 0) {
		debug_printf("libusb_set_interface_alt_setting: failed with error %d", ret);
		return 1;
	}

	/* Black magic */
	ret = ib200_init_ep82(devh);
	if (ret < 0)
		return 1;

	return 0;
}

/**
 * Tune to a given frequency.
 * @param devh USB device handle
 * @param freq frequency to tune to
 * @return 0 on success or a negative value on error
 */
int 
ib200_set_frequency(struct ib200_handle *handle, int frequency)
{
	uint16_t addr = (MAX2163_I2C_WRITE_ADDR << 8) | MAX2163_I2C_WRITE_ADDR;
	libusb_device_handle *devh = handle->devh;
	int i, ret, rflt_reg, n_divider;
	int valid_frequencies[] = {
		473, 479, 485, 491, 497, 503, 509, 515, 521, 527, 
		533, 539, 545, 551, 557, 563, 569, 575, 581, 587, 
		593, 599, 605, 611, 617, 623, 629, 635, 641, 647, 
		653, 659, 665, 671, 677, 683, 689, 695, 701, 707,
		713, 719, 725, 731, 737, 743, 749, 755, 761, 767, 
		773, 779, 785, 791, 797, 803
	};
	bool is_valid_frequency = false;

	for (i=0; i<sizeof(valid_frequencies)/sizeof(int); ++i)
		if (valid_frequencies[i] == frequency) {
			is_valid_frequency = true;
			break;
		}

	if (! is_valid_frequency) {
		debug_printf("Warning: there are no known broadcasters on frequency %d.", frequency);
		return -EINVAL;
	}

	if (frequency < 488)
		rflt_reg = _RF_FILTER_UHF_RANGE_470_488MHZ;
	else if (frequency < 512)
		rflt_reg = _RF_FILTER_UHF_RANGE_488_512MHZ;
	else if (frequency < 542)
		rflt_reg = _RF_FILTER_UHF_RANGE_512_542MHZ;
	else if (frequency < 572)
		rflt_reg = _RF_FILTER_UHF_RANGE_542_572MHZ;
	else if (frequency < 608)
		rflt_reg = _RF_FILTER_UHF_RANGE_572_608MHZ;
	else if (frequency < 656)
		rflt_reg = _RF_FILTER_UHF_RANGE_608_656MHZ;
	else if (frequency < 710)
		rflt_reg = _RF_FILTER_UHF_RANGE_656_710MHZ;
	else
		rflt_reg = _RF_FILTER_UHF_RANGE_710_806MHZ;


	/* Initialize the RF Filter Register at 0x03 */
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_RF_FILTER_REG, 
			 rflt_reg | _RF_FILTER_PWRDET_BUF_ON_GC1 | _RF_FILTER_UNUSED, 0x6e);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_RF_FILTER_REG << 24, 0x6e);
	if (ret < 0) {
		debug_printf("Failed to configure the RF Filter Register");
		return ret;
	}
	
	/* Initialize the N-Divider Registers */
	n_divider = (frequency * IB200_DEFAULT_RDIVIDER) + 40;
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_NDIVIDER_MSB_REG, 
			 _NDIVIDER_MSB_REG_PLL_MOST_DIV(n_divider >> 8), 0xd5);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_RF_FILTER_REG << 24, 0xd5);
	if (ret < 0) {
		debug_printf("Failed to configure the N-Divider MSB Register");
		return ret;
	}
	
	/* Initialize the N-Divider LSB/LIN Register */
	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_NDIVIDER_LSB_REG, 
			 _NDIVIDER_LSB_REG_STBY_NORMAL | _NDIVIDER_LSB_REG_RFVGA_NORMAL |
			 _NDIVIDER_LSB_REG_MIX_NORMAL | _NDIVIDER_LSB_REG_PLL_LEAST_DIV(n_divider), 0x11);
	if (ret == 0)
		ret = ib200_shadow_write(devh, MAX2163_I2C_NDIVIDER_LSB_REG << 24, 0x11);
	if (ret < 0) {
		debug_printf("Failed to configure the N-Divider LSB/LIN Register");
		return ret;
	}

	return 0;
}
	
bool
ib200_has_signal(struct ib200_handle *handle)
{
	uint16_t addr = (MAX2163_I2C_WRITE_ADDR << 8) | MAX2163_I2C_WRITE_ADDR;
	libusb_device_handle *devh = handle->devh;
	unsigned char buf[32];
	int ret;

	ret = ib200_i2c_write(devh, addr, 0x0b, 0x00, MAX2163_I2C_STATUS_REG, 0, 0);
	debug_printf("ib200_i2c_write=%d", ret);

	ret = ib200_i2c_read(devh, 0x0b, 0x00, buf, sizeof(buf));
	debug_printf("ib200_i2c_read=%d", ret);
	if (ret > 0)
		hexdump(buf, ret);

	return true;
}

static void 
iso_callback(struct libusb_transfer *transfer)
{
    int i, buf_index=0;
	struct ib200_handle *handle = (struct ib200_handle *) transfer->user_data;

	debug_printf("iso_callback called");
    for (i=0; i<transfer->num_iso_packets; ++i) {
		struct libusb_iso_packet_descriptor *desc =  &transfer->iso_packet_desc[i];
		unsigned char *pbuf = transfer->buffer + buf_index;
		buf_index+=desc->length;
		if (desc->actual_length != 0) {
			printf("isopacket %d received %d bytes:\n", i, desc->actual_length);
			hexdump(pbuf, desc->actual_length);
		}
		pthread_mutex_lock(&handle->lock);
		handle->packet_arrived = true;
		pthread_cond_signal(&handle->cond);
		pthread_mutex_unlock(&handle->lock);
	}
	libusb_free_transfer(transfer);
}

ssize_t
ib200_read(struct ib200_handle *handle, void *buf, size_t count)
{
	libusb_device_handle *devh = handle->devh;
	struct libusb_transfer *transfer;
	int ret;

	if ((count % 188) != 0)
		count = count - (count % 188);

	transfer = libusb_alloc_transfer(count / 188);
	if (! transfer) {
		perror("libusb_alloc_transfer");
		return -ENOMEM;
	}

	pthread_mutex_lock(&handle->lock);
	handle->packet_arrived = false;
	pthread_mutex_unlock(&handle->lock);

	libusb_fill_iso_transfer(transfer, devh, IB200_CONFIG_ENDPOINT, buf, count, count / 188, iso_callback, NULL, 0);
	ret = libusb_submit_transfer(transfer);
	if (ret) {
		debug_printf("Error submitting transfer");
		return ret;
	}

	debug_printf("aguardando reply");
	pthread_mutex_lock(&handle->lock);
	while (! handle->packet_arrived)
		pthread_cond_wait(&handle->cond, &handle->lock);
	pthread_mutex_unlock(&handle->lock);

	return 0;
}


struct user_options {
	int frequency;
	bool initialize;
	bool check_signal;
	char *writeto;
};

void
show_usage(char *appname)
{
	printf("Usage: %s <options>\n\n"
		   "Available options are:\n"
		   "  -i, --init                Initialize tuner\n"
		   "  -f, --frequency <freq>    Tune to frequency <freq>\n"
		   "  -s, --check-signal        Check signal\n"
		   "  -w  --writeto=<file>      Write transport stream packets to <file>\n"
		   "  -h, --help                This help\n"
		   , appname);

}

struct user_options *
parse_args(int argc, char **argv)
{
	struct user_options *opts, zeroed_opts;
	const char *short_options = "if:sw:h";
	struct option long_options[] = {
		{ "init", 0, 0, 0 },
		{ "frequency", 1, 0, 'f' },
		{ "check-signal", 0, 0, 0 },
		{ "writeto", 0, 0, 'w' },
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
			case 'w':
				opts->writeto = strdup(optarg);
				break;
			case 'h':
				show_usage(argv[0]);
				exit(0);
			case '?':
			default:
				exit(1);
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
	struct ib200_handle *handle;
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

	handle = ib200_open_device(dev_list, n);
	if (! handle)
		goto out_free;

	if (user_options->initialize) {
		ret = ib200_init(handle);
		if (ret < 0)
			goto out_close;
	}

	if (user_options->frequency) {
		ret = ib200_set_frequency(handle, user_options->frequency);
		if (ret < 0)
			goto out_close;
	}

	if (user_options->check_signal) {
		has_signal = ib200_has_signal(handle);
		printf("ib200_has_signal: %d\n", has_signal);
	}

	if (user_options->writeto) {
		char buf[188 * 10];
		FILE *fp = fopen(user_options->writeto, "w+");
		if (! fp) {
			perror(user_options->writeto);
			goto out_close;
		}
		while (true) {
			ret = ib200_read(handle, buf, sizeof(buf));
			printf("read %d bytes from the tuner\n", ret);
			if (ret <= 0)
				break;
			fwrite(buf, ret, sizeof(char), fp);
		}
		fclose(fp);
	}

out_close:
	ib200_close_device(handle);
out_free:
	libusb_free_device_list(dev_list, 1);
out_exit:
	libusb_exit(ctx);
	free(user_options);
	return ret;
}
