/* slipdec.c 20100513 Martin Peach */
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
    t_outlet    *x_status_out;
    t_atom      *x_slip_buf;
    t_int       x_slip_length;
    t_int       x_packet_index;
    t_int       x_valid_SLIP;
    t_int       x_esced;
    t_int       x_verbose;
} t_slipdec;

static void *slipdec_new(t_symbol *s, int argc, t_atom *argv);
static void slipdec_dump(t_slipdec *x, int dosend);
static void slipdec_list(t_slipdec *x, t_symbol *s, int ac, t_atom *av);
static void slipdec_float(t_slipdec *x, t_float f);
static void slipdec_verbosity(t_slipdec *x, t_float f);
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
    /* init the slip buf atoms to float type */
    for (i = 0; i < MAX_SLIP; ++i) x->x_slip_buf[i].a_type = A_FLOAT;
    x->x_slipdec_out = outlet_new(&x->x_obj, &s_list);
    x->x_status_out = outlet_new(&x->x_obj, &s_anything);
    x->x_packet_index = 0;
    x->x_valid_SLIP = 1;
    return (x);
}

static void slipdec_dump(t_slipdec *x, int dosend)
{
    if(dosend)
    {
        if ((0 != x->x_valid_SLIP) && (x->x_slip_length > 0)) 
        outlet_list(x->x_slipdec_out, &s_list, x->x_slip_length, x->x_slip_buf);
    }

    x->x_slip_length = x->x_esced = x->x_packet_index = 0;
    x->x_valid_SLIP = 1;
}

static void slipdec_list(t_slipdec *x, t_symbol *s, int ac, t_atom *av)
{
    /* SLIP decode a list of bytes */
    float   f;
    int     i = 0, c;

    /* x_slip_length will be non-zero if an incomplete packet is in the buffer */
    if ((ac + x->x_slip_length) > MAX_SLIP)
    {
        pd_error (x, "slipdec_list: input packet longer than %d", MAX_SLIP);
        x->x_slip_length = x->x_esced = x->x_packet_index = 0;
        return;
    }
    /* for each byte in the packet, send the appropriate character sequence */
    for(; ((i < ac) && (x->x_slip_length < MAX_SLIP)); ++i)
    {
        /* check each atom for byteness */
        f = atom_getfloat(&av[i]);
        c = (((int)f) & 0x0FF);
        if (c != f)
        {
            /* abort, input list needs to be fixed before this is gonna wuk */
            pd_error (x, "slipdec: input %d out of range [0..255]", f);
            return;
        }
        if(SLIP_END == c)
        {
            /* If it's the beginning of a packet, ignore it */
            if (x->x_slip_length)
            {
                /* send the packet */
                slipdec_dump(x, 1);
            }
            continue;
        }
        if (SLIP_ESC == c)
        {
            x->x_esced = 1;
            continue;
        }
        if (1 == x->x_esced)
        {
            if (SLIP_ESC_END == c) c = SLIP_END;
            else if (SLIP_ESC_ESC == c) c = SLIP_ESC;
            else x->x_valid_SLIP = 0; /* not valid SLIP */
            x->x_esced = 0;
        }
        /* Add the character to the list */
        x->x_slip_buf[x->x_slip_length++].a_w.w_float = c;
    }
    if (0 != x->x_slip_length)
    {
        if(SLIP_END != c) x->x_valid_SLIP = 0;
        outlet_float(x->x_status_out, x->x_valid_SLIP);
        if (0 != x->x_valid_SLIP) outlet_list(x->x_slipdec_out, &s_list, x->x_slip_length, x->x_slip_buf);
        x->x_slip_length = x->x_esced = x->x_packet_index = 0;
        x->x_valid_SLIP = 1;
        /* any remaining data in the list is ignored for now... */
        if (i < ac-1) post("slipdec_list: dropped %d bytes after packet", ac-1-i);
    }
}

static void slipdec_float(t_slipdec *x, t_float f)
{
    /* SLIP decode a byte */
    int         c;
    /* for each byte in the packet, send the appropriate character sequence */
    /* check each atom for byteness */
    c = (((int)f) & 0x0FF);
    if (c != f)
    {
        /* abort, input list needs to be fixed before this is gonna wuk */
        pd_error (x, "slipdec: input %d out of range [0..255]", f);
        x->x_slip_length = x->x_esced = x->x_packet_index = 0;
        x->x_valid_SLIP = 1;
        return;
    }
    if(SLIP_END == c)
    {
        if (x->x_verbose) post ("slipdec_float: SLIP_END packet index is %d", x->x_packet_index);
        /* If it's the beginning of a packet, ignore it */
        if (0 == x->x_packet_index) return;
        /* send the packet */
        else
        {
            if (x->x_verbose) post ("slipdec_float: end of packet");
            outlet_float(x->x_status_out, x->x_valid_SLIP);
            if ((0 != x->x_slip_length) && (0 != x->x_valid_SLIP))
                outlet_list(x->x_slipdec_out, &s_list, x->x_slip_length, x->x_slip_buf);
            x->x_slip_length = x->x_esced = x->x_packet_index = 0;
            x->x_valid_SLIP = 1;
            return;
        }
    }
    if (SLIP_ESC == c)
    {
        if (x->x_verbose) post ("slipdec_float: SLIP_ESC %f = %d", f, c);
        x->x_esced = 1;
        return;
    }
    if (1 == x->x_esced)
    {
        if (SLIP_ESC_END == c) c = SLIP_END;
        else if (SLIP_ESC_ESC == c) c = SLIP_ESC;
        else x->x_valid_SLIP = 0; /* not valid SLIP */
        if (x->x_verbose) post ("slipdec_float: ESCED %f = %d", f, c);
        x->x_esced = 0;
    }
    /* Add the character to the list */
    if (0 == x->x_packet_index++) x->x_slip_length = 0;
    if (x->x_slip_length < MAX_SLIP)
    {
        x->x_slip_buf[x->x_slip_length++].a_w.w_float = c;
    }
    else
    {
        pd_error (x, "slipdec: input packet longer than %d", x->x_slip_length);
        x->x_slip_length = x->x_esced = x->x_packet_index = 0;
    }
}

static void slipdec_verbosity(t_slipdec *x, t_float f)
{
    x->x_verbose = (0 != f)?1:0;
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
    class_addfloat(slipdec_class, slipdec_float);
    class_addmethod(slipdec_class, (t_method)slipdec_verbosity, gensym("verbosity"), A_FLOAT, 0);
    class_sethelpsymbol(slipdec_class, gensym("slipenc")); /* use slipenc-help.pd */
}

/* fin slipdec.c*/
