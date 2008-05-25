/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Change pix to greyscale

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM. zmoelnig@iem.kug.ac.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_OPENCV_FACEDETECT_H_
#define INCLUDE_PIX_OPENCV_FACEDETECT_H_

#include "Base/GemPixObj.h"

#ifndef _EiC
#include "cv.h"
#endif


const char* cascade_name ="./haarcascade_frontalface_alt.xml";

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_opencv_haarcascade
    
    Change pix to greyscale

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_opencv_haarcascade : public GemPixObj
{
    CPPEXTERN_HEADER(pix_opencv_haarcascade, GemPixObj)

    public:

	    //////////
	    // Constructor
    	pix_opencv_haarcascade();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_opencv_haarcascade();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	virtual void 	processRGBImage(imageStruct &image);
	virtual void 	processYUVImage(imageStruct &image);
    	virtual void 	processGrayImage(imageStruct &image); 
    	
	//////////
    	// Set the new edge threshold
    	void	    	scaleFactorMess(float scale_factor);
    	void	    	minNeighborsMess(float min_neighbors);
    	void	    	modeMess(float mode);
    	void	    	minSizeMess(float min_size);
	void 		loadCascadeMess(t_symbol *filename);
    	// The parameters for cvHaarDetectObjects function
	float 		scale_factor;
	int 		min_neighbors;
	int		mode;
	int		min_size;
	// to detect changes in the image size
	int 		comp_xsize;
	int		comp_ysize;

    private:
    
    	//////////
    	// Static member functions
    	static void 	scaleFactorMessCallback(void *data, t_floatarg scale_factor);
    	static void	minNeighborsMessCallback(void *data, float min_neighbors);
    	static void	modeMessCallback(void *data, float mode);
    	static void	minSizeMessCallback(void *data, float min_size);
    	static void 	loadCascadeMessCallback(void *data, t_symbol* filename);

	CvHaarClassifierCascade* cascade;
	/////////
	// IplImage needed
    	IplImage 	*rgba, *frame, *grey;
	
	t_outlet 	*m_dataout;
};

#endif	// for header file
