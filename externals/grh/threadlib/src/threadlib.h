/* 
* 
* threadlib
* library for threaded patching in PureData
* Copyright (C) 2005 Georg Holzmann <grh@mur.at>
* heavily based on code by Tim Blechmann
* (detach, join, pd_devel)
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#ifndef __PD_THREADLIB_H_
#define __PD_THREADLIB_H_

#include "m_pd.h"
#include "pthread.h"


// threadlib version string
#define VERSION "0.1pre"

// define it to use the lockfree fifo
#define THREADLIB_LOCKFREE

// for debuging
//#define DEBUG


/* --------- lockfree FIFO of pd devel ----------- */
// (implemted in fifo.c)

/* used data structures */
EXTERN_STRUCT _fifo;
#define t_fifo struct _fifo

/* function prototypes */
t_fifo * fifo_init(void);
void fifo_destroy(t_fifo*);

/* fifo_put() and fifo_get are the only threadsafe functions!!! */
void fifo_put(t_fifo*, void*);
void* fifo_get(t_fifo*);


/* --------- callback FIFO of pd devel ----------- */
// (implemted in callbacks.c)

/* NOTE: difference to pd_devel
 * in pd_devel the callbacks are called in idle time
 * (idle callbacks), because this is not possible
 * in current pd, they are called here by the
 * clock callbacks
 */

/* set up callback fifo and start clock callback */
void h_init_callbacks();

/* free fifo and clock callback */
void h_free_callbacks();

/* tb: to be called at idle time */
/* Holzmann: idle callbacks of current PD are not reliable, so
             it will be called by the clock-callbacks for now */
/* register a new callback in FIFO */
void h_set_callback(t_int (*callback) (t_int* argv), t_int* argv, t_int argc);


#endif // __PD_THREADLIB_H_
