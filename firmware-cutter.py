#!/usr/bin/env python

#
# Firmware cutter for ZTUB10.sys.
# Should serve as reference for extracting the firmware from ZTUB20.sys,
# ZTUB21.sys, ZTUC10.sys and ZTUC13.sys, provided that we get a USB hotplug
# dump from those devices.
#
# Copyright (c) 2010, Lucas C. Villa Real <lucasvr@gobolinux.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
#  any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

import os

class FirmwareCutter :
	def __init__(self, filename) :
		self.filename = filename
		self.fp = None
		self.outfp = None

	def extractFirmware(self, outfile) :
		self.fp = open(self.filename)
		if not self.fp :
			print "Error opening file %s" %self.filename
			return None

		self.outfp = open(outfile, "w+")
		if not self.outfp :
			print "Error creating file %s" %outfile
			self.fp.close()
			return None

		buf = ""
		while True :
			data = self.fp.read(1024*1024)
			if not data :
				break
			buf += data

		magic = buf.find("\x44\x4e\x00\x00\x44\x4e\x00\x00")
		if magic == -1 :
			print "Could not find the firmware signature in this driver"
			self.outfp.close()
			self.fp.close()
			return None

		print "Found the firmware at offset %d" %(magic+8)
		firmware = buf[magic+8:magic+8+56]
		self.outfp.write(firmware)

		self.outfp.close()
		self.fp.close()


if len(os.sys.argv) != 3 :
	print "Syntax: %s <driver.sys> <firmware.out>" %os.sys.argv[0]
	os.sys.exit(1)

fc = FirmwareCutter(os.sys.argv[1])
firmware = fc.extractFirmware(os.sys.argv[2])
