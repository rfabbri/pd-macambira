/*
  (c) 2002:cxc@web.fm
  randomix: various PRNG's
  code taken from: http://remus.rutgers.edu/%7Erhoads/Code/code.html
  let's check it out
 */

#include <m_pd.h>
#include <stdlib.h>
#include <math.h>

#define SMALLEST_RANGE .0001

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

static unsigned random_icg_INVERSE_seed ();
static int makeseed(void);
static int rand_random1(int);
static int rand_random_fl(int);
//static void random_tw_rand_seed(t_class, int, int, int);

static int makeseed(void)
{
    static unsigned int random1_nextseed = 1489853723;
    random1_nextseed = random1_nextseed * 435898247 + 938284287;
    return (random1_nextseed & 0x7fffffff);
}

/* -------------------------- random1 ------------------------------ */
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */
static int rand_random1(seed)
{
  int state;
  
  //unsigned int a = 1588635695, m = 4294967291U, q = 2, r = 1117695901;
  //  unsigned int a = 1588635695, m = (RAND_MAX * 2), q = 2, r = 1117695901;
  
  /* static unsigned int a = 1223106847, m = 4294967291U, q = 3, r = 625646750;*/
  /* static unsigned int a = 279470273, m = 4294967291U, q = 15, r = 102913196;*/
  //static unsigned int a = 1583458089, m = 2147483647, q = 1, r = 564025558;
  static unsigned int a = 784588716, m = 2147483647, q = 2, r = 578306215;
  /* static unsigned int a = 16807, m = 2147483647, q = 127773, r = 2836;      */
  /* static unsigned int a = 950706376, m = 2147483647, q = 2, r = 246070895;  */
  
  //state = (seed) ? seed : makeseed();
  state = seed;
  state = a*(state % q) - r*(state / q);
  return (state);
}

static int rand_random_fl(seed) {
  int q;
  double state;

  /* The following parameters are recommended settings based on research
     uncomment the one you want. */
  
  double a = 1389796, m = RAND_MAX;  
  /* static double a = 950975,  m = 2147483647;  */
  /* static double a = 3467255, m = 21474836472; */
  /* static double a = 657618,  m = 4294967291;  */
  /* static double a = 93167,   m = 4294967291;  */
  /* static double a = 1345659, m = 4294967291;  */

  state = seed;
  state *= a;
  q = state / m;
  state -= q*m;
  return state;
}

/* -------------------------- random1 ------------------------------ */
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */

static t_class *random1_class;

typedef struct _random1
{
  t_object x_obj;
  t_float x_f; // lower limit
  t_float x_g; // upper limit
  unsigned int x_state; // current seed
} t_random1;

