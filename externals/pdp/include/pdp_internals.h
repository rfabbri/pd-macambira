
/*
 *   Pure Data Packet internal header file.
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


/* this file contains prototypes for "private" pdp methods */


/* pdp system code is not very well organized, this is an
   attempt to do better. */


#ifndef PDP_INTERNALS_H
#define PDP_INTERNALS_H

#ifdef __cplusplus
extern "C" 
{
#endif

//#include "pdp.h"
//#include <pthread.h>
//#include <unistd.h>
//#include <stdio.h>

void pdp_queue_use_thread(int t);

#ifdef __cplusplus
}
#endif


#endif 
