/*
 *  pix_2pdp : pix to pdp bridge
 *
 *  Capture the contents of the Gem pix and transform it to a PDP Packet whenever a bang is received
 *
 *  Based on code of gem2pdp by Yves Degoyon
 *  Many thanks to IOhannes M Zmölnig
 *
 *  Copyright (c) 2005 Georg Holzmann <grh@mur.at>
 *  parts Copyright (c) 2005-2006 James Tittle II <tigital@mac.com>
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
  gem_upsidedown = 0;
}

// Image processing
void pix_2pdp::processImage(imageStruct &image)
{
  gem_image = image.data;
  gem_xsize = image.xsize;
  gem_ysize = image.ysize;
  gem_csize = image.csize;
  gem_format = image.format;
  gem_upsidedown = image.upsidedown;
}

// pdp processing
void pix_2pdp::bangMess()
{
  t_int psize, px, py;
  short int *pY, *pY2, *pU, *pV;
  unsigned char g1,g2,g3,g4;
  t_int helper;

  if(gem_image)
  {
    // make pdp packet
    psize = gem_xsize * gem_ysize;
    m_packet0 = pdp_packet_new_image_YCrCb( gem_xsize, gem_ysize);
    m_header = pdp_packet_header(m_packet0);
    m_data = (short int *)pdp_packet_data(m_packet0);

    pY = m_data;
    pY2 = m_data + gem_xsize;
    pV = m_data+psize;
    pU = m_data+psize+(psize>>2);
    
    switch(gem_format)
    {
      // RGB
      case GL_RGB:
      case GL_RGBA:
	  case GL_BGR:
	  case GL_BGRA:
       for ( py=0; py<gem_ysize; py++)
        {
          const t_int py2=(gem_upsidedown)?py:(gem_ysize-py);
          for ( px=0; px<gem_xsize; px++)
          {
            // the way to access the pixels: (C=chRed, chBlue, ...)
            // image[Y * xsize * csize + X * csize + C]
            helper = py2*gem_xsize*gem_csize + px*gem_csize;
            g1=gem_image[helper+chRed];   // R
            g2=gem_image[helper+chGreen]; // G
            g3=gem_image[helper+chBlue];  // B
            
            *(pY) = yuv_RGBtoY( (g1<<16) +  (g2<<8) +  g3 ) << 7;
            *(pV) = ( yuv_RGBtoV( (g1<<16) +  (g2<<8) +  g3 ) - 128 ) << 8;
            *(pU) = ( yuv_RGBtoU( (g1<<16) +  (g2<<8) +  g3 ) - 128 ) << 8;
            pY++;
            if ( (px%2==0) && (py%2==0) )
              pV++; pU++;
          }
        }
        pdp_packet_pass_if_valid(m_pdpoutlet, &m_packet0);
        break;
        
      // YUV
      case GL_YUV422_GEM: {
#ifdef __VEC__
 		YUV422_to_YV12_altivec(pY, pY2, pV, pU, psize);
#else
	    int row=gem_ysize>>1;
		int cols=gem_xsize>>1;
		short u,v;
		unsigned char *pixel = gem_image;
		unsigned char *pixel2 = gem_image + gem_xsize * gem_csize;
        const int row_length = gem_xsize* gem_csize;
                if(0==gem_upsidedown){
                  pixel=gem_image+row_length * (gem_ysize-1);
                  pixel2=gem_image+row_length * (gem_ysize-2);
                }
		while (row--){
		  int col=cols;
		  while(col--){
				u=(pixel[0]-128)<<8; v=(pixel[2]-128)<<8;
				*pU = u;
				*pY++ = (pixel[1])<<7;
				*pV = v;
				*pY++ = (pixel[3])<<7;
				*pY2++ = (pixel2[1])<<7;
				*pY2++ = (pixel2[3])<<7;
				pixel+=4;
				pixel2+=4;
				pU++; pV++;
		  }
                  if(gem_upsidedown){
                    pixel += row_length;
                    pixel2 += row_length;
                  } else {
                    pixel -= 3*row_length;
                    pixel2 -= 3*row_length;
                  }
                  pY += gem_xsize; pY2 += gem_xsize;
		}
#endif // __VEC__
        pdp_packet_pass_if_valid(m_pdpoutlet, &m_packet0);
        } break;
      
      // grey
      case GL_LUMINANCE:
        for ( py=0; py<gem_ysize; py++)
        {
          const t_int py2=(gem_upsidedown)?py:(gem_ysize-py);
          for ( px=0; px<gem_xsize; px++)
          {
            *pY = gem_image[py2*gem_xsize*gem_csize + px*gem_csize] << 7;
            pY++;
            if ( (px%2==0) && (py%2==0) )
            {
              *pV++=128;
              *pU++=128;
            }
          }
        }
        pdp_packet_pass_if_valid(m_pdpoutlet, &m_packet0);
        break;
        
      default:
        post( "pix_2pdp: Sorry, wrong input type!" );
    }
  }
}

#ifdef __VEC__
void pix_2pdp :: YUV422_to_YV12_altivec(short*pY, short*pY2, short*pU, short*pV, size_t psize)
{
  // UYVY UYVY UYVY UYVY
  vector unsigned char *pixels1=(vector unsigned char *)gem_image;
  vector unsigned char *pixels2=(vector unsigned char *)(gem_image+(gem_xsize*2));
  // PDP packet to be filled:
  // first Y plane
  vector signed short *py1 = (vector signed short *)pY;
  // 2nd Y pixel
  vector signed short *py2 = (vector signed short *)pY2;
  // U plane
  vector signed short *pCr = (vector signed short *)pU;
  // V plane
  vector signed short *pCb = (vector signed short *)pV;
  vector signed short uvSub = (vector signed short)( 128, 128, 128, 128,
													 128, 128, 128, 128 );
  vector signed short yShift = (vector signed short)( 7, 7, 7, 7, 7, 7, 7, 7 );
  vector signed short uvShift = (vector signed short)( 8, 8, 8, 8, 8, 8, 8, 8 );
  
  vector signed short tempY1, tempY2, tempY3, tempY4,
		tempUV1, tempUV2, tempUV3, tempUV4, tempUV5, tempUV6;

  vector unsigned char uvPerm = (vector unsigned char)( 16, 0, 17, 4, 18,  8, 19, 12,   // u0..u3
  														20, 2, 21, 6, 22, 10, 23, 14 ); // v0..v3

  vector unsigned char uPerm = (vector unsigned char)( 0, 1, 2, 3, 4, 5, 6, 7, 
													   16,17,18,19,20,21,22,23);
  vector unsigned char vPerm = (vector unsigned char)( 8, 9, 10,11,12,13,14,15,
													   24,25,26,27,28,29,30,31);
  
  vector unsigned char yPerm = (vector unsigned char)( 16, 1, 17,  3, 18,  5, 19,  7, // y0..y3
													   20, 9, 21, 11, 23, 13, 25, 15);// y4..y7
  vector unsigned char zeroVec = (vector unsigned char)(0);
  
  int row=gem_ysize>>1;
  int cols=gem_xsize>>4;
  
  while(row--){
    int col=cols;
    while(col--){
      tempUV1 = (vector signed short) vec_perm( *pixels1, zeroVec, uvPerm);
      tempY1  = (vector signed short) vec_perm( *pixels1, zeroVec, yPerm);
      tempY2  = (vector signed short) vec_perm( *pixels2, zeroVec, yPerm);
	  pixels1++;pixels2++;
      
      tempUV2 = (vector signed short) vec_perm( *pixels1, zeroVec, uvPerm);
      tempY3  = (vector signed short) vec_perm( *pixels1, zeroVec, yPerm);
      tempY4  = (vector signed short) vec_perm( *pixels2, zeroVec, yPerm);
	  pixels1++;pixels2++;
  
	  tempUV3 = vec_sub( tempUV1, uvSub );
	  tempUV4 = vec_sub( tempUV2, uvSub );
	  tempUV5 = vec_sl( tempUV3, uvShift );
	  tempUV6 = vec_sl( tempUV4, uvShift );
	  
	  *pCb = vec_perm( tempUV5, tempUV6, uPerm );
	  *pCr = vec_perm( tempUV5, tempUV6, vPerm );
	  pCr++; pCb++;

	  *py1++ = vec_sl( tempY1, yShift);
      *py2++ = vec_sl( tempY2, yShift);
      *py1++ = vec_sl( tempY3, yShift);
      *py2++ = vec_sl( tempY4, yShift);      
	}

	py1+=(gem_xsize>>3); py2+=(gem_xsize>>3);
	pixels1+=(gem_xsize*2)>>4; pixels2+=(gem_xsize*2)>>4;
  }
}
#endif

void pix_2pdp::obj_setupCallback(t_class *classPtr)
{
  post( "pix_2pdp : a bridge between a Gem pix and PDP/PiDiP, Georg Holzmann 2005 <grh@mur.at> & tigital 2005 <tigital@mac.com>" );
  class_addmethod(classPtr, (t_method)&pix_2pdp::bangMessCallback,
    	    gensym("bang"), A_NULL);
}

void pix_2pdp::bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}
