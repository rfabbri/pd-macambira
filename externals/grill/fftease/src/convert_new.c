/* T.Grill */
#include "pv.h" 

/* #include <stdio.h> */
#include <math.h>



/* S is a spectrum in rfft format, i.e., it contains N real values
   arranged as real followed by imaginary values, except for first
   two values, which are real parts of 0 and Nyquist frequencies;
   convert first changes these into N/2+1 PAIRS of magnitude and
   phase values to be stored in output array C; the phases are then
   unwrapped and successive phase differences are used to compute
   estimates of the instantaneous frequencies for each phase vocoder
   analysis channel; decimation rate D and sampling rate R are used
   to render these frequency values directly in Hz. */





void convert_new(float *S, float *C, int N2, float *lastphase,  float fundamental, float factor )
{
  float 	phase,
		phasediff;
  int 		real,
		imag,
		amp,
		freq;
  float 	a,
		b;
  int 		i;

  float myTWOPI, myPI;
  
  myTWOPI = 8.*atan(1.);
  myPI = 4.*atan(1.);


    for ( i = 0; i <= N2; i++ ) {
      imag = freq = ( real = amp = i<<1 ) + 1;
      a = ( i == N2 ? S[1] : S[real] );
      b = ( i == 0 || i == N2 ? 0. : S[imag] );

      C[amp] = hypot( a, b );
      if ( C[amp] == 0. )
	phasediff = 0.;
      else {
	phasediff = ( phase = -atan2( b, a ) ) - lastphase[i];
	lastphase[i] = phase;
	
	while ( phasediff > myPI )
	  phasediff -= myTWOPI;
	while ( phasediff < -myPI )
	  phasediff += myTWOPI;
      }
      C[freq] = phasediff*factor + i*fundamental;
      /*
      if( i > 8 && i < 12 ) {
	fprintf(stderr,"convert freq %d: %f\n",i, C[freq]);
      }
      */
    }
}
