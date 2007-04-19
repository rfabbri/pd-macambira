/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef INCLUDE_IEM16_TABLE_H__
#define INCLUDE_IEM16_TABLE_H__

/* sampling */

#include "iem16.h"
#include <string.h>
/* ------------------------- table16 -------------------------- */
/* a 16bit table */

static t_class *table16_class;
typedef struct _table16 {
  t_object x_obj;

  t_symbol *x_tablename;
  long      x_size;
  t_iem16_16bit    *x_table; /* hold the data */

  int x_usedindsp;
  t_canvas *x_canvas; /* for file i/o */
} t_table16;


EXTERN int table16_getarray16(t_table16*x, int*size,t_iem16_16bit**vec);
EXTERN void table16_usedindsp(t_table16*x);


#define UNITBIT32 1572864.  /* 3*2^19; bit 32 has place value 1 */

    /* machine-dependent definitions.  These ifdefs really
    should have been by CPU type and not by operating system! */
#ifdef __irix__
    /* big-endian.  Most significant byte is at low address in memory */
# define HIOFFSET 0    /* word offset to find MSB */
# define LOWOFFSET 1    /* word offset to find LSB */
# define int32 long  /* a data type that has 32 bits */
#elif defined __WIN32__
    /* little-endian; most significant byte is at highest address */
# define HIOFFSET 1
# define LOWOFFSET 0
# define int32 long
#elif defined __FreeBSD__
# include <machine/endian.h>
# if BYTE_ORDER == LITTLE_ENDIAN
#  define HIOFFSET 1
#  define LOWOFFSET 0
# else
#  define HIOFFSET 0    /* word offset to find MSB */
#  define LOWOFFSET 1    /* word offset to find LSB */
# endif /* BYTE_ORDER */
# include <sys/types.h>
# define int32 int32_t
#elif defined __linux__
# include <endian.h>
# if !defined(__BYTE_ORDER) || !defined(__LITTLE_ENDIAN)                         
#  error No byte order defined                                                    
# endif
# if __BYTE_ORDER == __LITTLE_ENDIAN                                             
#  define HIOFFSET 1                                                              
#  define LOWOFFSET 0                                                             
# else                                                                           
#  define HIOFFSET 0    /* word offset to find MSB */                             
#  define LOWOFFSET 1    /* word offset to find LSB */                            
# endif /* __BYTE_ORDER */                                                       
# include <sys/types.h>
# define int32 int32_t
#elif defined __APPLE__
# ifdef __BIG_ENDIAN__
#  define HIOFFSET 0    /* word offset to find MSB */
#  define LOWOFFSET 1    /* word offset to find LSB */
# else
#  define HIOFFSET 1
#  define LOWOFFSET 0
# endif
# define int32 int  /* a data type that has 32 bits */
#endif /* system */

union tabfudge
{
    double tf_d;
    int32 tf_i[2];
};

#endif
