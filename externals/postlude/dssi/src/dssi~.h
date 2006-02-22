/* dssi~ - A DSSI host for PD 
 * 
 * Copyright 2006 Jamie Bullock and others 
 *
 * This file incorporates code from the following sources:
 * 
 * jack-dssi-host (BSD-style license): Copyright 2004 Chris Cannam, Steve Harris and Sean Bolton.
 *		   
 * Hexter (GPL license): Copyright (C) 2004 Sean Bolton and others.
 * 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "m_pd.h"
#include "dssi.h"
#include <dlfcn.h>
#include <lo/lo.h> 
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h> /*for exit()*/
#include <sys/types.h> /* for fork() */
#include <signal.h> /* for kill() */
#include <dirent.h> /* for readdir() */

#define DX7_VOICE_SIZE_PACKED 	128 /*From hexter_types.h by Sean Bolton */
#define DX7_DUMP_SIZE_BULK 	4096+8


#define VERSION 0.83
#define EVENT_BUFSIZE 1024
#define OSC_BASE_MAX 1024
#define TYPE_STRING_SIZE 20 /* Max size of event type string (must be two more bytes than needed) */
#define DIR_STRING_SIZE 1024 /* Max size of directory string */
#define ASCII_n 110
#define ASCII_p 112
#define ASCII_c 99
#define ASCII_b 98

#define LOADGUI 1 /* FIX: depracate this */
#define DEBUG 0
#ifdef DEBUG
	#define CHECKSUM_PATCH_FILES_ON_LOAD 1
#endif