static void *random1_new(t_floatarg f, t_floatarg g)
{
    t_random1 *x = (t_random1 *)pd_new(random1_class);
    x->x_f = (f) ? f : 0;
    x->x_g = (g) ? g : RAND_MAX;
    //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
    x->x_state = makeseed();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

/* -------------------------- random_fl ------------------------------ */
/* An improved (faster) implementation of the Linear Congruential Generator. Has parameters for 6 separate */
/* linear congruence formulas. These formulas are different than those above because the previous formulas won't work */
/* correctly in this implementation. Also, this method only works if your floating point mantissa has at least 53 bits. */

/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */
/* works if your floating pt. mantissa has at least 53 bits.
   faster than other versions */
static void random1_bang(t_random1 *x)
{
    int n = x->x_f;
    double nval;
    unsigned int m;
    // this seems weird?
    m = (RAND_MAX * 2);
    //    unsigned int a = 1588635695, m = 4294967291U, q = 2, r = 1117695901;

    //    post("cxc/randomix.c: x_state: %d",x->x_state);
    x->x_state = rand_random1(x->x_state);
    nval = ((double)x->x_state / (double)m) * (double)(x->x_g - x->x_f) + (double)x->x_f;
    //    post("cxc/randomix.c: lolim: %f, uplim: %f", x->x_f, x->x_g);
    //    post("cxc/randomix.c: nval: %f",nval);
    outlet_float(x->x_obj.ob_outlet, nval);
}

void random1_low(t_random1 *x, t_floatarg f)
{
  if(f >= x->x_g) {
    post("cxc/randomix.c: lower larger than upper, setting to upper - %f = %f",
	 SMALLEST_RANGE,
	 x->x_g - SMALLEST_RANGE);
    x->x_f = x->x_g - SMALLEST_RANGE;
  } else x->x_f = f;
}

void random1_upp(t_random1 *x, t_floatarg f)
{
  if(f <= x->x_f) {
    post("cxc/randomix.c: upper smaller than lower, setting to lower + %f = %f",
	 SMALLEST_RANGE,
	 x->x_f + SMALLEST_RANGE);
    x->x_g = x->x_f + SMALLEST_RANGE;
  } else x->x_g = f;
}

static void random1_seed(t_random1 *x, float f, float glob)
{
    x->x_state = f;
}

void random1_setup(void)
{
  random1_class = class_new(gensym("random1"), (t_newmethod)random1_new, 0,
			    sizeof(t_random1), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(random1_class, random1_bang);
  class_addmethod(random1_class, (t_method)random1_low, gensym("fl1"), A_FLOAT, 0);
  class_addmethod(random1_class, (t_method)random1_upp, gensym("fl2"), A_FLOAT, 0);

  class_addmethod(random1_class, (t_method)random1_seed,
		  gensym("seed"), A_FLOAT, 0);
}


/* -------------------------- random_fl ------------------------------ */
/* An improved (faster) implementation of the Linear Congruential Generator. Has parameters for 6 separate */
/* linear congruence formulas. These formulas are different than those above because the previous formulas won't work */
/* correctly in this implementation. Also, this method only works if your floating point mantissa has at least 53 bits. */

/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */
/* works if your floating pt. mantissa has at least 53 bits.
   faster than other versions */

static t_class *random_fl_class;

typedef struct _random_fl
{
  t_object x_obj;
  t_float x_f; // lower limit
  t_float x_g; // upper limit
  unsigned int x_state; // current seed
} t_random_fl;

static void *random_fl_new(t_floatarg f, t_floatarg g)
{
    t_random_fl *x = (t_random_fl *)pd_new(random_fl_class);
    x->x_f = (f) ? f : 0;
    x->x_g = (g) ? g : RAND_MAX;
    //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
    x->x_state = makeseed();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void random_fl_bang(t_random_fl *x)
{
  int n = x->x_f;
  int q;
  double nval;
  double m;
  
  m = RAND_MAX;
  x->x_state = rand_random_fl(x->x_state);
  nval = ((x->x_state / m) * (double)(x->x_g - x->x_f) + (double)x->x_f);
  outlet_float(x->x_obj.ob_outlet, nval);
}

void random_fl_low(t_random_fl *x, t_floatarg f)
{
  if(f >= x->x_g) {
    post("cxc/randomix.c: lower larger than upper, setting to upper - %f = %f",
	 SMALLEST_RANGE,
	 x->x_g - SMALLEST_RANGE);
    x->x_f = x->x_g - SMALLEST_RANGE;
  } else x->x_f = f;
}

void random_fl_upp(t_random_fl *x, t_floatarg f)
{
  if(f <= x->x_f) {
    post("cxc/randomix.c: upper smaller than lower, setting to lower + %f = %f",
	 SMALLEST_RANGE,
	 x->x_f + SMALLEST_RANGE);
    x->x_g = x->x_f + SMALLEST_RANGE;
  } else x->x_g = f;
}

static void random_fl_seed(t_random_fl *x, float f, float glob)
{
    x->x_state = f;
}

void random_fl_setup(void)
{
  random_fl_class = class_new(gensym("random_fl"), (t_newmethod)random_fl_new, 0,
			    sizeof(t_random_fl), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(random_fl_class, random_fl_bang);
  class_addmethod(random_fl_class, (t_method)random_fl_low, gensym("fl1"), A_FLOAT, 0);
  class_addmethod(random_fl_class, (t_method)random_fl_upp, gensym("fl2"), A_FLOAT, 0);

  class_addmethod(random_fl_class, (t_method)random_fl_seed,
		  gensym("seed"), A_FLOAT, 0);
}


/* -------------------------- random_icg ------------------------------ */
/* Inverse Congruential generator. This generator is quite a bit slower than the other ones on this page. and it */
/* fails some statistical tests. The main factor in its favor is that its properties tend to be the opposite of linear congruential */
/* generators. I.e. this generator is very likely to be good for those applications where linear congruential generators are */
/* bad. You can choose among several parameters. */

/* inversive congruential generator. */

static t_class *random_icg_class;

typedef struct _random_icg
{
  t_object x_obj;
  t_float x_f; // lower limit
  t_float x_g; // upper limit
  t_float x_p; // 1st shared parameter of iter function ..
  unsigned int x_state; // current seed
} t_random_icg;

static void *random_icg_new(t_floatarg f, t_floatarg g)
{
    t_random_icg *x = (t_random_icg *)pd_new(random_icg_class);
    x->x_f = (f) ? f : 0;
    x->x_g = (g) ? g : RAND_MAX;
    x->x_p = 2147483053;
    //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
    x->x_state = makeseed();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void random_icg_bang(t_random_icg *x)
{
  int n = x->x_f;
  int p = x->x_p;
  static int a, b, q, r;
  double nval;
  
  unsigned int inv;
  inv = random_icg_INVERSE_seed(x);
  //  post("cxc/randomix.c: current inv: %d", inv);

  a = 22211, b = 11926380,q = 96685, r = 12518;
  /* static int p = 2147483053, a = 858993221, b = 1,q = 2, r = 429496611;*/
  /* static int p = 2147483053, a = 579,   b = 24456079, q = 3708951, r = 424;*/
  /* static int p = 2147483053, a = 11972, b = 62187060,q = 179375, r = 5553;*/
  /* static int p = 2147483053, a = 21714, b = 94901263,q = 98898, r = 11881;*/
  /* static int p = 2147483053, a = 4594, b = 44183289,q = 467453, r = 3971;*/
  /* static int p = 2147483647, a = 1288490188, b = 1, q = 1, r = 858993459;*/
  /* static int p = 2147483647, a = 9102, b = 36884165, q = 235935, r =3277;*/
  /* static int p = 2147483647, a = 14288, b = 758634, q = 150299, r = 11535;*/
  /* static int p = 2147483647, a = 21916, b = 71499791, q = 97987, r = 555;*/
  /* static int p = 2147483647, a = 28933, b = 59217914, q = 74222, r = 18521;*/
  /* static int p = 2147483647, a = 31152, b = 48897674, q = 68935, r = 20527;*/

  x->x_state = a*(inv % q) - r*(inv / q) + b;
  
  if (x->x_state < 0) x->x_state += x->x_p;
  else if (x->x_state >= x->x_state) x->x_state -= x->x_p;

  nval = (((x->x_state / x->x_p) - 1) * (double)(x->x_g - x->x_f) + (double)x->x_f);

  // hakc, why is it out of range?
  if(nval < (double)x->x_f) {
    random_icg_bang(x);
  } else {
    outlet_float(x->x_obj.ob_outlet, nval);
  }
}

/* Modular Inversion using the extended Euclidean alg. for GCD */
/***************************************************************/
static unsigned random_icg_INVERSE_seed (t_random_icg *x)
{
  unsigned int q,d;

  //  int p = x->x_p;
  
  signed int u,v,inv,t;
  
  if (x->x_state <= 1) return(x->x_state);
  
  d = x->x_p; inv = 0; v = 1; u = x->x_state;
  
  do {
    q = d / u;
    t = d % u;
    d = u;
    u = t;
    t = inv - q*v;
    inv = v;
    v = t;
  } while (u != 0);

  if (inv < 0 ) inv += x->x_p;

/*   if (1 != d)  */
/*     post ("inverse_iter: Can't invert !"); */

  return(inv);
}


void random_icg_low(t_random_icg *x, t_floatarg f)
{
  if(f >= x->x_g) {
    post("cxc/randomix.c: lower larger than upper, setting to upper - %f = %f",
	 SMALLEST_RANGE,
	 x->x_g - SMALLEST_RANGE);
    x->x_f = x->x_g - SMALLEST_RANGE;
  } else x->x_f = f;
}

void random_icg_upp(t_random_icg *x, t_floatarg f)
{
  if(f <= x->x_f) {
    post("cxc/randomix.c: upper smaller than lower, setting to lower + %f = %f",
	 SMALLEST_RANGE,
	 x->x_f + SMALLEST_RANGE);
    x->x_g = x->x_f + SMALLEST_RANGE;
  } else x->x_g = f;
}

static void random_icg_seed(t_random_icg *x, float f, float glob)
{
    x->x_state = f;
}

void random_icg_setup(void)
{
  random_icg_class = class_new(gensym("random_icg"), (t_newmethod)random_icg_new, 0,
			    sizeof(t_random_icg), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(random_icg_class, random_icg_bang);
  class_addmethod(random_icg_class, (t_method)random_icg_low, gensym("fl1"), A_FLOAT, 0);
  class_addmethod(random_icg_class, (t_method)random_icg_upp, gensym("fl2"), A_FLOAT, 0);

  class_addmethod(random_icg_class, (t_method)random_icg_seed,
		  gensym("seed"), A_FLOAT, 0);
}



/* -------------------------- random_tw ------------------------------ */
/* Combination of three tausworth generators. Has parameters for two different generators. Fast and excellent. */

/* Combination of 3 tausworth generators -- assumes 32-bit integers */

static t_class *random_tw_class;

typedef struct _random_tw
{
  t_object x_obj;
  t_float x_f; // lower limit
  t_float x_g; // upper limit

  t_int x_s1;
  t_int x_s2;
  t_int x_s3;

  t_int x_mask1;
  t_int x_mask2;
  t_int x_mask3;

  t_int x_shft1;
  t_int x_shft2;
  t_int x_shft3;
  t_int x_k1;
  t_int x_k2;
  t_int x_k3;

  t_int x_q1;
  t_int x_q2;
  t_int x_q3;
  t_int x_p1;
  t_int x_p2;
  t_int x_p3;

  unsigned int x_state; // current seed
} t_random_tw;

void random_tw_rand_seed (t_random_tw *x, unsigned int a, unsigned int b, unsigned int c)
{
    static unsigned int zf = 4294967295U;

    x->x_shft1 = x->x_k1 - x->x_p1;
    x->x_shft2=x->x_k2-x->x_p2;
    x->x_shft3=x->x_k3-x->x_p3;
    x->x_mask1 = zf << (32-x->x_k1);
    x->x_mask2 = zf << (32-x->x_k2);
    x->x_mask3 = zf << (32-x->x_k3);
    if (a > (1<<x->x_shft1)) x->x_s1 = a;
    if (b > (1<<x->x_shft2)) x->x_s2 = b;
    if (c > (1<<x->x_shft3)) x->x_s3 = c;
    //    rand();
}

static void *random_tw_new(t_floatarg f, t_floatarg g)
{
  t_random_tw *x = (t_random_tw *)pd_new(random_tw_class);

  x->x_f = (f) ? f : 0;
  x->x_g = (g) ? g : RAND_MAX;
  //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
  x->x_state = makeseed();
  
  x->x_s1=390451501;
  x->x_s2=613566701;
  x->x_s3=858993401;
  
  x->x_k1=31;
  x->x_k2=29;
  x->x_k3=28;
  
  x->x_q1=13;
  x->x_q2=2;
  x->x_q3=3;
  x->x_p1=12;
  x->x_p2=4;
  x->x_p3=17;
  
/*   x->x_q1=3; */
/*   x->x_q2=2; */
/*   x->x_q3=13; */
/*   x->x_p1=20; */
/*   x->x_p2=16; */
/*   x->x_p3=7; */
  
  random_tw_rand_seed(x, makeseed(),makeseed(),makeseed());
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
  outlet_new(&x->x_obj, &s_float);
  return (x);
}


static void random_tw_bang(t_random_tw *x)
{
  unsigned int b;
  double nval;
  static unsigned int zf = 4294967295U;
  
  b  = ((x->x_s1 << x->x_q1)^x->x_s1) >> x->x_shft1;
  x->x_s1 = ((x->x_s1 & x->x_mask1) << x->x_p1) ^ b;
  b  = ((x->x_s2 << x->x_q2) ^ x->x_s2) >> x->x_shft2;
  x->x_s2 = ((x->x_s2 & x->x_mask2) << x->x_p2) ^ b;
  b  = ((x->x_s3 << x->x_q3) ^ x->x_s3) >> x->x_shft3;
  x->x_s3 = ((x->x_s3 & x->x_mask3) << x->x_p3) ^ b;
  nval = (((double)(x->x_s1 ^ x->x_s2 ^ x->x_s3) / (double)(zf) + 0.5) * (double)(x->x_g - x->x_f) + (double)x->x_f);
  
  //nval = ((x->x_state / (double)m) * (double)(x->x_g - x->x_f) + (double)x->x_f);
  //post("cxc/randomix.c: current rand: %f", nval);
  outlet_float(x->x_obj.ob_outlet, nval);
}

void random_tw_low(t_random_tw *x, t_floatarg f)
{
  if(f >= x->x_g) {
    post("cxc/randomix.c: lower larger than upper, setting to upper - %f = %f",
	 SMALLEST_RANGE,
	 x->x_g - SMALLEST_RANGE);
    x->x_f = x->x_g - SMALLEST_RANGE;
  } else x->x_f = f;
}

void random_tw_upp(t_random_tw *x, t_floatarg f)
{
  if(f <= x->x_f) {
    post("cxc/randomix.c: upper smaller than lower, setting to lower + %f = %f",
	 SMALLEST_RANGE,
	 x->x_f + SMALLEST_RANGE);
    x->x_g = x->x_f + SMALLEST_RANGE;
  } else x->x_g = f;
}

static void random_tw_seed(t_random_tw *x, float f, float glob)
{
  //x->x_state = f;
  // questionable .. dont quite get how this one's seeded ..
  random_tw_rand_seed(x, f, (f*0.455777), f);
}

static void random_tw_help(t_random_tw *x)
{
  post("RAND_MAX: %d",RAND_MAX);
  post("range: %f - %f", x->x_f, x->x_g);
}

void random_tw_setup(void)
{
  random_tw_class = class_new(gensym("random_tw"), (t_newmethod)random_tw_new, 0,
			    sizeof(t_random_tw), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(random_tw_class, random_tw_bang);
  class_addmethod(random_tw_class, (t_method)random_tw_low, gensym("fl1"), A_FLOAT, 0);
  class_addmethod(random_tw_class, (t_method)random_tw_upp, gensym("fl2"), A_FLOAT, 0);

  class_addmethod(random_tw_class, (t_method)random_tw_seed,
		  gensym("seed"), A_FLOAT, 0);
  class_addmethod(random_tw_class, (t_method)random_tw_help,
		  gensym("help"), 0, 0);
}




/* -------------------------- dist_normal ------------------------------ */
/* Generate a normal random variable with mean 0 and standard deviation
   of 1.  To adjust to some other distribution, multiply by the standard
   deviation and add the mean.  Box-Muller method
   note: rand() is a function that returns a uniformly distributed random
   number from 0 to RAND_MAX
*/

static t_class *dist_normal_class;

typedef struct _dist_normal
{
  t_object x_obj;
  t_float x_mn; // mean
  t_float x_dv; // deviation
  t_float x_u1;
  t_float x_u2;

  t_float x_f; // lower limit
  t_float x_g; // upper limit
  unsigned int x_state; // current seed
} t_dist_normal;

static void *dist_normal_new(t_floatarg mn, t_floatarg dv)
{
    t_dist_normal *x = (t_dist_normal *)pd_new(dist_normal_class);
    x->x_mn = (mn) ? mn : 0;
    x->x_dv = (dv) ? dv : 1;
    x->x_u1 = 13;
    x->x_u2 = 1000;
    x->x_f =  0;
    x->x_g =  RAND_MAX;
    //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
    x->x_state = makeseed();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    //    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    //outlet_new(&x->x_obj, &s_float);
    outlet_new(&x->x_obj, 0);
    return (x);
}

static double dist_normal_rand(t_dist_normal *x)
{
  int n = x->x_f;
  double nval;
  double m;
  
  m = (double)RAND_MAX * 2;
  
    //    post("cxc/randomix.c: x_state: %d",x->x_state);
  x->x_state = rand_random_fl(x->x_state);
  //nval = ((double)x->x_state / m) * (double)(x->x_g - x->x_f) + (double)x->x_f;
  nval = (double)x->x_state;
  return (nval);
}

static void dist_normal_doit(t_dist_normal *x)
{
  static double V2, fac;
  static int phase = 0;
  double S, Z, U1, U2, V1;

  if (phase)
    Z = V2 * fac;
  else
    {
      do {
/* 	U1 = (double)rand() / RAND_MAX; */
/* 	U2 = (double)rand() / RAND_MAX; */
	U1 = (double)dist_normal_rand(x) / RAND_MAX;
	U2 = (double)dist_normal_rand(x) / RAND_MAX;

	//	post("cxc/randomix.c: test %f %f %f %f", x->x_u1, x->x_u2, U1, U2);
	
	V1 = 2 * U1 - 1;
	V2 = 2 * U2 - 1;
	S = V1 * V1 + V2 * V2;
      } while(S >= 1);
      
      fac = sqrt (-2 * log(S) / S);
      Z = V1 * fac;
    }
  
  phase = 1 - phase;
  
  //return Z;
  outlet_float(x->x_obj.ob_outlet, Z);
}

static void dist_normal_bang(t_dist_normal *x)
{
/*   post("cxc/randomix.c: dist_normal banged"); */
/*   post("cxc/randomix.c: RAND_MAX: %d",RAND_MAX); */
/*   post("cxc/randomix.c: test: %f %f", x->x_u1, x->x_u2); */
  dist_normal_doit(x);
}

void dist_normal_low(t_dist_normal *x, t_floatarg mn)
{
  x->x_mn = mn;
}

void dist_normal_upp(t_dist_normal *x, t_floatarg dv)
{
  x->x_dv = dv;
}

void dist_normal_float(t_dist_normal *x, t_floatarg r)
{
  outlet_float(x->x_obj.ob_outlet, r);
}

void dist_normal_list(t_dist_normal *x, t_symbol *s, int argc, t_atom *argv)
{ 
  outlet_list(x->x_obj.ob_outlet, s, argc, argv);
}

void dist_normal_setup(void)
{
  dist_normal_class = class_new(gensym("dist_normal"), (t_newmethod)dist_normal_new, 0,
			    sizeof(t_dist_normal), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(dist_normal_class, dist_normal_bang);
  class_addmethod(dist_normal_class, (t_method)dist_normal_low, gensym("fl1"), A_FLOAT, 0);
  //  class_addmethod(dist_normal_class, (t_method)dist_normal_upp, gensym("fl2"), A_FLOAT, 0);
  class_addlist    (dist_normal_class, dist_normal_list);

/*   class_addmethod(dist_normal_class, (t_method)dist_normal_seed, */
/* 		  gensym("seed"), A_FLOAT, 0); */
}


