/*
 * jMax
 * Copyright (C) 1994, 1995, 1998, 1999 by IRCAM-Centre Georges Pompidou, Paris, France.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * See file LICENSE for further informations on licensing terms.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * Based on Max/ISPW by Miller Puckette.
 *
 * Authors: Maurizio De Cecco, Francois Dechelle, Enzo Maggi, Norbert Schnell.
 *
 */

/* "expr" was written by Shahrokh Yadegari c. 1989. -msp */
/* Nov. 2001 - conversion for expr~ --sdy */

/*
 * vexp_func.c -- this file include all the functions for vexp.
 *		  the first two arguments to the function are the number
 *		  of argument and an array of arguments (argc, argv)
 *		  the last argument is a pointer to a struct ex_ex for
 *		  the result.  Up do this point, the content of the
 *		  struct ex_ex that these functions receive are either
 *		  ET_INT (long), ET_FLT (float), or ET_SYM (char **, it is
 *		  char ** and not char * since NewHandle of Mac returns
 *		  a char ** for relocatability.)  The common practice in
 *		  these functions is that they figure out the type of their
 *		  result according to the type of the arguments. In general
 *		  the ET_SYM is used an ET_INT when we expect a value.
 *		  It is the users responsibility not to pass strings to the
 *		  function.
 */

#include <stdlib.h>

#define __STRICT_BSD__
#include <math.h>
#undef __STRICT_BSD__


#include "vexp.h"

/* forward declarations */

