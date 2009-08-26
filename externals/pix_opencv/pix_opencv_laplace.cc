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

#include "pix_opencv_laplace.h"

CPPEXTERN_NEW(pix_opencv_laplace)

/////////////////////////////////////////////////////////
//
// pix_opencv_laplace
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_opencv_laplace :: pix_opencv_laplace()
{ 
  int i;

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));

  aperture_size = 3;
  comp_xsize  = 0;
  comp_ysize  = 0;

  frame = NULL;
  rgba = NULL;
  alpha = NULL;
  laplace = NULL;
  colorlaplace = NULL;
  for (i=0; i<3; i++) planes[i] = NULL;

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_opencv_laplace :: ~pix_opencv_laplace()
{
  int i; 
    	//Destroy cv_images to clean memory
    	for( i = 0; i < 3; i++ )
    		cvReleaseImage( &planes[i] );
    	cvReleaseImage( &frame );
    	cvReleaseImage( &rgba );
    	cvReleaseImage( &alpha );
    	cvReleaseImage( &laplace );
    	cvReleaseImage( &colorlaplace );
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_opencv_laplace :: processRGBAImage(imageStruct &image)
{
  unsigned char *pixels = image.data;
  int i;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!rgba)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
    	for( i = 0; i < 3; i++ )
    		cvReleaseImage( &planes[i] );
    	cvReleaseImage( &frame );
    	cvReleaseImage( &rgba );
    	cvReleaseImage( &alpha );
    	cvReleaseImage( &laplace );
    	cvReleaseImage( &colorlaplace );

	//Create cv_images 
    	for( i = 0; i < 3; i++ )
     	 	planes[i] = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 1 );
    	laplace = cvCreateImage( cvSize(image.xsize, image.ysize), IPL_DEPTH_16S, 1 );
    	colorlaplace = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 3 );
    	frame = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 3 );
    	rgba = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 4 );
    	alpha = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 1 );
    }
    // FEM UNA COPIA DEL PACKET A image->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( rgba->imageData, image.data, image.xsize*image.ysize*4 );
    
    CvArr* in[] = { rgba };
    CvArr* out[] = { frame, alpha };
    int from_to[] = { 0, 0, 1, 1, 2, 2, 3, 3 };
    //cvSet( rgba, cvScalar(1,2,3,4) );
    cvMixChannels( (const CvArr**)in, 1, out, 2, from_to, 4 );

	cvCvtPixToPlane( frame, planes[0], planes[1], planes[2], 0 );
        for( i = 0; i < 3; i++ )
        {
            cvLaplace( planes[i], laplace, aperture_size );
            cvConvertScaleAbs( laplace, planes[i], 1, 0 );
        }
        cvCvtPlaneToPix( planes[0], planes[1], planes[2], 0, colorlaplace );
        colorlaplace->origin = frame->origin;


    CvArr* src[] = { colorlaplace, alpha };
    CvArr* dst[] = { rgba };
    cvMixChannels( (const CvArr**)src, 2, (CvArr**)dst, 1, from_to, 4 );
    //cvShowImage(wndname, cedge);
    memcpy( image.data, rgba->imageData, image.xsize*image.ysize*4 );
}

void pix_opencv_laplace :: processRGBImage(imageStruct &image)
{
  unsigned char *pixels = image.data;
  int i;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!frame)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
    	for( i = 0; i < 3; i++ )
    		cvReleaseImage( &planes[i] );
    	cvReleaseImage( &frame );
    	cvReleaseImage( &laplace );
    	cvReleaseImage( &colorlaplace );

	//Create cv_images 
    	for( i = 0; i < 3; i++ )
     	 	planes[i] = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 1 );
    	laplace = cvCreateImage( cvSize(image.xsize, image.ysize), IPL_DEPTH_16S, 1 );
    	colorlaplace = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 3 );
    	frame = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 3 );
    }
    // FEM UNA COPIA DEL PACKET A image->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( frame->imageData, image.data, image.xsize*image.ysize*3 );

	cvCvtPixToPlane( frame, planes[0], planes[1], planes[2], 0 );
        for( i = 0; i < 3; i++ )
        {
            cvLaplace( planes[i], laplace, 3 );
            cvConvertScaleAbs( laplace, planes[i], 1, 0 );
        }
        cvCvtPlaneToPix( planes[0], planes[1], planes[2], 0, colorlaplace );
        colorlaplace->origin = frame->origin;


    //cvShowImage(wndname, cedge);
    memcpy( image.data, colorlaplace->imageData, image.xsize*image.ysize*3 );
}

void pix_opencv_laplace :: processYUVImage(imageStruct &image)
{
  post( "pix_opencv_laplace : yuv format not supported" );
}
    	
void pix_opencv_laplace :: processGrayImage(imageStruct &image)
{ 
  unsigned char *pixels = image.data;
  int i;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!alpha)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
    	for( i = 0; i < 3; i++ )
    		cvReleaseImage( &planes[i] );
    	cvReleaseImage( &frame );
    	cvReleaseImage( &laplace );
    	cvReleaseImage( &colorlaplace );
    	cvReleaseImage( &alpha );

	//Create cv_images 
    	for( i = 0; i < 3; i++ )
     	 	planes[i] = cvCreateImage( cvSize(image.xsize, image.ysize), 8, 1 );
    	laplace = cvCreateImage( cvSize(image.xsize, image.ysize), IPL_DEPTH_16S, 1 );
    	colorlaplace = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 3 );
    	frame = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 3 );
    	alpha = cvCreateImage( cvSize(image.xsize,image.ysize), 8, 1 );
    }
    // FEM UNA COPIA DEL PACKET A image->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( alpha->imageData, image.data, image.xsize*image.ysize );

    cvCvtColor( alpha, frame, CV_GRAY2RGB);

	cvCvtPixToPlane( frame, planes[0], planes[1], planes[2], 0 );
        for( i = 0; i < 3; i++ )
        {
            cvLaplace( planes[i], laplace, 3 );
            cvConvertScaleAbs( laplace, planes[i], 1, 0 );
        }
        cvCvtPlaneToPix( planes[0], planes[1], planes[2], 0, colorlaplace );
        colorlaplace->origin = frame->origin;


    cvCvtColor( colorlaplace, alpha, CV_RGB2GRAY);
    //cvShowImage(wndname, cedge);
    memcpy( image.data, alpha->imageData, image.xsize*image.ysize );
}

/////////////////////////////////////////////////////////
// floatApertureMess
//
/////////////////////////////////////////////////////////
void pix_opencv_laplace :: floatApertureMess (float aperture_size)
{
  if ((aperture_size==1)||(aperture_size==3)||(aperture_size==5)||(aperture_size==7)) this->aperture_size = (int)aperture_size;
  else post("aperture size out of range ...  must be 1,3,5 or 7");
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_opencv_laplace :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, (t_method)&pix_opencv_laplace::floatApertureMessCallback,
  		  gensym("ft1"), A_FLOAT, A_NULL);
}
void pix_opencv_laplace :: floatApertureMessCallback(void *data, t_floatarg aperture_size)
{
  GetMyClass(data)->floatApertureMess((float)aperture_size);
}
