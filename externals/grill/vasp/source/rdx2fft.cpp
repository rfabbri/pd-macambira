#include <math.h>

#define PI 3.1415926535897932384f

//////////////////////////////////////////////////////////////////////////

/* calculate bidirectional fourier transform of complex data radix 2 */
/* adapted from subroutine FOUREA listed in                          */
/* Programs for Digital Signal Processing                            */
/* edited by Digital Signal Processing Committee                     */
/* IEEE Acoustics Speech and Signal Processing Committee             */
/* Chapter 1 Section 1.1 Page 1.1-4,5                                */
/* direct -1 forward +1 reverse                                      */

bool fft_bidir_complex_radix2(int size,float *real,float *imag,int direct)
{
  int i,j,m,mmax,istep;
  float c,s,treal,timag,theta; 

  /* compute transform */

  j=1;
  for(i=1;i<=size;i++)
  {
    if(i<j)
    {
      treal=real[j-1];
      timag=imag[j-1];
      real[j-1]=real[i-1];
      imag[j-1]=imag[i-1];
      real[i-1]=treal;
      imag[i-1]=timag;
    }
    m=size/2;
    while(j>m)
    {
      j-=m;
      m=(m+1)/2;
    }
    j+=m;
  }
  mmax=1;
  while(size>mmax)
  {
    istep=2*mmax;
    for(m=1;m<=mmax;m++)
    {
      theta=PI*(float)direct*(float)(m-1)/(float)mmax;
      c=(float)cos(theta);
      s=(float)sin(theta);
      for(i=m;i<=size;i+=istep)
      {
	j=i+mmax;
	treal=real[j-1]*c-imag[j-1]*s;
	timag=imag[j-1]*c+real[j-1]*s;
	real[j-1]=real[i-1]-treal;
	imag[j-1]=imag[i-1]-timag;
	real[i-1]+=treal;
	imag[i-1]+=timag;
      }
    }
    mmax=istep;
  }

  return true;
}

#if 0
/* calculate forward fourier transform of complex data radix 2 */

bool fft_fwd_complex_radix2(int size,float *real,float *imag)
{
  return fft_bidir_complex_radix2(size,real,imag,-1);
}

/* calculate inverse fourier transform of complex data radix 2 */

bool fft_inv_complex_radix2(int size,float *real,float *imag)
{
  return fft_bidir_complex_radix2(size,real,imag,1);
}
#endif
