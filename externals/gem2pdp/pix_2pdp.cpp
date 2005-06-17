/*
 *  pix_2pdp : pix to pdp bridge
 *
 *  Capture the contents of the Gem pix and transform it to a PDP Packet whenever a bang is received
 *
 *  Based on code of gem2pdp by Yves Degoyon
 *  Many thanks to IOhannes M Zmölnig
 *
 *  Copyright (c) 2005 Georg Holzmann <grh@mur.at>
 *
 */

#include "pix_2pdp.h"
#include "yuv.h"

CPPEXTERN_NEW(pix_2pdp)

pix_2pdp::pix_2pdp(void)
{
  gem_image = NULL;
  m_pdpoutlet = outlet_new(this->x_obj, &s_anything);
}

pix_2pdp::~pix_2pdp()
{
  gem_image = NULL;
  gem_xsize = 0;
  gem_ysize = 0;
  gem_csize = 0;
}

// Image processing
void pix_2pdp::processImage(imageStruct &image)
{
  gem_image = image.data;
  gem_xsize = image.xsize;
  gem_ysize = image.ysize;
  gem_csize = image.csize;
  gem_format = image.format;
}

// pdp processing
void pix_2pdp::bangMess()
{
  t_int psize, px, py;
  short int *pY, *pU, *pV;
  unsigned char r,g,b;
  t_int helper;

  if(gem_image)
  {
    if(gem_format == GL_RGBA)
    {
      // make pdp packet
      psize = gem_xsize * gem_ysize;
      m_packet0 = pdp_packet_new_image_YCrCb( gem_xsize, gem_ysize);
      m_header = pdp_packet_header(m_packet0);
      m_data = (short int *)pdp_packet_data(m_packet0);

      pY = m_data;
      pV = m_data+psize;
      pU = m_data+psize+(psize>>2);
  
      for ( py=0; py<gem_ysize; py++)
      {
        for ( px=0; px<gem_xsize; px++)
        {
          // the way to access the pixels: (C=chRed, chBlue, ...)
          // image[Y * xsize * csize + X * csize + C]
          helper = py*gem_xsize*gem_csize + px*gem_csize;
          r=gem_image[helper+chRed];
          g=gem_image[helper+chGreen];
          b=gem_image[helper+chBlue];
          
          *(pY) = yuv_RGBtoY( (r<<16) +  (g<<8) +  b ) << 7;
          *(pV) = ( yuv_RGBtoV( (r<<16) +  (g<<8) +  b ) - 128 ) << 8;
          *(pU) = ( yuv_RGBtoU( (r<<16) +  (g<<8) +  b ) - 128 ) << 8;
          pY++;
          if ( (px%2==0) && (py%2==0) )
          {
            pV++; pU++;
          }
        }
      }

      pdp_packet_pass_if_valid(m_pdpoutlet, &m_packet0);

    }
    else
    {
      post( "pix_2pdp: Sorry, Gem-input RGB only for now!" );
    }
  }
}

void pix_2pdp::obj_setupCallback(t_class *classPtr)
{
  post( "pix_2pdp : a bridge between a Gem pix and PDP/PiDiP, Georg Holzmann 2005 <grh@mur.at>" );
  class_addmethod(classPtr, (t_method)&pix_2pdp::bangMessCallback,
    	    gensym("bang"), A_NULL);
}

void pix_2pdp::bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}
