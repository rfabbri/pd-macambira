/* unpackOSC is like dumpOSC but outputs two lists: a list of symbols for the path  */
/* and a list of floats and/or symbols for the data  */
/* This allows for the separation of the protocol and its transport. */
/* Started by Martin Peach 20060420 */
/* This version tries to be standalone from LIBOSC MP 20060425 */
/* MP 20060505 fixed a bug (line 209) where bytes are wrongly interpreted as negative */
/* MP 20070705 added timestamp outlet */
/* dumpOSC.c header follows: */
/*
Written by Matt Wright and Adrian Freed, The Center for New Music and
Audio Technologies, University of California, Berkeley.  Copyright (c)
1992,93,94,95,96,97,98,99,2000,01,02,03,04 The Regents of the University of
California (Regents).

Permission to use, copy, modify, distribute, and distribute modified versions
of this software and its documentation without fee and without a signed
licensing agreement, is hereby granted, provided that the above copyright
notice, this paragraph and the following two paragraphs appear in all copies,
modifications, and distributions.

IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


The OSC webpage is http://cnmat.cnmat.berkeley.edu/OpenSoundControl
*/


/*

    dumpOSC.c
    server that displays OpenSoundControl messages sent to it
    for debugging client udp and UNIX protocol

    by Matt Wright, 6/3/97
    modified from dumpSC.c, by Matt Wright and Adrian Freed

    version 0.2: Added "-silent" option a.k.a. "-quiet"

    version 0.3: Incorporated patches from Nicola Bernardini to make
    things Linux-friendly.  Also added ntohl() in the right places
    to support little-endian architectures.

    to-do:

    More robustness in saying exactly what's wrong with ill-formed
    messages.  (If they don't make sense, show exactly what was
    received.)

    Time-based features: print time-received for each packet

    Clean up to separate OSC parsing code from socket/select stuff

    pd: branched from http://www.cnmat.berkeley.edu/OpenSoundControl/src/dumpOSC/dumpOSC.c
    -------------
    -- added pd functions
    -- socket is made differently than original via pd mechanisms
    -- tweaks for Win32    www.zeggz.com/raf	13-April-2002
    -- the OSX changes from cnmat didnt make it here yet but this compiles
        on OSX anyway.

*/

#if HAVE_CONFIG_H 
#include <config.h> 
#endif

#include "m_pd.h"

#ifdef _WIN32
    #include <string.h>
    #include <stdlib.h>
    #include <winsock2.h>
    #include <stdio.h>
#else
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <ctype.h>
#endif /* _WIN32 */

/* Declarations */
#ifdef WIN32
  typedef unsigned __int64 osc_time_t;
#else
  typedef unsigned long long osc_time_t;
#endif

#define MAX_MESG 65536 // was 32768 MP: make same as MAX_UDP_PACKET

/* ----------------------------- was dumpOSC ------------------------- */

#define MAX_PATH_AT 50 // maximum nuber of elements in OSC path

static t_class *unpackOSC_class;

typedef struct _unpackOSC
{
    t_object x_obj;
    t_outlet *x_data_out;
    t_outlet *x_timetag_out;
    t_atom   x_timetag[4];// timetag as four floats
    t_atom   x_data_at[MAX_MESG];// symbols making up the path + payload
    int      x_data_atc;// number of symbols to be output
    char     x_raw[MAX_MESG];// bytes making up the entire OSC message
    int      x_raw_c;// number of bytes in OSC message
} t_unpackOSC;

void unpackOSC_setup(void);
static void *unpackOSC_new(void);
static void unpackOSC_free(t_unpackOSC *x);
void unpackOSC_setup(void);
static void unpackOSC_list(t_unpackOSC *x, t_symbol *s, int argc, t_atom *argv);
static int unpackOSC_path(t_unpackOSC *x, char *path);
static void unpackOSC_Smessage(t_unpackOSC *x, void *v, int n);
static void unpackOSC_PrintTypeTaggedArgs(t_unpackOSC *x, void *v, int n);
static void unpackOSC_PrintHeuristicallyTypeGuessedArgs(t_unpackOSC *x, void *v, int n, int skipComma);
static char *unpackOSC_DataAfterAlignedString(char *string, char *boundary);
static int unpackOSC_IsNiceString(char *string, char *boundary);

static void *unpackOSC_new(void)
{
    t_unpackOSC *x;

    x = (t_unpackOSC *)pd_new(unpackOSC_class);
    x->x_data_out = outlet_new(&x->x_obj, &s_list);
    x->x_timetag_out = outlet_new(&x->x_obj, &s_list);
    x->x_raw_c = x->x_data_atc = 0;
    return (x);
}

