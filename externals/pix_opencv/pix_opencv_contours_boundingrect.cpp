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

#include "pix_opencv_contours_boundingrect.h"

CPPEXTERN_NEW(pix_opencv_contours_boundingrect)

/////////////////////////////////////////////////////////
//
// pix_opencv_contours_boundingrect
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_opencv_contours_boundingrect :: pix_opencv_contours_boundingrect()
{ 
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("minarea"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("maxarea"));
  m_dataout = outlet_new(this->x_obj, 0);
  minarea = 1;
  maxarea = 320*240;
  comp_xsize  = 0;
  comp_ysize  = 0;
  orig = NULL;
  gray = NULL;
  cnt_img = NULL;
  rgb = NULL;

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_opencv_contours_boundingrect :: ~pix_opencv_contours_boundingrect()
{ 
    	//Destroy cv_images to clean memory
	cvReleaseImage(&orig);
    	cvReleaseImage(&gray);
    	cvReleaseImage(&cnt_img);
    	cvReleaseImage(&rgb);
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_opencv_contours_boundingrect :: processRGBAImage(imageStruct &image)
{
  unsigned char *pixels = image.data;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!orig)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
	cvReleaseImage(&orig);
    	cvReleaseImage(&gray);
    	cvReleaseImage(&cnt_img);
    	cvReleaseImage(&rgb);

	//create the orig image with new size
        orig = cvCreateImage(cvSize(image.xsize,image.ysize), IPL_DEPTH_8U, 4);

    	// Create the output images with new sizes
    	rgb = cvCreateImage(cvSize(orig->width,orig->height), IPL_DEPTH_8U, 3);
    	cnt_img = cvCreateImage(cvSize(orig->width,orig->height), IPL_DEPTH_8U, 3);

    	gray = cvCreateImage(cvSize(orig->width,orig->height), IPL_DEPTH_8U, 1);
    
    }
    // Here we make a copy of the pixel data from image to orig->imageData
    // orig is a IplImage struct, the default image type in openCV, take a look on the IplImage data structure here
    // http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html 
    memcpy( orig->imageData, image.data, image.xsize*image.ysize*4 );
    
    // Convert to grayscale
    cvCvtColor(orig, gray, CV_RGBA2GRAY);

    CvSeq* contours;
    CvMemStorage* stor02;
    stor02 = cvCreateMemStorage(0);

    cvFindContours( gray, stor02, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
    if (contours) contours = cvApproxPoly( contours, sizeof(CvContour), stor02, CV_POLY_APPROX_DP, 3, 1 );
  

    int i = 0;                   // Indicator of cycles.
    for( ; contours != 0; contours = contours->h_next )
        {
        int count = contours->total; // This is number point in contour
        CvRect rect;

	rect = cvContourBoundingRect( contours, 1);
	if ( ( (rect.width*rect.height) > minarea ) && ( (rect.width*rect.height) < maxarea ) ) {
	    cvRectangle( orig, cvPoint(rect.x,rect.y), cvPoint(rect.x+rect.width,rect.y+rect.height), CV_RGB(255,0,0), 2, 8 , 0 );

    	    t_atom rlist[4];
            SETFLOAT(&rlist[0], i);
            SETFLOAT(&rlist[1], rect.x);
            SETFLOAT(&rlist[2], rect.y);
            SETFLOAT(&rlist[3], rect.width);
            SETFLOAT(&rlist[4], rect.height);

    	    outlet_list( m_dataout, 0, 5, rlist );
	    i++;
	}
	
        }

    cvReleaseMemStorage( &stor02 );


    //copy back the processed frame to image
    memcpy( image.data, orig->imageData, image.xsize*image.ysize*4 );
}

void pix_opencv_contours_boundingrect :: processRGBImage(imageStruct &image)
{
  unsigned char *pixels = image.data;

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!rgb)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
	cvReleaseImage(&orig);
    	cvReleaseImage(&gray);
    	cvReleaseImage(&cnt_img);
    	cvReleaseImage(&rgb);

	//create the orig image with new size
        rgb = cvCreateImage(cvSize(image.xsize,image.ysize), IPL_DEPTH_8U, 3);

    	// Create the output images with new sizes
    	cnt_img = cvCreateImage(cvSize(rgb->width,rgb->height), IPL_DEPTH_8U, 3);

    	gray = cvCreateImage(cvSize(rgb->width,rgb->height), IPL_DEPTH_8U, 1);
    
    }
    // FEM UNA COPIA DEL PACKET A image->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( rgb->imageData, image.data, image.xsize*image.ysize*3 );
    
    // Convert to grayscale
    cvCvtColor(rgb, gray, CV_RGB2GRAY);
  
    CvSeq* contours;
    CvMemStorage* stor02;
    stor02 = cvCreateMemStorage(0);

    cvFindContours( gray, stor02, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
    if (contours) contours = cvApproxPoly( contours, sizeof(CvContour), stor02, CV_POLY_APPROX_DP, 3, 1 );
  

    int i = 0;                   // Indicator of cycles.
    for( ; contours != 0; contours = contours->h_next )
        {
        int count = contours->total; // This is number point in contour
        CvRect rect;

	rect = cvContourBoundingRect( contours, 1);
	if ( ( (rect.width*rect.height) > minarea ) && ( (rect.width*rect.height) < maxarea ) ) {
	    cvRectangle( rgb, cvPoint(rect.x,rect.y), cvPoint(rect.x+rect.width,rect.y+rect.height), CV_RGB(255,0,0), 2, 8 , 0 );

    	    t_atom rlist[4];
            SETFLOAT(&rlist[0], i);
            SETFLOAT(&rlist[1], rect.x);
            SETFLOAT(&rlist[2], rect.y);
            SETFLOAT(&rlist[3], rect.width);
            SETFLOAT(&rlist[4], rect.height);

    	    outlet_list( m_dataout, 0, 5, rlist );
	    i++;
	}
	
        }

    cvReleaseMemStorage( &stor02 );


    //cvShowImage(wndname, cedge);
    memcpy( image.data, rgb->imageData, image.xsize*image.ysize*3 );
}

