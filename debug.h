/**
 * IBayo ISDB-T 1Seg DTV USB device driver
 * debug.h - debug routines
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
#ifndef __debug_h
#define __debug_h

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

#endif /* __debug_h */