static void unpackOSC_free(t_unpackOSC *x)
{
}

void unpackOSC_setup(void)
{
    unpackOSC_class = class_new(gensym("unpackOSC"),
        (t_newmethod)unpackOSC_new, (t_method)unpackOSC_free,
        sizeof(t_unpackOSC), 0, 0);
    class_addlist(unpackOSC_class, (t_method)unpackOSC_list);
}

/* unpackOSC_list expects an OSC packet in the form of a list of floats on [0..255] */
static void unpackOSC_list(t_unpackOSC *x, t_symbol *s, int argc, t_atom *argv) 
{
    int size, messageLen, i, j;
    char *messageName, *args, *buf;
    unsigned long timetag_s;
    unsigned long timetag_ms;
    unsigned short timetag[4];

    if ((argc%4) != 0)
    {
        post("unpackOSC: packet size (%d) not a multiple of 4 bytes: dropping packet", argc);
        return;
    }
    /* copy the list to a byte buffer, checking for bytes only */
    for (i = 0; i < argc; ++i)
    {
        if (argv[i].a_type == A_FLOAT)
        {
            j = (int)argv[i].a_w.w_float;
//          if ((j == argv[i].a_w.w_float) && (j >= 0) && (j <= 255))
// this can miss bytes between 128 and 255 because they are interpreted somewhere as negative
// , so change to this:
            if ((j == argv[i].a_w.w_float) && (j >= -128) && (j <= 255))
            {
                x->x_raw[i] = (char)j;
            }
            else
            {
                post("unpackOSC: data out of range (%d), dropping packet", argv[i].a_w.w_float);
                return;
            }
        }
        else
        {
            post("unpackOSC: data not float, dropping packet");
            return;
        }
    }
    x->x_raw_c = argc;
    buf = x->x_raw;

    if ((argc >= 8) && (strncmp(buf, "#bundle", 8) == 0))
    { /* This is a bundle message. */
#ifdef DEBUG
        post("unpackOSC: bundle msg: bundles not yet supported\n");
#endif

        if (argc < 16)
        {
            post("unpackOSC: Bundle message too small (%d bytes) for time tag", argc);
            return;
        }

        /* Print the time tag */
#ifdef DEBUG
        printf("unpackOSC: [ %lx%08lx\n", ntohl(*((unsigned long *)(buf+8))),
            ntohl(*((unsigned long *)(buf+12))));
#endif
/* split the timetag into 4 16-bit fragments so we can output them as floats */
        timetag_s = ntohl(*((unsigned long *)(buf+8)));
        timetag_ms = ntohl(*((unsigned long *)(buf+12)));
        timetag[0] = (short)(timetag_s>>8);
        timetag[1] = (short)(timetag_s & 0xFFFF);
        timetag[2] = (short)(timetag_ms>>8);
        timetag[3] = (short)(timetag_ms & 0xFFFF);
        for (i = 0; i < 4; ++i) SETFLOAT(&x->x_timetag[i], (float)timetag[i]);
        outlet_list(x->x_timetag_out, &s_list, 4, x->x_timetag);
        /* Note: if we wanted to actually use the time tag as a little-endian
          64-bit int, we'd have to word-swap the two 32-bit halves of it */

        i = 16; /* Skip "#group\0" and time tag */

        while(i < argc)
        {
            size = ntohl(*((int *) (buf + i)));
            if ((size % 4) != 0)
            {
                post("unpackOSC: Bad size count %d in bundle (not a multiple of 4)", size);
                return;
            }
            if ((size + i + 4) > argc)
            {
                post("unpackOSC: Bad size count %d in bundle (only %d bytes left in entire bundle)",
                    size, argc-i-4);
                return;	
            }

            /* Recursively handle element of bundle */
            unpackOSC_list(x, s, size, &argv[i+4]);
            i += 4 + size;
        }

        if (i != argc)
        {
            post("unpackOSC: This can't happen");
        }
#ifdef DEBUG
        printf("]\n");
#endif

    } 
    else if ((argc == 24) && (strcmp(buf, "#time") == 0))
    {
        post("unpackOSC: Time message: %s\n :).\n", buf);
        return; 	
    }
    else
    { /* This is not a bundle message or a time message */

        messageName = buf;
        args = unpackOSC_DataAfterAlignedString(messageName, buf+x->x_raw_c);
        if (args == 0)
        {
            post("unpackOSC: Bad message name string: (%s) Dropping entire message.",
            messageName);
            return;
        }
#ifdef DEBUG
        post("unpackOSC: message name string: %s", messageName);
#endif
        messageLen = args-messageName;
        /* put the OSC path into a single symbol */
        x->x_data_atc = unpackOSC_path(x, messageName);
        if (x->x_data_atc == 1) unpackOSC_Smessage(x, (void *)args, x->x_raw_c-messageLen);
    }
    if (x->x_data_atc >= 1) outlet_list(x->x_data_out, &s_list, x->x_data_atc, x->x_data_at);
    x->x_data_atc = 0;
}