void pix_opencv_contours_boundingrect :: processYUVImage(imageStruct &image)
{
}
    	
void pix_opencv_contours_boundingrect :: processGrayImage(imageStruct &image)
{ 

  if ((this->comp_xsize!=image.xsize)||(this->comp_ysize!=image.ysize)||(!orig)) {

	this->comp_xsize = image.xsize;
	this->comp_ysize = image.ysize;

    	//Destroy cv_images to clean memory
	cvReleaseImage(&orig);
    	cvReleaseImage(&gray);
    	cvReleaseImage(&cnt_img);
    	cvReleaseImage(&rgb);

	//create the orig image with new size
        orig = cvCreateImage(cvSize(image.xsize,image.ysize), IPL_DEPTH_8U, 4);

    	// Create the output images with new sizes
    	rgb = cvCreateImage(cvSize(orig->width,orig->height), IPL_DEPTH_8U, 3);
    	cnt_img = cvCreateImage(cvSize(orig->width,orig->height), IPL_DEPTH_8U, 3);

    	gray = cvCreateImage(cvSize(orig->width,orig->height), IPL_DEPTH_8U, 1);
    
    }
    // Here we make a copy of the pixel data from image to orig->imageData
    // orig is a IplImage struct, the default image type in openCV, take a look on the IplImage data structure here
    // http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html 
    memcpy( gray->imageData, image.data, image.xsize*image.ysize );
    

    CvSeq* contours;
    CvMemStorage* stor02;
    stor02 = cvCreateMemStorage(0);

    cvFindContours( gray, stor02, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
    if (contours) contours = cvApproxPoly( contours, sizeof(CvContour), stor02, CV_POLY_APPROX_DP, 3, 1 );
  

    int i = 0;                   // Indicator of cycles.
    for( ; contours != 0; contours = contours->h_next )
        {
        int count = contours->total; // This is number point in contour
        CvRect rect;

	rect = cvContourBoundingRect( contours, 1);
	if ( ( (rect.width*rect.height) > minarea ) && ( (rect.width*rect.height) < maxarea ) ) {
	    cvRectangle( gray, cvPoint(rect.x,rect.y), cvPoint(rect.x+rect.width,rect.y+rect.height), CV_RGB(255,0,0), 2, 8 , 0 );

    	    t_atom rlist[4];
            SETFLOAT(&rlist[0], i);
            SETFLOAT(&rlist[1], rect.x);
            SETFLOAT(&rlist[2], rect.y);
            SETFLOAT(&rlist[3], rect.width);
            SETFLOAT(&rlist[4], rect.height);

    	    outlet_list( m_dataout, 0, 5, rlist );
	    i++;
	}
	
        }

    cvReleaseMemStorage( &stor02 );


    //copy back the processed frame to image
    memcpy( image.data, gray->imageData, image.xsize*image.ysize );
}

/////////////////////////////////////////////////////////
// floatThreshMess
//
/////////////////////////////////////////////////////////
void pix_opencv_contours_boundingrect :: floatMinAreaMess (float minarea)
{
  if (minarea>0) this->minarea = (int)minarea;
}
void pix_opencv_contours_boundingrect :: floatMaxAreaMess (float maxarea)
{
  if (maxarea>0) this->maxarea = (int)maxarea;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_opencv_contours_boundingrect :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, (t_method)&pix_opencv_contours_boundingrect::floatMinAreaMessCallback,
  		  gensym("minarea"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_contours_boundingrect::floatMaxAreaMessCallback,
  		  gensym("maxarea"), A_FLOAT, A_NULL);
}
void pix_opencv_contours_boundingrect :: floatMaxAreaMessCallback(void *data, t_floatarg maxarea)
{
  GetMyClass(data)->floatMaxAreaMess((float)maxarea);
}
void pix_opencv_contours_boundingrect :: floatMinAreaMessCallback(void *data, t_floatarg minarea)
{
  GetMyClass(data)->floatMinAreaMess((float)minarea);
}
