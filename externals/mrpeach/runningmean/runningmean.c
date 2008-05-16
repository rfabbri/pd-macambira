/* runningmean.c MP 20080516 */
/* output the running mean of the input */
#include "m_pd.h"

/* We implement a circular buffer x_data of length x_n and */
/* add all the values in it and divide by the */
/* number of values to get the mean x_mean. */

/* for simplicity and to avoid reallocation problems, we preallocate a longish array for the data */
#define RUNNINGMEAN_MAX 1024 /* arbitrary maximum length of the data list*/

typedef struct _runningmean
{
    t_object        x_obj;
    t_int           x_in1;
    t_int           x_in2;
    t_int           x_in3;
    t_outlet        *x_out;
    t_inlet         *x_inlet2;
    t_int           x_n;
    t_float         x_data[RUNNINGMEAN_MAX];
    t_float         x_mean;
    t_int           x_pointer;
} t_runningmean;

static t_class *runningmean_class;

void runningmean_setup(void);
static void *runningmean_new(t_symbol *s, t_floatarg f);
static void runningmean_free(t_runningmean *x);
static void runningmean_bang(t_runningmean *x);
static void runningmean_float(t_runningmean *x, t_float f);
static void runningmean_length(t_runningmean *x, t_float f);
static void runningmean_zero(t_runningmean *x);

static void runningmean_float(t_runningmean *x, t_float f)
{
    float   *p = x->x_data;
    float   total = 0;
    int     i;

    /* add a float at the current location, overwriting the oldest data */
    x->x_data[x->x_pointer] = f;
    if (++x->x_pointer >= x->x_n) x->x_pointer = 0; /* wrap pointer */
    for (i = 0; i < x->x_n; ++i) total += *p++;
    x->x_mean = total/x->x_n;
    outlet_float(x->x_out, x->x_mean);
    return;
}

static void runningmean_bang(t_runningmean *x)
{
    outlet_float(x->x_out, x->x_mean);
    return;
}

static void runningmean_length(t_runningmean *x, t_float f)
{

    if ((f >= 1) && ((int)f == f) && (f < RUNNINGMEAN_MAX))
    {
        x->x_n = (int)f;
        runningmean_zero(x);
    }
    else post("runningmean length must be an integer between 1 and %d.", RUNNINGMEAN_MAX);
    return;
}

static void runningmean_zero(t_runningmean *x)
{
    float   *p = x->x_data;
    int     i;

    /* zero the entire array */
    for (i = 0; i < RUNNINGMEAN_MAX; ++i) *p++ = 0;
    x->x_mean = 0;
    x->x_pointer = 0;
    return;
}


static void runningmean_free(t_runningmean *x)
{
    return;
}

static void *runningmean_new(t_symbol *s, t_floatarg f)
{
    t_runningmean   *x;

    post("runningmean_new %f", f);

    x = (t_runningmean *)pd_new(runningmean_class);
    if (x == NULL) return (x);
    x->x_out = outlet_new((t_object *)x, &s_float);
    x->x_inlet2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("length"));
    if (!((f >= 1) && ((int)f == f) && (f < RUNNINGMEAN_MAX)))
    {
        post("runningmean length %f must be an integer between 1 and %d, using %d", f, RUNNINGMEAN_MAX, RUNNINGMEAN_MAX);
        f = RUNNINGMEAN_MAX;
    }
    {
        x->x_n = (int)f;
        runningmean_zero(x);
    }
    return (x);
}

void runningmean_setup(void)
{
    runningmean_class = class_new
    (
        gensym("runningmean"),
        (t_newmethod)runningmean_new,
        (t_method)runningmean_free,
        sizeof(t_runningmean),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        0
    ); /* one argument for length */
    class_addbang(runningmean_class, runningmean_bang);
    class_addfloat(runningmean_class, runningmean_float);
    class_addmethod(runningmean_class, (t_method)runningmean_length, gensym("length"), A_FLOAT, 0);
    class_addmethod(runningmean_class, (t_method)runningmean_zero, gensym("clear"), 0);
}
/* end runningmean.c */

