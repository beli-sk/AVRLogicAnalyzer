#!/usr/bin/python
# vim: ai:ts=4:sw=4:sts=4:et:fileencoding=utf-8
#
# AVRLogicAnalyzer - Download
#
# Copyright 2013 Michal Belica <devel@beli.sk>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import serial
import sys
import struct
import select
import time
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-d", "--device", dest="device",
        help="read from serial port DEVICE (required)", metavar="DEVICE")
parser.add_option("-s", "--speed", dest="speed", type="int", default=9600,
        help="serial port baud rate (default: 9600)", metavar="BAUD")
parser.add_option("-o", "--output", dest="output", default="-",
        help="output file (default: stdout)", metavar="FILE")
(options, args) = parser.parse_args()

# check for required options
for opt in ['device']:
    if opt not in options.__dict__ or options.__dict__[opt] is None:
        parser.error("parameter --%s is required" % opt)

if( options.output == '-' ):
    out = sys.stdout
else:
    out = open(options.output, 'w')

ser = serial.Serial(options.device, options.speed)
ser.flushInput()
ser.flushOutput()
sys.stderr.write("ready\n")
sys.stderr.write(str(ser.write('abc'))+'\n')
for l in ser:
    out.write(l)
    out.flush()
    if l[0:3] == 'end':
        break;
out.close()
ser.close()

