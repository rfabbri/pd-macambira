# Python script to search the udev file system for devices
# Copyright (C) 2005 Tim Blechmann
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# $Id: find_hid.py,v 1.1 2005-12-04 18:11:08 timblech Exp $
#

from os import popen, listdir

def find (*args):
	name = ""
	for token in args:
		name+=" " + str(token)
	name = name.strip()

	events = filter(lambda x: "event" in x,  listdir('/sys/class/input/'))
	
	for event in events:
		pipe = popen('udevinfo -a -p /sys/class/input/%s' % event)

		line = pipe.readline()
		while line:
			if name in line:
				print event
				return float(event.strip("event"))

			line = pipe.readline()
		pipe.close()
	return -1