static void ex_min(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_max(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_toint(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_rint(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_tofloat(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_pow(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_exp(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_log(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_ln(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_sin(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_cos(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_asin(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_acos(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_tan(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_atan(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_sinh(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_cosh(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_asinh(t_expr *expr, long argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_acosh(t_expr *expr, long argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_tanh(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_atanh(t_expr *expr, long argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_atan2(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_sqrt(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_fact(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_random(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);
static void ex_abs(t_expr *expr, long int argc, struct ex_ex *argv, struct ex_ex *optr);

t_ex_func ex_funcs[] = {
	{"min",		ex_min,		2},
	{"max",		ex_max,		2},
	{"int",		ex_toint,	1},
	{"rint",	ex_rint,	1},
	{"float",	ex_tofloat,	1},
	{"pow",		ex_pow,		2},
	{"sqrt",	ex_sqrt,	1},
	{"exp",		ex_exp,		1},
	{"log10",	ex_log,		1},
	{"ln",		ex_ln,		1},
	{"log",		ex_ln,		1},
	{"sin",		ex_sin,		1},
	{"cos",		ex_cos,		1},
	{"tan",		ex_tan,		1},
	{"asin",	ex_asin,	1},
	{"acos",	ex_acos,	1},
	{"atan",	ex_atan,	1},
	{"atan2",	ex_atan2,	2},
	{"sinh",	ex_sinh,	1},
	{"cosh",	ex_cosh,	1},
	{"tanh",	ex_tanh,	1},
	{"fact",	ex_fact,	1},
	{"random",	ex_random,	2},	/* random number */
	{"abs",		ex_abs,		1},
#ifndef NT
	{"asinh",	ex_asinh,	1},
	{"acosh",	ex_acosh,	1},
	{"atanh",	ex_atanh,	1},	/* hyperbolic atan */
#endif
#ifdef PD
	{"size",	ex_size,	1},
	{"sum",		ex_sum,		1},
	{"Sum",		ex_Sum,		3},
	{"avg",		ex_avg,		1},
	{"Avg",		ex_Avg,		3},
	{"store",	ex_store,	3},
#endif
	{0,		0,		0}
};

/*
 * FUN_EVAL --
 */
#define	FUNC_EVAL(left, right, func, leftfuncast, rightfuncast, optr) \
switch (left->ex_type) {						\
case ET_INT:								\
	switch(right->ex_type) {					\
	case ET_INT:							\
		if (optr->ex_type == ET_VEC) {				\
			op = optr->ex_vec;				\
			scalar = (float)func(leftfuncast left->ex_int,  \
					rightfuncast right->ex_int);	\
			j = e->exp_vsize;				\
			while (j--)					\
				*op++ = scalar;				\
		} else {						\
			optr->ex_type = ET_INT;				\
			optr->ex_int = (int)func(leftfuncast left->ex_int,  \
					rightfuncast right->ex_int);	\
		}							\
		break;							\
	case ET_FLT:							\
		if (optr->ex_type == ET_VEC) {				\
			op = optr->ex_vec;				\
			scalar = (float)func(leftfuncast left->ex_int,  \
					rightfuncast right->ex_flt);	\
			j = e->exp_vsize;				\
			while (j--)					\
				*op++ = scalar;				\
		} else {						\
			optr->ex_type = ET_FLT;				\
			optr->ex_flt = (float)func(leftfuncast left->ex_int,  \
					rightfuncast right->ex_flt);	\
		}							\
		break;							\
	case ET_VEC:							\
	case ET_VI:							\
		if (optr->ex_type != ET_VEC) {				\
			if (optr->ex_type == ET_VI) {			\
				post("expr~: Int. error %d", __LINE__);	\
				abort();				\
			}						\
			optr->ex_type = ET_VEC;				\
			optr->ex_vec = (t_float *)			\
			  fts_malloc(sizeof (t_float)*e->exp_vsize);	\
		}							\
		scalar = left->ex_int;					\
		rp = right->ex_vec;					\
		op = optr->ex_vec;					\
		j = e->exp_vsize;					\
		while (j--) {						\
			*op++ = (float)func(leftfuncast scalar,		\
						rightfuncast *rp);	\
			rp++;						\
		}							\
		break;							\
	case ET_SYM:							\
	default:							\
		post_error((fts_object_t *) e, 			   	\
		      "expr: FUNC_EVAL(%d): bad right type %ld\n",	\
					      __LINE__, right->ex_type);\
	}								\
	break;								\
case ET_FLT:								\
	switch(right->ex_type) {					\
	case ET_INT:							\
		if (optr->ex_type == ET_VEC) {				\
			op = optr->ex_vec;				\
			scalar = (float)func(leftfuncast left->ex_flt,  \
					rightfuncast right->ex_int);	\
			j = e->exp_vsize;				\
			while (j--)					\
				*op++ = scalar;				\
		} else {						\
			optr->ex_type = ET_INT;				\
			optr->ex_int = (int)func(leftfuncast left->ex_flt,  \
					rightfuncast right->ex_int);	\
		}							\
		break;							\
	case ET_FLT:							\
		if (optr->ex_type == ET_VEC) {				\
			op = optr->ex_vec;				\
			scalar = (float)func(leftfuncast left->ex_flt,  \
					rightfuncast right->ex_flt);	\
			j = e->exp_vsize;				\
			while (j--)					\
				*op++ = scalar;				\
		} else {						\
			optr->ex_type = ET_FLT;				\
			optr->ex_flt = (float)func(leftfuncast left->ex_flt,  \
					rightfuncast right->ex_flt);	\
		}							\
		break;							\
	case ET_VEC:							\
	case ET_VI:							\
		if (optr->ex_type != ET_VEC) {				\
			if (optr->ex_type == ET_VI) {			\
				post("expr~: Int. error %d", __LINE__);	\
				abort();				\
			}						\
			optr->ex_type = ET_VEC;				\
			optr->ex_vec = (t_float *)			\
			  fts_malloc(sizeof (t_float) * e->exp_vsize);\
		}							\
		scalar = left->ex_flt;					\
		rp = right->ex_vec;					\
		op = optr->ex_vec;					\
		j = e->exp_vsize;					\
		while (j--) {						\
			*op++ = (float)func(leftfuncast scalar,		\
						rightfuncast *rp);	\
			rp++;						\
		}							\
		break;							\
	case ET_SYM:							\
	default:							\
		post_error((fts_object_t *) e, 		   	\
		      "expr: FUNC_EVAL(%d): bad right type %ld\n",	\
					      __LINE__, right->ex_type);\
	}								\
	break;								\
case ET_VEC:								\
case ET_VI:								\
	if (optr->ex_type != ET_VEC) {					\
		if (optr->ex_type == ET_VI) {				\
			post("expr~: Int. error %d", __LINE__);		\
			abort();					\
		}							\
		optr->ex_type = ET_VEC;					\
		optr->ex_vec = (t_float *)				\
		  fts_malloc(sizeof (t_float) * e->exp_vsize);	\
	}								\
	op = optr->ex_vec;						\
	lp = left->ex_vec;						\
	switch(right->ex_type) {					\
	case ET_INT:							\
		scalar = right->ex_int;					\
		j = e->exp_vsize;					\
		while (j--) {						\
			*op++ = (float)func(leftfuncast *lp,		\
						rightfuncast scalar);	\
			lp++;						\
		}							\
		break;                                                  \
	case ET_FLT:							\
		scalar = right->ex_flt;					\
		j = e->exp_vsize;					\
		while (j--) {						\
			*op++ = (float)func(leftfuncast *lp,		\
						rightfuncast scalar);	\
			lp++;						\
		}							\
		break;                                                  \
	case ET_VEC:							\
	case ET_VI:							\
		rp = right->ex_vec;					\
		j = e->exp_vsize;					\
		while (j--) {						\
			/*						\
			 * on a RISC processor one could copy		\
			 * 8 times in each round to get a considerable	\
			 * improvement 					\
			 */						\
			*op++ = (float)func(leftfuncast *lp,		\
						rightfuncast *rp);	\
			rp++; lp++;					\
		} 							\
		break;                                                  \
	case ET_SYM:							\
	default:							\
		post_error((fts_object_t *) e, 		   	\
		      "expr: FUNC_EVAL(%d): bad right type %ld\n",	\
					      __LINE__, right->ex_type);\
	}								\
	break;								\
case ET_SYM:								\
default:								\
		post_error((fts_object_t *) e, 		   	\
		      "expr: FUNC_EVAL(%d): bad left type %ld\n",		\
					      __LINE__, left->ex_type);	\
}

/*
 * evaluate a unary operator, TYPE is applied to float operands
 */
#define FUNC_EVAL_UNARY(left, func, leftcast, optr)		\
switch(left->ex_type) {						\
case ET_INT:							\
	if (optr->ex_type == ET_VEC) {				\
		ex_mkvector(optr->ex_vec,			\
		(float)(func (leftcast left->ex_int)), e->exp_vsize);\
		break;						\
	}							\
	optr->ex_type = ET_INT;					\
	optr->ex_int = (int) func(leftcast left->ex_int);	\
	break;							\
case ET_FLT:							\
	if (optr->ex_type == ET_VEC) {				\
		ex_mkvector(optr->ex_vec,			\
		(float)(func (leftcast left->ex_flt)), e->exp_vsize);\
		break;						\
	}							\
	optr->ex_type = ET_FLT;					\
	optr->ex_flt = (float) func(leftcast left->ex_flt);	\
	break;							\
case ET_VI:							\
case ET_VEC:							\
	if (optr->ex_type != ET_VEC) {				\
		optr->ex_type = ET_VEC;				\
		optr->ex_vec = (t_float *)			\
		  fts_malloc(sizeof (t_float)*e->exp_vsize);	\
	}							\
	op = optr->ex_vec;					\
	lp = left->ex_vec;					\
	j = e->exp_vsize;					\
	while (j--)						\
		*op++ = (float)(func (leftcast *lp++));		\
	break;							\
default:							\
	post_error((fts_object_t *) e,				\
		"expr: FUNV_EVAL_UNARY(%d): bad left type %ld\n",\
				      __LINE__, left->ex_type);	\
}

#undef min
#undef max
#define min(x,y)	(x > y ? y : x)
#define max(x,y)	(x > y ? x : y)

/*
 * ex_min -- if any of the arfuments are or the output are vectors, a vector
 *	     of floats is generated otherwise the type of the result is the
 *	     type of the smaller value
 */
static void
ex_min(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left, *right;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;
	right = argv;

	/* minimum needs no cast, as it is not a real function */
	FUNC_EVAL(left, right, min, (double), (double), optr);
}

/*
 * ex_max -- if any of the arfuments are or the output are vectors, a vector
 *	     of floats is generated otherwise the type of the result is the
 *	     type of the larger value
 */
static void
ex_max(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left, *right;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;
	right = argv;

	/* minimum needs no cast, as it is not a real function */
	FUNC_EVAL(left, right, max, (double), (double), optr);
}

/* SDY changed to new form up to here */

/*
 * ex_toint -- convert to integer
 */
static void
ex_toint(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

#define toint(x)	((int)(x))
	FUNC_EVAL_UNARY(left, toint, (int), optr);
}

#ifdef NT
/* No rint in NT land ??? */
double rint(double x);

double
rint(double x)
{
	return (floor(x + 0.5));
}
#endif

/*
 * ex_rint -- rint() round to the nearest int according to the common
 *	      rounding mechanism
 */
static void
ex_rint(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;


	FUNC_EVAL_UNARY(left, rint, (double), optr);

#ifdef old

	if (argv->ex_type == ET_INT)
		*optr = *argv;
	else if (argv->ex_type == ET_FLT) {
		optr->ex_type = ET_FLT;
#ifdef NT		/* no rint() in NT??? */
		optr->ex_flt = floor(argv->ex_flt + 0.5);
#else
		optr->ex_flt = rint(argv->ex_flt);
#endif
	} else {
/* SDY what does this mean? this is wrong!!???? */
		optr->ex_type = ET_INT;
		optr->ex_int = (int)argv->ex_ptr;
	}
#endif
}

/*
 * ex_tofloat -- convert to float
 */
static void
ex_tofloat(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

#define tofloat(x)	((float)(x))
	FUNC_EVAL_UNARY(left, toint, (int), optr);
}


/*
 * ex_pow -- the power of
 */
static void
ex_pow(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left, *right;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;
	right = argv;
	FUNC_EVAL(left, right, pow, (double), (double), optr);
}

/*
 * ex_sqrt -- square root
 */
static void
ex_sqrt(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, sqrt, (double), optr);
}

/*
 * ex_exp -- e to the power of
 */
static void
ex_exp(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, exp, (double), optr);
}

/*
 * ex_log -- 10 based logarithm
 */
static void
ex_log(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, log10, (double), optr);
}

/*
 * ex_ln -- natural log
 */
static void
ex_ln(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, log, (double), optr);
}

static void
ex_sin(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, sin, (double), optr);
}

static void
ex_cos(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, cos, (double), optr);
}


static void
ex_tan(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, tan, (double), optr);
}

static void
ex_asin(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, asin, (double), optr);
}

static void
ex_acos(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, acos, (double), optr);
}


static void
ex_atan(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, atan, (double), optr);
}

/*
 *ex_atan2 --
 */
static void
ex_atan2(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left, *right;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;
	right = argv;
	FUNC_EVAL(left, right, atan2, (double), (double), optr);
}


static void
ex_sinh(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, sinh, (double), optr);
}

static void
ex_cosh(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, cosh, (double), optr);
}


static void
ex_tanh(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, tanh, (double), optr);
}


#ifndef NT
static void
ex_asinh(t_expr *e, long argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, asinh, (double), optr);
}

static void
ex_acosh(t_expr *e, long argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, acosh, (double), optr);
}

static void
ex_atanh(t_expr *e, long argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, atanh, (double), optr);
}
#endif

static int
ex_dofact(int i)
{
	int ret = 0;

	if (i)
		ret = 1;
	else
		return (0);

	do {
		ret *= i;
	} while (--i);

	return(ret);
}

static void
ex_fact(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, ex_dofact,  (int), optr);
}

static int
ex_dorandom(int i1, int i2)
{
	return(i1 + (((i2 - i1) * (rand() & 0x7fffL)) >> 15));
}
/*
 * ex_random -- return a random number
 */
static void
ex_random(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left, *right;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;
	right = argv;
	FUNC_EVAL(left, right, ex_dorandom, (int), (int), optr);
}


static void
ex_abs(t_expr *e, long int argc, struct ex_ex *argv, struct ex_ex *optr)
{
	struct ex_ex *left;
	float *op; /* output pointer */
	float *lp, *rp; 	/* left and right vector pointers */
	float scalar;
	int j;

	left = argv++;

	FUNC_EVAL_UNARY(left, fabs, (double), optr);
}