static int unpackOSC_path(t_unpackOSC *x, char *path)
{
    int i;

    if (path[0] != '/')
    {
        post("unpackOSC: bad path (%s)", path);
        return 0;
    }
    for (i = 1; i < MAX_MESG; ++i)
    {
        if (path[i] == '\0')
        { /* the end of the path: turn path into a symbol */
            SETSYMBOL(&x->x_data_at[0],gensym(path));
            return 1;
        }
    }
    post("unpackOSC: path too long");
    return 0;
}
#define SMALLEST_POSITIVE_FLOAT 0.000001f

static void unpackOSC_Smessage(t_unpackOSC *x, void *v, int n)
{
    char   *chars = v;

    if (n != 0)
    {
        if (chars[0] == ',')
        {
            if (chars[1] != ',')
            {
                /* This message begins with a type-tag string */
                unpackOSC_PrintTypeTaggedArgs(x, v, n);
            }
            else
            {
                /* Double comma means an escaped real comma, not a type string */
                unpackOSC_PrintHeuristicallyTypeGuessedArgs(x, v, n, 1);
            }
        }
        else
        {
            unpackOSC_PrintHeuristicallyTypeGuessedArgs(x, v, n, 0);
        }
    }
}

static void unpackOSC_PrintTypeTaggedArgs(t_unpackOSC *x, void *v, int n)
{ 
    char    *typeTags, *thisType, *p;
    int     myargc = x->x_data_atc;
    t_atom  *mya = x->x_data_at;

    typeTags = v;

    if (!unpackOSC_IsNiceString(typeTags, typeTags+n))
    {
        /* No null-termination, so maybe it wasn't a type tag
        string after all */
        unpackOSC_PrintHeuristicallyTypeGuessedArgs(x, v, n, 0);
        return;
    }

    p = unpackOSC_DataAfterAlignedString(typeTags, typeTags+n);

    for (thisType = typeTags + 1; *thisType != 0; ++thisType)
    {
        switch (*thisType)
        {
            case 'i': case 'r': case 'm': case 'c':
#ifdef DEBUG
                post("integer: %d", ntohl(*((int *) p)));
#endif
                SETFLOAT(mya+myargc,(signed)ntohl(*((int *) p)));
                myargc++;
                p += 4;
                break;
            case 'f':
                {
                    int i = ntohl(*((int *) p));
                    float *floatp = ((float *) (&i));
#ifdef DEBUG
                    post("float: %f", *floatp);
#endif
                    SETFLOAT(mya+myargc,*floatp);
                    myargc++;
                    p += 4;
                    break;
                }
            case 'h': case 't':
#ifdef DEBUG
                printf("[A 64-bit int] ");
#endif
                post("[A 64-bit int] not implemented");
                p += 8;
                break;
            case 'd':
#ifdef DEBUG
                printf("[A 64-bit float] ");
#endif
                post("[A 64-bit float] not implemented");
                p += 8;
                break;
            case 's': case 'S':
                if (!unpackOSC_IsNiceString(p, typeTags+n))
                {
                    post("Type tag said this arg is a string but it's not!\n");
                    return;
                }
                else
                {
#ifdef DEBUG
                    post("string: \"%s\"", p);
#endif
                    SETSYMBOL(mya+myargc,gensym(p));
                    myargc++;
                    p = unpackOSC_DataAfterAlignedString(p, typeTags+n);

                }
                break;
            case 'T':
#ifdef DEBUG
                printf("[True] ");
#endif
                SETFLOAT(mya+myargc,1.);
                myargc++;
                break;
            case 'F':
#ifdef DEBUG
                printf("[False] ");
#endif
                SETFLOAT(mya+myargc,0.);
                myargc++;
                break;
            case 'N':
#ifdef DEBUG
                printf("[Nil]");
#endif
                SETFLOAT(mya+myargc,0.);
                myargc++;
                break;
            case 'I':
#ifdef DEBUG
                printf("[Infinitum]");
#endif
                SETSYMBOL(mya+myargc,gensym("INF"));
                myargc++;
                break;
            default:
                post("unpackOSC: [Unrecognized type tag %c]", *thisType);
                myargc++;
         }
    }
    x->x_data_atc = myargc;
}

