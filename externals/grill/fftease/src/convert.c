#include "pv.h"

/* S is a spectrum in rfft format, i.e., it contains N real values
   arranged as real followed by imaginary values, except for first
   two values, which are real parts of 0 and Nyquist frequencies;
   convert first changes these into N/2+1 PAIRS of magnitude and
   phase values to be stored in output array C; the phases are then
   unwrapped and successive phase differences are used to compute
   estimates of the instantaneous frequencies for each phase vocoder
   analysis channel; decimation rate D and sampling rate R are used
   to render these frequency values directly in Hz. */

void convert(float *S, float *C, int N2, float *lastphase,  float fundamental, float factor )
{
	float phase, phasediff;
	int even,odd;
	float a,b;
	int i;

	for ( i = 0; i <= N2; i++ ) {
		odd = ( even = i<<1 ) + 1;
		a = ( i == N2 ? S[1] : S[even] );
		b = ( i == 0 || i == N2 ? 0. : S[odd] );

		C[even] = hypot( a, b );
		if ( C[even] == 0. )
			phasediff = 0.;
		else {
			phasediff = ( phase = -atan2( b, a ) ) - lastphase[i];
			lastphase[i] = phase;

			while ( phasediff > PV_PI )	phasediff -= PV_2PI;
			while ( phasediff < -PV_PI ) phasediff += PV_2PI;
		}

		C[odd] = phasediff*factor + i*fundamental;
	}
}


void unconvert(float  *C, float *S, int N2, float *lastphase, float fundamental,  float factor )
{
	int i,even,odd;
	float mag,phase;

	for ( i = 0; i <= N2; i++ ) {
		odd = ( even = i<<1 ) + 1;

		mag = C[even];
		lastphase[i] += C[odd] - i*fundamental;
		phase = lastphase[i]*factor;

		if(i != N2) {
			S[even] = mag*cos( phase );
			S[odd] = -mag*sin( phase );
		}
		else
			S[1] = mag*cos( phase );
	}
}
