/* This file is generated with usbsnoop2libusb_1.0.pl from a usbsnoop log file. */
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>

void print_bytes(unsigned char *bytes, int len) {
	int i;
	if (len > 0) {
	for (i=0; i<len; i++) {
		printf("%02x ", (int)(bytes[i]));
	}
	printf("\"");
		for (i=0; i<len; i++) {
		printf("%c", isprint(bytes[i]) ? bytes[i] : '.');
		}
		printf("\"");
	}
}

static void iso_callback(struct libusb_transfer *transfer){
	int i;
//	int buf_index=0;
	printf("iso_callback called :: num_iso_packets=%d, endpoint=%#x, actual_length(invalid)=%d, buffer=%p\n", 
			transfer->num_iso_packets, transfer->endpoint, transfer->actual_length, transfer->buffer);
	for (i = 0; i < transfer->num_iso_packets; i++) {
		struct libusb_iso_packet_descriptor *desc =  &transfer->iso_packet_desc[i];
		print_bytes((unsigned char *) desc, sizeof(struct libusb_iso_packet_descriptor));
		printf("\n");
#if 0
		unsigned char *pbuf = transfer->buffer + buf_index;
		buf_index+=desc->length;
		printf("    desc->length=%d, desc->actual_length=%d, desc->status=%#x\n", desc->length, desc->actual_length, desc->status);
		if (desc->actual_length != 0) {
			printf("isopacket %d received %d bytes:\n", i, desc->actual_length);
			print_bytes(pbuf, desc->actual_length);
		}
#endif
	}
	libusb_free_transfer(transfer);
}

int main(int argc, char **argv) {
	int ret, vendor, product;
	libusb_context *ctx;
	unsigned char buf[65535];
	unsigned char isobuf[65536];
	static struct libusb_device_handle *devh = NULL;
	struct libusb_transfer* transfer;

	if (argc!=3) {
		printf("usage: %s vendorID productID\n", argv[0]);
		exit(1);
	}

	char *endptr;
	vendor = strtol(argv[1], &endptr, 16);
	if (*endptr != '\0') {
		printf("invalid vendor id\n");
		exit(1);
	}
	product = strtol(argv[2], &endptr, 16);
	if (*endptr != '\0') {
		printf("invalid product id\n");
		exit(1);
	}


	ret = libusb_init(&ctx);
	if (ret < 0)
		return ret;

	libusb_set_debug(ctx, 3);

	devh = libusb_open_device_with_vid_pid(NULL, vendor, product);

	ret = libusb_get_descriptor(devh, LIBUSB_DT_DEVICE, 0x0000000, buf, 0x0000012);
	printf("1 get descriptor returned %d bytes: \n ", ret);
	print_bytes(buf, ret);
	printf("\n");
	usleep(0*1000);
	ret = libusb_get_descriptor(devh, LIBUSB_DT_CONFIG, 0x0000000, buf, 0x0000009);
	printf("2 get descriptor returned %d bytes: \n ", ret);
	print_bytes(buf, ret);
	printf("\n");
	usleep(1*1000);
	ret = libusb_get_descriptor(devh, LIBUSB_DT_CONFIG, 0x0000000, buf, 0x0000042);
	printf("3 get descriptor returned %d bytes: \n ", ret);
	print_bytes(buf, ret);
	printf("\n");
	usleep(0*1000);
	ret = libusb_release_interface(devh, 0);
	if (ret != 0) printf("failed to release interface before set_configuration: %d\n", ret);
	ret = libusb_set_configuration(devh, 0x0000001);
	printf("4 set configuration returned %d\n", ret);
	ret = libusb_claim_interface(devh, 0);
	if (ret != 0) printf("claim after set_configuration failed with error %d\n", ret);
	ret = libusb_set_interface_alt_setting(devh,	0, 0);
	printf("4 set alternate setting returned %d\n", ret);
	usleep(0*1000);

	// ib200_init_configuration_descriptor()


#define OUT_USB(data, n)\
	memcpy(buf, data, 0x000000d);\
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE, 0x0000001, 0x000000b, 0x0000000, buf, 0x000000d, 1000);\
	printf("control msg returned %d bytes: \n ", ret);\
	print_bytes(buf, ret);\
	printf("\n");\
	usleep(n*1000);

#define IN_USB(a,b,c,d, n)\
	ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR + LIBUSB_RECIPIENT_DEVICE + LIBUSB_ENDPOINT_IN, a,  b, c, buf, d, 1000);\
	printf("returned %d bytes: \n ", ret);\
	print_bytes(buf, ret);\
	printf("\n");\
	usleep(n*1000);

	ret = libusb_set_interface_alt_setting(devh,	0, 1);
	printf("70 set alternate setting returned %d\n", ret);
	usleep(18*1000);

	// start of ISOCH transfers
	transfer = libusb_alloc_transfer(64);
	libusb_fill_iso_transfer(transfer, devh, 0x00000082, isobuf, 940 * 64, 64, iso_callback, NULL, 10000);
	ret = libusb_submit_transfer(transfer);
	if (ret) printf("111 isochronous submit returned %d\n", ret);

//	OUT_USB("\x0b\x00\x00\x82\x01\x30\x80\x00\x00\x00\x00\x00\x5a", 0)
	OUT_USB("\x0b\x00\x00\x82\x01\x16\x00\x00\xa8\x6d\x0d\x89\x43", 16)
	OUT_USB("\x0b\x00\x20\x82\x01\x15\x80\x00\x1c\x0b\x00\x00\x74", 0)

	while (42){}

	libusb_close(devh);
	libusb_exit(NULL);
	return 0;
}
