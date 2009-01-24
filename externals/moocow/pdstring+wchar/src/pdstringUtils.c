/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: pdstringUtils.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: pdstring: common utilities
 *
 * Copyright (c) 2009 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file "COPYING", in this distribution.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#ifndef PDSTRING_UTILS_C
#define PDSTRING_UTILS_C

#include <string.h>
#include <m_pd.h>
#include <stdlib.h>
#include "mooPdUtils.h"

/*=====================================================================
 * Debugging
 *=====================================================================*/
#define PDSTRING_UTILS_DEBUG 1
//#undef  PDSTRING_UTILS_DEBUG

#ifdef PDSTRING_UTILS_DEBUG
# define PDSDEBUG(x) x
#else
# define PDSDEBUG(x)
#endif

/*=====================================================================
 * Constants
 *=====================================================================*/

/* PDSTRING_EOS_NONE
 *  + "safe" float value to use as x_eos if no truncation is desired
 */
#define PDSTRING_EOS_NONE 1e38f

/* PDSTRING_DEFAULT_BUFLEN
 *  + common default buffer length
 */
#define PDSTRING_DEFAULT_BUFLEN 256

/* PDSTRING_DEFAULT_GET
 *  + common default buffer grow length
 */
#define PDSTRING_DEFAULT_GET 256

/* PDSTRING_BYSTES_GET
 *  + number of extra bytes to get when buffer must grow
 */
#define PDSTRING_BYTES_GET PDSTRING_DEFAULT_GET
#define PDSTRING_WCHARS_GET PDSTRING_DEFAULT_GET
#define PDSTRING_ATOMS_GET PDSTRING_DEFAULT_GET

/*=====================================================================
 * Structures & Types
 *=====================================================================*/

/* t_pdstring_bytes
 *  + a byte-string buffer
 */
typedef struct _pdstring_bytes {
  unsigned char *b_buf;    //-- byte-string buffer
  int            b_len;    //-- current length of b_buf
  size_t         b_alloc;  //-- allocated size of b_buf
} t_pdstring_bytes;

/* t_pdstring_wchars
 *  + a wide character buffer
 */
typedef struct _pdstring_wchars {
  wchar_t       *w_buf;    //-- wide character buffer
  int            w_len;    //-- current length of w_buf
  size_t         w_alloc;  //-- allocated size of w_buf
} t_pdstring_wchars;

/* t_pdstring_atoms
 *  + an atom-list buffer
 */
typedef struct _pdstring_atoms {
  t_atom        *a_buf;    //-- t_atom buffer (aka argv)
  int            a_len;    //-- current length of a_buf (aka argc)
  size_t         a_alloc;  //-- allocated size of a_buf
} t_pdstring_atoms;

/*=====================================================================
 * Initialization
 *=====================================================================*/

//-- bytes
static void pdstring_bytes_clear(t_pdstring_bytes *b)
{
  if (b->b_alloc) freebytes(b->b_buf, (b->b_alloc)*sizeof(unsigned char));
  b->b_buf   = NULL;
  b->b_len   = 0;
  b->b_alloc = 0;
}
static void pdstring_bytes_realloc(t_pdstring_bytes *b, size_t n)
{
  pdstring_bytes_clear(b);
  b->b_buf   = n ? (unsigned char*)getbytes(n*sizeof(unsigned char)) : NULL;
  b->b_alloc = n;
}
static void pdstring_bytes_init(t_pdstring_bytes *b, size_t n)
{
  pdstring_bytes_clear(b);
  pdstring_bytes_realloc(b,n);
}

//-- wchars
static void pdstring_wchars_clear(t_pdstring_wchars *w)
{
  if (w->w_alloc) freebytes(w->w_buf, (w->w_alloc)*sizeof(wchar_t));
  w->w_buf   = NULL;
  w->w_len   = 0;
  w->w_alloc = 0;
}
static void pdstring_wchars_realloc(t_pdstring_wchars *w, size_t n)
{
  pdstring_wchars_clear(w);
  w->w_buf   = n ? (wchar_t*)getbytes(n*sizeof(wchar_t)) : NULL;
  w->w_alloc = n;
}
static void pdstring_wchars_init(t_pdstring_wchars *w, size_t n)
{
  pdstring_wchars_clear(w);
  pdstring_wchars_realloc(w,n);
}

//-- atoms
static void pdstring_atoms_clear(t_pdstring_atoms *a)
{
  if (a->a_alloc) freebytes(a->a_buf, (a->a_alloc)*sizeof(t_atom));
  a->a_buf   = NULL;
  a->a_len   = 0;
  a->a_alloc = 0;
}
static void pdstring_atoms_realloc(t_pdstring_atoms *a, size_t n)
{
  pdstring_atoms_clear(a);
  a->a_buf   = n ? (t_atom*)getbytes(n*sizeof(t_atom)) : NULL;
  a->a_alloc = n;
}
static void pdstring_atoms_init(t_pdstring_atoms *a, size_t n)
{
  pdstring_atoms_clear(a);
  pdstring_atoms_realloc(a,n);
}


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pdstring_atoms2bytes()
 *  + always appends a final NUL byte to *dst_buf, even if src_argv doesn't contain one
 *  + returns number of bytes actually written to *dst_buf, __including__ implicit trailing NUL
 */
