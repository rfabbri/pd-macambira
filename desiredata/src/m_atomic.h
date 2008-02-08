/* Copyright (c) 2005, Tim Blechmann
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.  */


#if defined(__GNUC__) && (defined(_X86_) || defined(__i386__) || defined(__i586__) || defined(__i686__))

/* gcc, x86 */
#define ATOMIC_INC(X)							\
   asm __volatile__("lock incl (%0) \n"			\
	   : :"r"(X) :"memory")
 

#define ATOMIC_DEC(X)							\
   asm __volatile__("lock decl (%0) \n"			\
	   : :"r"(X) :"memory")



#elif defined(NT) && defined(_MSC_VER)

/* msvc */
#include <windows.h>
#define ATOMIC_INC(X) InterlockedIncrement(X)
#define ATOMIC_DEC(X) InterlockedDecrement(X)


#elif defined(__GNUC__) && defined(__POWERPC__)

/* ppc */
#define ATOMIC_INC(X) {							\
int X##_i;										\
asm __volatile__(								\
				 "1: \n"						\
				 "lwarx %0, 0, %2   \n"			\
				 "addic %0, %0, 1   \n"			\
				 "stwcx. %0, 0, %2   \n"			\
				 "bne-  1b          \n"			\
				 :"=&r"(X##_i), "=m"(X)			\
				 : "r" (&X), "m"(X)				\
				 : "cc"); }
    

#define ATOMIC_DEC(X) {							\
int X##_i;										\
asm __volatile__(								\
				 "1: \n"						\
				 "lwarx %0, 0, %2   \n"			\
				 "addic %0, %0, -1  \n"			\
				 "stwcx. %0, 0, %2   \n"			\
				 "bne-  1b          \n"			\
				 :"=&r"(X##_i), "=m"(X)				\
				 : "r" (&X), "m"(X)			\
				 : "cc"); }
#endif
