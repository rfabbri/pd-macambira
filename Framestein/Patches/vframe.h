#ifndef __VFRAME_H
#define __VFRAME_H

#include "plugin.h"

struct vframeimage
{
	char bitsname[40];	// name of shared memory to image data
				// use this, not f.bits!
	_frame f;		// image header, see Plugins\plugin.h
};

LPVOID openframedatabyid(	int id,
				HANDLE *hvf, HANDLE *hbits,
				LPVOID *memvf, LPVOID *membits,
				struct vframeimage **vfp )
{
	char s[80];

	if(*membits!=NULL) smfree(hbits, *membits);
	if(*memvf!=NULL) smfree(hvf, *memvf);

	*membits=NULL;

	itoa(id, s, 10);

	*memvf = smopen(hvf, s);
	if(*memvf!=NULL)
	{
		*vfp = *memvf;

		*membits = smopen(hbits, ((struct vframeimage *)*vfp)->bitsname);
		if(*membits==NULL)
		{
			printf("membits open error.\n");
			smfree(hvf, *memvf);
		} else {
		}
	}
	return(*membits);
}

float max16 = 2<<14;
float max32 = 2<<23;

__inline float colortosample16(short c)
{
	return(c / max16);
}

__inline float colortosample32(long c)
{
	return(c / max32);
}

__inline short sampletocolor16(float s)
{
	return(s * max16);
}

__inline long sampletocolor32(float s)
{
	return(s * max32);
}

#endif
