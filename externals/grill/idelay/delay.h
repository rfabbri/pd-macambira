/* 

idelay~ - interpolating delay using flext layer

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef _IDELAY_DELAY_H
#define _IDELAY_DELAY_H

#include <math.h>
#include <string.h>

#define F float
#define S t_sample
#define I int
#define V void

template <class T>
class DelayLine
{ 
public:
	DelayLine(): buf(NULL),len(0),wpos(0) {}
	DelayLine(I smps): buf(NULL),len(0),wpos(0) { SetLen(smps); }
	~DelayLine() { if(buf) delete[] buf; }

	V SetLen(I smps); 

	V Put(T s);
	T Get(F delsmps);

	T *buf; I dim;
	I wpos,len;
};

template <class T>
V DelayLine<T>::SetLen(I smps)
{
	if(len != smps) {
		if(buf) delete[] buf;
		len = smps;
		buf = new T[dim = (len+4)]; // some more samples for interpolation
		memset(buf,0,dim*sizeof(T)); 
		wpos = 4; 
	}
}

template <class T>
V DelayLine<T>::Put(T s)
{
	if(!len) return;
	buf[wpos] = s;
	if(wpos >= len) buf[wpos-len] = s; // initialize interpolation security zone
	if(++wpos == dim) wpos -= len;
}

template <class T>
T DelayLine<T>::Get(F delsmps)
{
	if(!len) return 0;
	if(delsmps < 0) 
		return buf[wpos-1];
	else {
    	if(delsmps > len-1) delsmps = (F)len-1;
    	I idelsmps = (I)delsmps;

		if(idelsmps == 0) {
			const T *bp = buf+wpos;
			const F r = 1.f-delsmps;
			return (F)(r*(r-1)*bp[-3]/2.+(1-r*r)*bp[-2]+r*(r+1)*bp[-1]/2.);
		}
		else {
			const T *bp = buf+wpos-idelsmps;
			if(bp < (const T *)buf+4) bp += len;
	    	const F r = 1.f-(delsmps - (F)idelsmps);
			return (F)(
				((2-r)*(r-1)*r*bp[-3])/6. + 
				((r-2)*(r*r-1)*bp[-2])/2. + 
				((2-r)*r*(r+1)*bp[-1])/2. + 
				((r*r-1)*r*bp[0])/6.
			);
		}
	}
}


#endif

