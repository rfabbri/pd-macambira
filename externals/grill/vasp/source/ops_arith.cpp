/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "ops_arith.h"
#include "opdefs.h"
#include "util.h"
#include <math.h>


VASP_BINARY("vasp.+",add,true,VASP_ARG_R(0),"Adds a value, envelope or vasp")
VASP_BINARY("vasp.-",sub,true,VASP_ARG_R(0),"Subtracts a value, envelope or vasp")
VASP_BINARY("vasp.!-",subr,true,VASP_ARG_R(0),"Reverse subtracts a value, envelope or vasp")
VASP_BINARY("vasp.*",mul,true,VASP_ARG_R(1),"Multiplies with a value, envelope or vasp")
VASP_BINARY("vasp./",div,true,VASP_ARG_R(1),"Divides by a value, envelope or vasp")
VASP_BINARY("vasp.!/",divr,true,VASP_ARG_R(1),"Reverse divides by a value, envelope or vasp")
VASP_BINARY("vasp.%",mod,true,VASP_ARG_R(0),"Calculates the remainder of the division by a value, envelope or vasp")

// -----------------------------------------------------

VASP_UNARY("vasp.sqr",sqr,true,"Calculates the square") 
VASP_UNARY("vasp.ssqr",ssqr,true,"Calculates the square with preservation of the sign") 

// -----------------------------------------------------

VASP_UNARY("vasp.sign",sign,true,"Calculates the sign (signum function)") 
VASP_UNARY("vasp.abs",abs,true,"Calulates the absolute value") 

