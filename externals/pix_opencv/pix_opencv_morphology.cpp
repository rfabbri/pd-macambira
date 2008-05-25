////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_opencv_morphology.h"

CPPEXTERN_NEW(pix_opencv_morphology)

/////////////////////////////////////////////////////////
//
// pix_opencv_morphology
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_opencv_morphology :: pix_opencv_morphology()
{ 
  int i;

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));

  pos = 0;
  comp_xsize  = 0;
  comp_ysize  = 0;

  rgba = NULL;
  alpha = NULL;
  src = NULL;
  dst = NULL;
  
  element_shape = CV_SHAPE_RECT;

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_opencv_morphology :: ~pix_opencv_morphology()
{
    	//Destroy cv_images to clean memory
    	cvReleaseImage( &src );
    	cvReleaseImage( &dst );
    	cvReleaseImage( &rgba );
    	cvReleaseImage( &alpha );
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_opencv_morphology :: processRGBAImage(imageStruct &image)
{
  unsigned char *pixels = image.data;
  int i;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!rgba)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
    	cvReleaseImage( &src );
    	cvReleaseImage( &dst );
    	cvReleaseImage( &rgba );
    	cvReleaseImage( &alpha );

	//Create cv_images 
    	src = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 3 );
	dst = cvCloneImage(src);
    	rgba = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 4 );
    	alpha = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 1 );
    }
    // FEM UNA COPIA DEL PACKET A image->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( rgba->imageData, image.data, image.xsize*image.ysize*4 );
    
    CvArr* in[] = { rgba };
    CvArr* out[] = { src, alpha };
    int from_to[] = { 0, 0, 1, 1, 2, 2, 3, 3 };
    //cvSet( rgba, cvScalar(1,2,3,4) );
    cvMixChannels( (const CvArr**)in, 1, out, 2, from_to, 4 );

    if (this->mode == 1) { //open/close
    	int n = pos;
    	int an = n > 0 ? n : -n;
    	element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    	if( n < 0 )
    	{
        	cvErode(src,dst,element,1);
        	cvDilate(dst,dst,element,1);
    	}
    	else
    	{
        	cvDilate(src,dst,element,1);
        	cvErode(dst,dst,element,1);
    	}
    	cvReleaseStructuringElement(&element);

    } else {
    	int n = pos;
    	int an = n > 0 ? n : -n;
    	element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    	if( n < 0 )
    	{
        	cvErode(src,dst,element,1);
    	}
    	else
    	{
        	cvDilate(src,dst,element,1);
    	}
    	cvReleaseStructuringElement(&element);

    }

    CvArr* src[] = { dst, alpha };
    CvArr* dst[] = { rgba };
    cvMixChannels( (const CvArr**)src, 2, (CvArr**)dst, 1, from_to, 4 );
    //cvShowImage(wndname, cedge);
    memcpy( image.data, rgba->imageData, image.xsize*image.ysize*4 );
}

void pix_opencv_morphology :: processRGBImage(imageStruct &image)
{
  unsigned char *pixels = image.data;
  int i;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!src)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
    	cvReleaseImage( &src );
    	cvReleaseImage( &dst );

	//Create cv_images 
    	src = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 4 );
	dst = cvCloneImage(src);
    }
    // FEM UNA COPIA DEL PACKET A image->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( src->imageData, image.data, image.xsize*image.ysize*3 );

    if (this->mode == 1) { //open/close
    	int n = pos;
    	int an = n > 0 ? n : -n;
    	element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    	if( n < 0 )
    	{
        	cvErode(src,dst,element,1);
        	cvDilate(dst,dst,element,1);
    	}
    	else
    	{
        	cvDilate(src,dst,element,1);
        	cvErode(dst,dst,element,1);
    	}
    	cvReleaseStructuringElement(&element);

    } else {
    	int n = pos;
    	int an = n > 0 ? n : -n;
    	element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    	if( n < 0 )
    	{
        	cvErode(src,dst,element,1);
    	}
    	else
    	{
        	cvDilate(src,dst,element,1);
    	}
    	cvReleaseStructuringElement(&element);

    }

    //cvShowImage(wndname, cedge);
    memcpy( image.data, dst->imageData, image.xsize*image.ysize*3 );
}

void pix_opencv_morphology :: processYUVImage(imageStruct &image)
{
}
    	
void pix_opencv_morphology :: processGrayImage(imageStruct &image)
{ 
  unsigned char *pixels = image.data;
  int i;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!alpha)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
    	cvReleaseImage( &src );
    	cvReleaseImage( &dst );
    	cvReleaseImage( &alpha );

	//Create cv_images 
    	src = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 3 );
	dst = cvCloneImage(src);
    	alpha = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 1 );
    }
    // FEM UNA COPIA DEL PACKET A image->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( alpha->imageData, image.data, image.xsize*image.ysize );
    
    cvCvtColor(alpha, src, CV_GRAY2RGB);
    
    if (this->mode == 1) { //open/close
    	int n = pos;
    	int an = n > 0 ? n : -n;
    	element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    	if( n < 0 )
    	{
        	cvErode(src,dst,element,1);
        	cvDilate(dst,dst,element,1);
    	}
    	else
    	{
        	cvDilate(src,dst,element,1);
        	cvErode(dst,dst,element,1);
    	}
    	cvReleaseStructuringElement(&element);

    } else {
    	int n = pos;
    	int an = n > 0 ? n : -n;
    	element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    	if( n < 0 )
    	{
        	cvErode(src,dst,element,1);
    	}
    	else
    	{
        	cvDilate(src,dst,element,1);
    	}
    	cvReleaseStructuringElement(&element);

    }

    cvCvtColor(dst, alpha, CV_RGB2GRAY);
    //cvShowImage(wndname, cedge);
    memcpy( image.data, alpha->imageData, image.xsize*image.ysize );
}

/////////////////////////////////////////////////////////
// floatPosMess
//
/////////////////////////////////////////////////////////
void pix_opencv_morphology :: floatPosMess (float pos)
{
  this->pos = (int)pos;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_opencv_morphology :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, (t_method)&pix_opencv_morphology::floatPosMessCallback,
  		  gensym("ft1"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_morphology::modeMessCallback,
		  gensym("mode"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_morphology::shapeMessCallback,
		  gensym("shape"), A_DEFFLOAT, A_NULL);
}
void pix_opencv_morphology :: floatPosMessCallback(void *data, t_floatarg pos)
{
  GetMyClass(data)->floatPosMess((float)pos);
}
void pix_opencv_morphology :: modeMessCallback(void *data, t_floatarg mode)
{
  GetMyClass(data)->mode=!(!(int)mode);
}
void pix_opencv_morphology :: shapeMessCallback(void *data, t_floatarg f)
{
        if( (int)f == 1 )
            GetMyClass(data)->element_shape = CV_SHAPE_RECT;
        else if( (int)f == 2 )
            GetMyClass(data)->element_shape = CV_SHAPE_ELLIPSE;
        else if( (int)f == 3 )
            GetMyClass(data)->element_shape = CV_SHAPE_CROSS;
}