static void unpackOSC_PrintHeuristicallyTypeGuessedArgs(t_unpackOSC *x, void *v, int n, int skipComma)
{
    int     i, thisi;
    int     *ints;
    float   thisf;
    char    *chars, *string, *nextString;
    int     myargc= x->x_data_atc;
    t_atom* mya = x->x_data_at;


    /* Go through the arguments 32 bits at a time */
    ints = v;
    chars = v;

    for (i = 0; i < n/4; )
    {
        string = &chars[i*4];
        thisi = ntohl(ints[i]);
        /* Reinterpret the (potentially byte-reversed) thisi as a float */
        thisf = *(((float *) (&thisi)));

        if  (thisi >= -1000 && thisi <= 1000000)
        {
#ifdef DEBUG
            printf("%d ", thisi);
#endif
            SETFLOAT(mya+myargc,(t_float) (thisi));
            myargc++;
            i++;
        }
        else if (thisf >= -1000.f && thisf <= 1000000.f &&
            (thisf <=0.0f || thisf >= SMALLEST_POSITIVE_FLOAT))
        {
#ifdef DEBUG
            printf("%f ",  thisf);
#endif
            SETFLOAT(mya+myargc,thisf);
            myargc++;
            i++;
        }
        else if (unpackOSC_IsNiceString(string, chars+n))
        {
            nextString = unpackOSC_DataAfterAlignedString(string, chars+n);
#ifdef DEBUG
            printf("\"%s\" ", (i == 0 && skipComma) ? string +1 : string);
#endif
            SETSYMBOL(mya+myargc,gensym(string));
            myargc++;
            i += (nextString-string) / 4;
        }
        else
        {
            // unhandled .. ;)
#ifdef DEBUG
            post("unpackOSC: indeterminate type: 0x%x xx", ints[i]);
#endif
            i++;
        }
        x->x_data_atc = myargc;
    }
}

#define STRING_ALIGN_PAD 4

static char *unpackOSC_DataAfterAlignedString(char *string, char *boundary) 
{
    /* The argument is a block of data beginning with a string.  The
        string has (presumably) been padded with extra null characters
        so that the overall length is a multiple of STRING_ALIGN_PAD
        bytes.  Return a pointer to the next byte after the null
        byte(s).  The boundary argument points to the character after
        the last valid character in the buffer---if the string hasn't
        ended by there, something's wrong.

        If the data looks wrong, return 0, and set htm_error_string */

    int i;

    if ((boundary - string) %4 != 0)
    {
        post("unpackOSC: DataAfterAlignedString: bad boundary");
        return 0;
    }

    for (i = 0; string[i] != '\0'; i++)
    {
        if (string + i >= boundary)
        {
            post("unpackOSC: DataAfterAlignedString: Unreasonably long string");
            return 0;
        }
    }

    /* Now string[i] is the first null character */
    i++;

    for (; (i % STRING_ALIGN_PAD) != 0; i++)
    {
        if (string + i >= boundary)
        {
            post("unpackOSC: DataAfterAlignedString: Unreasonably long string");
            return 0;
        }
        if (string[i] != '\0')
        {
            post("unpackOSC:DataAfterAlignedString: Incorrectly padded string");
            return 0;
        }
    }

    return string+i;
}

static int unpackOSC_IsNiceString(char *string, char *boundary) 
{
    /* Arguments same as DataAfterAlignedString().  Is the given "string"
       really a string?  I.e., is it a sequence of isprint() characters
       terminated with 1-4 null characters to align on a 4-byte boundary?
        Returns 1 if true, else 0. */

    int i;

    if ((boundary - string) %4 != 0)
    {
        fprintf(stderr, "Internal error: IsNiceString: bad boundary\n");
        return 0;
    }

    for (i = 0; string[i] != '\0'; i++)
        if ((!isprint(string[i])) || (string + i >= boundary)) return 0;

    /* If we made it this far, it's a null-terminated sequence of printing characters
       in the given boundary.  Now we just make sure it's null padded... */

    /* Now string[i] is the first null character */
    i++;
    for (; (i % STRING_ALIGN_PAD) != 0; i++)
        if (string[i] != '\0') return 0;

    return 1;
}

/* end of unpackOSC.c */
