/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
/*
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef NT
#include <sys/signal.h>
#include <unistd.h>
#endif
  
#ifdef NT
#include <io.h>
#endif
*/

static t_class *iem_t3_lib_class;

static void *iem_t3_lib_new(void)
{
  t_object *x = (t_object *)pd_new(iem_t3_lib_class);
  
  return (x);
}

void sigt3_line_setup(void);
void sigt3_sig_setup(void);
void t3_bpe_setup(void);
void t3_delay_setup(void);
void t3_metro_setup(void);
void t3_timer_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_t3_lib_setup(void)
{
  iem_t3_lib_class = class_new(gensym("iem_t3_lib"), iem_t3_lib_new, 0,
    sizeof(t_object), CLASS_NOINLET, 0);
  
  sigt3_line_setup();
  sigt3_sig_setup();
  t3_bpe_setup();
  t3_delay_setup();
  t3_metro_setup();
  t3_timer_setup();
  
	post("iem_t3_lib (R-1.16) library loaded!   (c) Gerhard Eckel, Thomas Musil 05.2005");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
