#include "pv.h"

void leanconvert( float *S, float *C, int N2 )
{
	register int i;

	float a = S[0];  // real value at f=0
	float b = S[1];  // real value at f=Nyquist

	C[0] = fabs(a);
	C[1] = 0;
	S += 2,C += 2;

	for ( i = 1; i < N2; i++,S += 2,C += 2 ) {
		C[0] = hypot( S[0], S[1] );
		C[1] = -atan2( S[1], S[0] );
	}

	C[0] = fabs(b);
	C[1] = 0;
}


/* unconvert essentially undoes what convert does, i.e., it
  turns N2+1 PAIRS of amplitude and frequency values in
  C into N2 PAIR of complex spectrum data (in rfft format)
  in output array S; sampling rate R and interpolation factor
  I are used to recompute phase values from frequencies */

void leanunconvert( float *C, float *S, int N2 )
{
	register int i;

	S[0] = fabs(C[0]);
	S[1] = fabs(C[N2*2]);
	S += 2,C += 2;

	for (i = 1; i < N2; i++,S += 2,C += 2 ) {
		S[0] = C[0] * cos( C[1] );
		S[1] = -C[0] * sin( C[1] );
	}
}

