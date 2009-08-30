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

#ifndef INCLUDE_PIX_OPENCV_BGSUBSTRACT_H_
#define INCLUDE_PIX_OPENCV_BGSUBSTRACT_H_

#include "Base/GemPixObj.h"

#ifndef _EiC
#include "cv.h"
#include "cvaux.h"
#endif

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_opencv_bgstats
    
    Statistical background substraction

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_opencv_bgstats : public GemPixObj
{
    CPPEXTERN_HEADER(pix_opencv_bgstats, GemPixObj)

    public:

	    //////////
	    // Constructor
    	pix_opencv_bgstats();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_opencv_bgstats();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	virtual void 	processRGBImage(imageStruct &image);
	virtual void 	processYUVImage(imageStruct &image);
    	virtual void 	processGrayImage(imageStruct &image); 
    	
	//////////
    	// Set the new edge threshold
    	void	    	floatMinAreaMess(float minarea);
    	void	    	floatErodeMess(float erode);
    	void	    	floatDeltaMess(float delta);
    	void	    	resetMess(void);

    	// The new threshold
        int             x_erode;
        float           x_minarea;
        float           x_delta;
        int             x_frames;

	// to detect changes in the image size
	int 		comp_xsize;
	int		comp_ysize;

    private:
    
    	//////////
    	// Static member functions
    	static void    	floatMinAreaMessCallback(void *data, float minarea);
    	static void    	floatErodeMessCallback(void *data, float erode);
    	static void    	floatDeltaMessCallback(void *data, float delta);
    	static void    	resetMessCallback(void *data);

	/////////
	// IplImage needed
    	IplImage 	*rgba, *rgb, *gray;
        IplImage *foreground, *incoming;

        // Stat background model data
        CvBGStatModel *x_model;
        CvFGDStatModelParams x_modelparams;

};

#endif	// for header file