static int pdstring_atoms2bytes(t_pdstring_bytes *dst, //-- destination byte buffer
				t_pdstring_atoms *src, //-- source t_atom float list
				t_float x_eos)         //-- EOS byte value: stop if reached (negative ~ 0)
{
  t_atom *argv = src->a_buf;
  int     argc = src->a_len;
  unsigned char *s;
  int     new_len=0;

  /*-- re-allocate? --*/
  if (dst->b_alloc <= (argc+1))
    pdstring_bytes_realloc(dst, argc + 1 + PDSTRING_BYTES_GET);

  /*-- get byte string --*/
  for (s=dst->b_buf, new_len=0; argc > 0; argc--, argv++, s++, new_len++)
    {
      *s = atom_getfloat(argv);
      if ((x_eos<0 && !*s) || (*s==x_eos)) { break; } /*-- hack: truncate at first EOS char --*/
    }
  *s = '\0'; /*-- always append terminating NUL */
  dst->b_len = new_len;

  return new_len+1;
}

/*--------------------------------------------------------------------
 * pdstring_bytes2any()
 *  + uses x_binbuf for conversion
 *  + x_binbuf may be NULL, in which case a temporary t_binbuf is created and used
 *    - in this case, output atoms are copied into *dst, reallocating if required
 *  + if x_binbuf is given and non-NULL, dst may be NULL.
 *    - if dst is non-NULL, its values will be clobbered by those returned by
 *      binbuf_getnatom() and binbuf_getvec()
 */
static void pdstring_bytes2any(t_pdstring_atoms *dst, t_pdstring_bytes *src, t_binbuf *x_binbuf)
{
  int bb_is_tmp=0;

  //-- create temporary binbuf?
  if (!x_binbuf) {
    x_binbuf = binbuf_new();
    bb_is_tmp = 1;
  }

  //-- populate binbuf
  binbuf_clear(x_binbuf);
  binbuf_text(x_binbuf, (char*)src->b_buf, src->b_len);
  //PDSDEBUG(post("bytes2any[dst=%p,src=%p,bb=%p]: binbuf_print: ", dst,src,x_binbuf));
  //PDSDEBUG(binbuf_print(x_binbuf));

  //-- populate atom list
  if (bb_is_tmp) {
    //-- temporary binbuf: copy atoms
    t_atom *argv = binbuf_getvec(x_binbuf);
    int     argc = binbuf_getnatom(x_binbuf);

    //-- reallocate?
    if ( dst->a_alloc < argc )
      pdstring_atoms_realloc(dst, argc + PDSTRING_ATOMS_GET);


    //-- copy
    memcpy(dst->a_buf, argv, argc*sizeof(t_atom));
    dst->a_len = argc;

    //-- cleanup
    binbuf_free(x_binbuf);
  }
  else if (dst) {
    //-- permanent binbuf: clobber dst
    dst->a_buf = binbuf_getvec(x_binbuf);
    dst->a_len = binbuf_getnatom(x_binbuf);
    dst->a_alloc = 0;  //-- don't try to free this
  }
}

/*--------------------------------------------------------------------
 * pdstring_bytes2wchars()
 */
static int pdstring_bytes2wchars(t_pdstring_wchars *dst, t_pdstring_bytes *src)
{
  /*
  //-- get required length
  int nwc=0, size;
  unsigned char *s;
  for (s=src->b_buf, size=src->b_len, nwc=0; size>0; nwc++) {
    int csize = mblen(s,size);
    if (csize < 0) {
      error("pdstring_bytes2wchars(): could not compute length for byte-string \"%s\" - skipping a byte!");
      s++;
      continue;
    }
    s += csize;
  }
  */

  //-- re-allocate?
  if ( dst->w_alloc < src->b_len )
    pdstring_wchars_realloc(dst, src->b_len + PDSTRING_WCHARS_GET);

  //-- convert
  size_t newlen = mbstowcs(dst->w_buf, (char*)src->b_buf, src->b_len);
  if (newlen==((size_t)-1)) {
    error("pdstring_bytes2wchars(): could not convert multibyte string \"%s\"", src->b_buf);
  }
  dst->w_len = newlen;

  return (newlen < src->b_len ? newlen : newlen+1);
}

/*--------------------------------------------------------------------
 * pdstring_wchars2atoms()
 */
static void pdstring_wchars2atoms(t_pdstring_atoms *dst, t_pdstring_wchars *src)
{
  int i;

  //-- re-allocate?
  if ( dst->a_alloc < src->w_len )
    pdstring_atoms_realloc(dst, src->w_len + PDSTRING_ATOMS_GET);

  //-- convert
  for (i=0; i < src->w_len; i++) {
    SETFLOAT((dst->a_buf+i), src->w_buf[i]);
  }
  dst->a_len = src->w_len;
}


#endif /* PDSTRING_UTILS_C */
