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

#include "pix_opencv_lk.h"
#include <stdio.h>

CPPEXTERN_NEW(pix_opencv_lk)

/////////////////////////////////////////////////////////
//
// pix_opencv_lk
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

pix_opencv_lk :: pix_opencv_lk()
{ 
  int i;

  comp_xsize=320;
  comp_ysize=240;

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("winsize"));

  m_dataout = outlet_new(this->x_obj, &s_anything);
  win_size = 10;

  points[0] = 0;
  points[1] = 0;
  status = 0;
  count = 0;
  need_to_init = 1;
  night_mode = 0;
  flags = 0;
  add_remove_pt = 0;
  quality = 0.1;
  min_distance = 10;
  maxmove = 8;

  for ( i=0; i<MAX_MARKERS; i++ )
  {
     x_xmark[i] = -1;
     x_ymark[i] = -1;
  }

  // initialize font
  cvInitFont( &font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0, 1, 8 );

  rgba = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 4 );
  rgb = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 3 );
  grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
  prev_grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
  pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
  prev_pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
  points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
  points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
  status = (char*)cvAlloc(MAX_COUNT);

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_opencv_lk :: ~pix_opencv_lk()
{
  // Destroy cv_images
  cvReleaseImage( &rgba );
  cvReleaseImage( &rgb );
  cvReleaseImage( &grey );
  cvReleaseImage( &prev_grey );
  cvReleaseImage( &pyramid );
  cvReleaseImage( &prev_pyramid );
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_opencv_lk :: processRGBAImage(imageStruct &image)
{
  int i, k;
  int im;

  if ((this->comp_xsize!=image.xsize)&&(this->comp_ysize!=image.ysize)) 
  {

    this->comp_xsize=image.xsize;
    this->comp_ysize=image.ysize;

    rgba = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 4 );
    rgb = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 3 );
    grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    prev_grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    prev_pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    status = (char*)cvAlloc(MAX_COUNT);

  }

  memcpy( rgba->imageData, image.data, image.xsize*image.ysize*4 );
  cvCvtColor(rgba, grey, CV_BGRA2GRAY);

  if( night_mode )
      cvZero( rgba );

  for ( im=0; im<MAX_MARKERS; im++ )
  {
       x_found[im] = 0;
  }

  if( need_to_init )
  {
     /* automatic initialization */
     IplImage* eig = cvCreateImage( cvSize(grey->width,grey->height), 32, 1 );
     IplImage* temp = cvCreateImage( cvSize(grey->width,grey->height), 32, 1 );

     count = MAX_COUNT;
     cvGoodFeaturesToTrack( grey, eig, temp, points[1], &count,
                               quality, min_distance, 0, 3, 0, 0.04 );
     cvFindCornerSubPix( grey, points[1], count,
          cvSize(win_size,win_size), cvSize(-1,-1),
          cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
     cvReleaseImage( &eig );
     cvReleaseImage( &temp );

     add_remove_pt = 0;
  }
  else if( count > 0 )
  {
     cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
              points[0], points[1], count, cvSize(win_size,win_size), 3, status, 0,
              cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
     flags |= CV_LKFLOW_PYR_A_READY;
     for( i = k = 0; i < count; i++ )
     {
        if( add_remove_pt )
        {
           double dx = pt.x - points[1][i].x;
           double dy = pt.y - points[1][i].y;

           if( dx*dx + dy*dy <= 25 )
           {
              add_remove_pt = 0;
              continue;
           }
         }

         if( !status[i] )
          continue;

         points[1][k++] = points[1][i];
         cvCircle( rgba, cvPointFrom32f(points[1][i]), 3, CV_RGB(0,255,0), -1, 8,0);

         for ( im=0; im<MAX_MARKERS; im++ )
         {
           // first marking
           if ( x_xmark[im] != -1.0 )
           {
             if ( ( abs( points[1][i].x - x_xmark[im] ) <= maxmove ) && ( abs( points[1][i].y - x_ymark[im] ) <= maxmove ) )
             {
               char tindex[4];
               sprintf( tindex, "%d", im+1 );
               cvPutText( rgba, tindex, cvPointFrom32f(points[1][i]), &font, CV_RGB(255,255,255));
               x_xmark[im]=points[1][i].x;
               x_ymark[im]=points[1][i].y;
               x_found[im]=1;
               SETFLOAT(&x_list[0], im+1);
               SETFLOAT(&x_list[1], x_xmark[im]);
               SETFLOAT(&x_list[2], x_ymark[im]);
               outlet_list( m_dataout, 0, 3, x_list );
             }
           }
         }
      }
      count = k;
  }

  for ( im=0; im<MAX_MARKERS; im++ )
  {
        if ( (x_xmark[im] != -1.0 ) && !x_found[im] )
        {
           x_xmark[im]=-1.0;
           x_ymark[im]=-1.0;
           SETFLOAT(&x_list[0], im+1);
           SETFLOAT(&x_list[1], x_xmark[im]);
           SETFLOAT(&x_list[2], x_ymark[im]);
           // send a lost point message to the patch
           outlet_list( m_dataout, 0, 3, x_list );
           post( "pix_opencv_lk : lost point %d", im+1 );
        }
  }

  if( add_remove_pt && count < MAX_COUNT )
  {
        points[1][count++] = cvPointTo32f(pt);
        cvFindCornerSubPix( grey, points[1] + count - 1, 1,
           cvSize(win_size,win_size), cvSize(-1,-1),
           cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
        add_remove_pt = 0;
  }

  CV_SWAP( prev_grey, grey, swap_temp );
  CV_SWAP( prev_pyramid, pyramid, swap_temp );
  CV_SWAP( points[0], points[1], swap_points );
  need_to_init = 0;

  memcpy( image.data, rgba->imageData, image.xsize*image.ysize*4 );
}

void pix_opencv_lk :: processRGBImage(imageStruct &image)
{ 
  int i, k;
  int im;

  if ((this->comp_xsize!=image.xsize)&&(this->comp_ysize!=image.ysize)) 
  {

    this->comp_xsize=image.xsize;
    this->comp_ysize=image.ysize;

    rgba = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 4 );
    rgb = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 3 );
    grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    prev_grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    prev_pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    status = (char*)cvAlloc(MAX_COUNT);

  }

  memcpy( rgb->imageData, image.data, image.xsize*image.ysize*3 );
  cvCvtColor(rgb, grey, CV_BGRA2GRAY);

  if( night_mode )
      cvZero( rgb );

  for ( im=0; im<MAX_MARKERS; im++ )
  {
       x_found[im] = 0;
  }

  if( need_to_init )
  {
     /* automatic initialization */
     IplImage* eig = cvCreateImage( cvSize(grey->width,grey->height), 32, 1 );
     IplImage* temp = cvCreateImage( cvSize(grey->width,grey->height), 32, 1 );

     count = MAX_COUNT;
     cvGoodFeaturesToTrack( grey, eig, temp, points[1], &count,
                               quality, min_distance, 0, 3, 0, 0.04 );
     cvFindCornerSubPix( grey, points[1], count,
          cvSize(win_size,win_size), cvSize(-1,-1),
          cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
     cvReleaseImage( &eig );
     cvReleaseImage( &temp );

     add_remove_pt = 0;
  }
  else if( count > 0 )
  {
     cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
              points[0], points[1], count, cvSize(win_size,win_size), 3, status, 0,
              cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
     flags |= CV_LKFLOW_PYR_A_READY;
     for( i = k = 0; i < count; i++ )
     {
        if( add_remove_pt )
        {
           double dx = pt.x - points[1][i].x;
           double dy = pt.y - points[1][i].y;

           if( dx*dx + dy*dy <= 25 )
           {
              add_remove_pt = 0;
              continue;
           }
         }

         if( !status[i] )
          continue;

         points[1][k++] = points[1][i];
         cvCircle( rgb, cvPointFrom32f(points[1][i]), 3, CV_RGB(0,255,0), -1, 8,0);

         for ( im=0; im<MAX_MARKERS; im++ )
         {
           // first marking
           if ( x_xmark[im] != -1.0 )
           {
             if ( ( abs( points[1][i].x - x_xmark[im] ) <= maxmove ) && ( abs( points[1][i].y - x_ymark[im] ) <= maxmove ) )
             {
               char tindex[4];
               sprintf( tindex, "%d", im+1 );
               cvPutText( rgb, tindex, cvPointFrom32f(points[1][i]), &font, CV_RGB(255,255,255));
               x_xmark[im]=points[1][i].x;
               x_ymark[im]=points[1][i].y;
               x_found[im]=1;
               SETFLOAT(&x_list[0], im+1);
               SETFLOAT(&x_list[1], x_xmark[im]);
               SETFLOAT(&x_list[2], x_ymark[im]);
               outlet_list( m_dataout, 0, 3, x_list );
             }
           }
         }
      }
      count = k;
  }

  for ( im=0; im<MAX_MARKERS; im++ )
  {
        if ( (x_xmark[im] != -1.0 ) && !x_found[im] )
        {
           x_xmark[im]=-1.0;
           x_ymark[im]=-1.0;
           SETFLOAT(&x_list[0], im+1);
           SETFLOAT(&x_list[1], x_xmark[im]);
           SETFLOAT(&x_list[2], x_ymark[im]);
           // send a lost point message to the patch
           outlet_list( m_dataout, 0, 3, x_list );
           post( "pix_opencv_lk : lost point %d", im+1 );
        }
  }

  if( add_remove_pt && count < MAX_COUNT )
  {
        points[1][count++] = cvPointTo32f(pt);
        cvFindCornerSubPix( grey, points[1] + count - 1, 1,
           cvSize(win_size,win_size), cvSize(-1,-1),
           cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
        add_remove_pt = 0;
  }

  CV_SWAP( prev_grey, grey, swap_temp );
  CV_SWAP( prev_pyramid, pyramid, swap_temp );
  CV_SWAP( points[0], points[1], swap_points );
  need_to_init = 0;

  memcpy( image.data, rgb->imageData, image.xsize*image.ysize*3 );
}

void pix_opencv_lk :: processYUVImage(imageStruct &image)
{
}
    	
void pix_opencv_lk :: processGrayImage(imageStruct &image)
{ 
  int i, k;
  int im;

  if ((this->comp_xsize!=image.xsize)&&(this->comp_ysize!=image.ysize)) 
  {

    this->comp_xsize=image.xsize;
    this->comp_ysize=image.ysize;

    rgba = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 4 );
    rgb = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 3 );
    grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    prev_grey = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    prev_pyramid = cvCreateImage( cvSize(comp_xsize, comp_ysize), 8, 1 );
    points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    status = (char*)cvAlloc(MAX_COUNT);

  }

  memcpy( grey->imageData, image.data, image.xsize*image.ysize );

  if( night_mode )
      cvZero( grey );

  for ( im=0; im<MAX_MARKERS; im++ )
  {
       x_found[im] = 0;
  }

  if( need_to_init )
  {
     /* automatic initialization */
     IplImage* eig = cvCreateImage( cvSize(grey->width,grey->height), 32, 1 );
     IplImage* temp = cvCreateImage( cvSize(grey->width,grey->height), 32, 1 );

     count = MAX_COUNT;
     cvGoodFeaturesToTrack( grey, eig, temp, points[1], &count,
                               quality, min_distance, 0, 3, 0, 0.04 );
     cvFindCornerSubPix( grey, points[1], count,
          cvSize(win_size,win_size), cvSize(-1,-1),
          cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
     cvReleaseImage( &eig );
     cvReleaseImage( &temp );

     add_remove_pt = 0;
  }
  else if( count > 0 )
  {
     cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
              points[0], points[1], count, cvSize(win_size,win_size), 3, status, 0,
              cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
     flags |= CV_LKFLOW_PYR_A_READY;
     for( i = k = 0; i < count; i++ )
     {
        if( add_remove_pt )
        {
           double dx = pt.x - points[1][i].x;
           double dy = pt.y - points[1][i].y;

           if( dx*dx + dy*dy <= 25 )
           {
              add_remove_pt = 0;
              continue;
           }
         }

         if( !status[i] )
          continue;

         points[1][k++] = points[1][i];
         cvCircle( grey, cvPointFrom32f(points[1][i]), 3, CV_RGB(0,255,0), -1, 8,0);

         for ( im=0; im<MAX_MARKERS; im++ )
         {
           // first marking
           if ( x_xmark[im] != -1.0 )
           {
             if ( ( abs( points[1][i].x - x_xmark[im] ) <= maxmove ) && ( abs( points[1][i].y - x_ymark[im] ) <= maxmove ) )
             {
               char tindex[4];
               sprintf( tindex, "%d", im+1 );
               cvPutText( grey, tindex, cvPointFrom32f(points[1][i]), &font, CV_RGB(255,255,255));
               x_xmark[im]=points[1][i].x;
               x_ymark[im]=points[1][i].y;
               x_found[im]=1;
               SETFLOAT(&x_list[0], im+1);
               SETFLOAT(&x_list[1], x_xmark[im]);
               SETFLOAT(&x_list[2], x_ymark[im]);
               outlet_list( m_dataout, 0, 3, x_list );
             }
           }
         }
      }
      count = k;
  }

  for ( im=0; im<MAX_MARKERS; im++ )
  {
        if ( (x_xmark[im] != -1.0 ) && !x_found[im] )
        {
           x_xmark[im]=-1.0;
           x_ymark[im]=-1.0;
           SETFLOAT(&x_list[0], im+1);
           SETFLOAT(&x_list[1], x_xmark[im]);
           SETFLOAT(&x_list[2], x_ymark[im]);
           // send a lost point message to the patch
           outlet_list( m_dataout, 0, 3, x_list );
           post( "pix_opencv_lk : lost point %d", im+1 );
        }
  }

  if( add_remove_pt && count < MAX_COUNT )
  {
        points[1][count++] = cvPointTo32f(pt);
        cvFindCornerSubPix( grey, points[1] + count - 1, 1,
           cvSize(win_size,win_size), cvSize(-1,-1),
           cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
        add_remove_pt = 0;
  }

  CV_SWAP( prev_grey, grey, swap_temp );
  CV_SWAP( prev_pyramid, pyramid, swap_temp );
  CV_SWAP( points[0], points[1], swap_points );
  need_to_init = 0;

  memcpy( image.data, grey->imageData, image.xsize*image.ysize );
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////

void pix_opencv_lk :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::winSizeMessCallback,
		  gensym("winsize"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::nightModeMessCallback,
		  gensym("nightmode"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::qualityMessCallback,
		  gensym("quality"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::initMessCallback,
		  gensym("init"), A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::markMessCallback,
		  gensym("mark"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::deleteMessCallback,
		  gensym("delete"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::clearMessCallback,
		  gensym("clear"), A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::minDistanceMessCallback,
		  gensym("mindistance"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_opencv_lk::maxMoveMessCallback,
		  gensym("maxmove"), A_FLOAT, A_NULL);
}

void  pix_opencv_lk :: winSizeMessCallback(void *data, t_floatarg winsize)
{
    GetMyClass(data)->winSizeMess((float)winsize);
}

void  pix_opencv_lk :: nightModeMessCallback(void *data, t_floatarg nightmode)
{
    GetMyClass(data)->nightModeMess((float)nightmode);
}

void  pix_opencv_lk :: qualityMessCallback(void *data, t_floatarg quality)
{
    GetMyClass(data)->qualityMess((float)quality);
}

void  pix_opencv_lk :: initMessCallback(void *data)
{
    GetMyClass(data)->initMess();
}

void  pix_opencv_lk :: markMessCallback(void *data, t_floatarg mx, t_floatarg my)
{
    GetMyClass(data)->markMess((float)mx, (float)my);
}

void  pix_opencv_lk :: deleteMessCallback(void *data, t_floatarg index)
{
    GetMyClass(data)->deleteMess((float)index);
}

void  pix_opencv_lk :: clearMessCallback(void *data)
{
    GetMyClass(data)->clearMess();
}

void  pix_opencv_lk :: minDistanceMessCallback(void *data, t_floatarg mindistance)
{
    GetMyClass(data)->minDistanceMess((float)mindistance);
}

void  pix_opencv_lk :: maxMoveMessCallback(void *data, t_floatarg maxmove)
{
    GetMyClass(data)->maxMoveMess((float)maxmove);
}

void  pix_opencv_lk :: winSizeMess(float winsize)
{
    if (winsize>1.0) win_size = (int)winsize;
}

void  pix_opencv_lk :: nightModeMess(float nightmode)
{
    if ((nightmode==0.0)||(nightmode==1.0)) night_mode = (int)nightmode;
}

void  pix_opencv_lk :: qualityMess(float quality)
{
    if (quality>0.0) quality = quality;
}

void  pix_opencv_lk :: initMess(void)
{
    need_to_init = 1;
}

void  pix_opencv_lk :: markMess(float mx, float my)
{
  int i;
  int inserted;

    if ( ( mx < 0.0 ) || ( mx >= comp_xsize ) || ( my < 0.0 ) || ( my >= comp_ysize ) )
    {
       return;
    }

    inserted = 0;
    for ( i=0; i<MAX_MARKERS; i++)
    {
       if ( x_xmark[i] == -1 )
       {
          x_xmark[i] = (int)(mx);
          x_ymark[i] = (int)(my);
          post( "pix_opencv_lk : inserted point (%d,%d)", x_xmark[i], x_ymark[i] );
          inserted = 1;
          break;
       }
    }
    if ( !inserted )
    {
       post( "pix_opencv_lk : max markers reached" );
    }
}

void  pix_opencv_lk :: deleteMess(float index)
{
  int i;

    if ( ( index < 1.0 ) || ( index > MAX_MARKERS ) )
    {
       return;
    }

    x_xmark[(int)index-1] = -1;
    x_ymark[(int)index-1] = -1;

}

void  pix_opencv_lk :: clearMess(void)
{
  int i;

    for ( i=0; i<MAX_MARKERS; i++)
    {
      x_xmark[i] = -1;
      x_ymark[i] = -1;
    }

}

void  pix_opencv_lk :: minDistanceMess(float mindistance)
{
    if (mindistance>1.0) min_distance = (int)mindistance;
}

void  pix_opencv_lk :: maxMoveMess(float maxmove)
{
  // has to be more than the size of a point
  if (maxmove>=3.0) maxmove = (int)maxmove;
}

