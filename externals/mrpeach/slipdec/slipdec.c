/* slipdec.c 20070711 Martin Peach */
/* decode a list of SLIP-encoded bytes */
#include "m_pd.h" 

/* -------------------------- slipdec -------------------------- */
#ifndef _SLIPCODES
/* SLIP special character codes */
#define SLIP_END     0300 /* indicates end of packet */
#define SLIP_ESC     0333 /* indicates byte stuffing */
#define SLIP_ESC_END 0334 /* SLIP_ESC SLIP_ESC_END means SLIP_END data byte */
#define SLIP_ESC_ESC 0335 /* SLIP_ESC SLIP_ESC_ESC means SLIP_ESC data byte */
#define MAX_SLIP 1006 /* maximum SLIP packet size */
#define _SLIPCODES
#endif // _SLIPCODES

static t_class *slipdec_class;

typedef struct _slipdec
{
    t_object    x_obj;
    t_outlet    *x_slipdec_out;
    t_atom      *x_slip_buf;
    t_int       x_slip_length;
} t_slipdec;

static void *slipdec_new(t_symbol *s, int argc, t_atom *argv);
static void slipdec_list(t_slipdec *x, t_symbol *s, int ac, t_atom *av);
static void slipdec_free(t_slipdec *x);
void slipdec_setup(void);

static void *slipdec_new(t_symbol *s, int argc, t_atom *argv)
{
    int i;
    t_slipdec  *x = (t_slipdec *)pd_new(slipdec_class);

    x->x_slip_buf = (t_atom *)getbytes(sizeof(t_atom)*MAX_SLIP);
    if(x->x_slip_buf == NULL)
    {
        error("slipdec: unable to allocate %lu bytes for x_slip_buf", (long)sizeof(t_atom)*MAX_SLIP);
        return NULL;
    }
    else post("slipdec: allocated %lu bytes for x_slip_buf", (long)sizeof(t_atom)*MAX_SLIP);
    /* init the slip buf atoms to float type */
    for (i = 0; i < MAX_SLIP; ++i) x->x_slip_buf[i].a_type = A_FLOAT;
    x->x_slipdec_out = outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void slipdec_list(t_slipdec *x, t_symbol *s, int ac, t_atom *av)
{
    /* SLIP decode a list of bytes */
    float   f;
    int     i, c, esced = 0;

    /* for each byte in the packet, send the appropriate character sequence */
    for(i = x->x_slip_length = 0; ((i < ac) && (x->x_slip_length < MAX_SLIP)); ++i)
    {
        /* check each atom for byteness */
        f = atom_getfloat(&av[i]);
        c = (((int)f) & 0x0FF);
        if (c != f)
        {
            /* abort, bad input character */
            pd_error (x, "slipdec: input %d out of range [0..255]", f);
            return;
        }
        if(SLIP_END == c)
        {
            /* If it's the beginning of a packet, ignore it */
            if (0 == i) continue;
            /* send the packet */
            else break;
        }
        if (SLIP_ESC == c)
        {
            esced = 1;
            continue;
        }
        if (0 != esced)
        {
            if (SLIP_ESC_END == c) c = SLIP_END;
            else if (SLIP_ESC_ESC == c) c = SLIP_ESC;
            esced = 0;
        }
        /* Add the character to the list */
        x->x_slip_buf[x->x_slip_length++].a_w.w_float = c;
    }
    if (0 != x->x_slip_length)
        outlet_list(x->x_slipdec_out, &s_list, x->x_slip_length, x->x_slip_buf);
}

static void slipdec_free(t_slipdec *x)
{
    if (x->x_slip_buf != NULL) freebytes((void *)x->x_slip_buf, sizeof(t_atom)*MAX_SLIP);
}

void slipdec_setup(void)
{
    slipdec_class = class_new(gensym("slipdec"), 
        (t_newmethod)slipdec_new, (t_method)slipdec_free,
        sizeof(t_slipdec), 0, A_GIMME, 0);
    class_addlist(slipdec_class, slipdec_list);
    class_sethelpsymbol(slipdec_class, gensym("slipenc")); /* use slipenc-help.pd */
}

/* fin slipdec.c*/
