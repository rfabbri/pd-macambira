/*
 *  pdp2gem : pdp to gem bridge
 *
 *  Holds the contents of a PDP packet and introduce it in the GEM rendering chain
 *
 *  Copyright (c) 2003 Yves Degoyon
 *
 */


#include "pdp2gem.h"
#include "yuv.h"

CPPEXTERN_NEW(pdp2gem)

pdp2gem :: pdp2gem(void)
{
  // initialize the pix block data
  m_pixBlock.image.data = NULL;
  m_pixBlock.image.xsize = 0;
  m_pixBlock.image.ysize = 0;
  m_pixBlock.image.csize = 3;
  m_pixBlock.image.format = GL_RGB;
  m_pixBlock.image.type = GL_UNSIGNED_BYTE;
  m_format = GL_RGB;
  m_csize = 3;
  m_data = NULL;
  m_packet0 = -1;
  m_xsize = 0;
  m_ysize = 0;
  m_dropped = 0;
  m_pdpdata = NULL;
  m_mutex = NULL;

  m_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  if ( pthread_mutex_init(m_mutex, NULL) < 0 )
  {
       perror("pdp2gem : couldn't create mutex");
  }
  else
  {
       post("pdp2gem : created mutex");
  }
 
}

pdp2gem :: ~pdp2gem()
{
  if ( m_mutex ) 
  {
    pthread_mutex_destroy(m_mutex);
    free(m_mutex);
    post("pdp2gem : destroyed mutex");
  }

  deleteBuffer();
}

void pdp2gem :: deleteBuffer()
{
  if ( m_data )
  {
    delete [] m_data;
  }
  m_pixBlock.image.data=NULL;
}

void pdp2gem :: createBuffer()
{
  const int neededXSize = m_xsize;
  const int neededYSize = m_ysize;
  int dataSize;

  deleteBuffer();

  m_pixBlock.image.xsize = neededXSize;
  m_pixBlock.image.ysize = neededYSize;
  m_pixBlock.image.csize = m_csize;
  m_pixBlock.image.format= m_format;

  // +4 from MPEG 
  dataSize = (m_pixBlock.image.xsize * m_pixBlock.image.ysize * m_pixBlock.image.csize)+4; 
  m_data = new unsigned char[dataSize];
  memset(m_data, 0, dataSize);

  m_pixBlock.image.data = m_data;

  post("pdp2gem : created buffer %dx%d", m_xsize, m_ysize );
}

void pdp2gem :: pdpMess(t_symbol *action, t_int pcktno)
{
 t_int psize;
 short int *pY, *pU, *pV;
 unsigned char r,g,b;
 unsigned char y,u,v;
 t_int cpt, px, py;

  if (action == gensym("register_ro"))  
  {
     m_dropped = pdp_packet_copy_ro_or_drop(&m_packet0, pcktno);
  }
  // post("pdp2gem : got pdp packet #%d : dropped : %d", pcktno, m_dropped );

  m_header = pdp_packet_header(m_packet0);

  if ((action == gensym("process")) && (-1 != m_packet0) && (!m_dropped))
  {
    if ( PDP_IMAGE == m_header->type )
    {
       if (pdp_packet_header(m_packet0)->info.image.encoding == PDP_IMAGE_YV12)
       {
          if ( ( m_xsize != (int)m_header->info.image.width ) ||
               ( m_ysize != (int)m_header->info.image.height ) )
          {
            m_xsize = m_header->info.image.width;
            m_ysize = m_header->info.image.height;
            createBuffer();
          }
          m_pdpdata = (short int *)pdp_packet_data(m_packet0);
          psize = m_xsize*m_ysize;
          pY = m_pdpdata;
          pV = m_pdpdata+psize;
          pU = m_pdpdata+psize+(psize>>2);
        
          // lock mutex
          pthread_mutex_lock(m_mutex);

          // copy image data
          for ( py=0; py<m_ysize; py++)
          {
            for ( px=0; px<m_xsize; px++)
            {
              cpt=((m_ysize-py)*m_xsize+px)*3;
              y = *(pY)>>7;
              v = (*(pV)>>8)+128;
              u = (*(pU)>>8)+128;
              m_data[cpt++] = yuv_YUVtoR( y, u, v );
              m_data[cpt++] = yuv_YUVtoG( y, u, v );
              m_data[cpt] = yuv_YUVtoB( y, u, v );
              pY++;
              if ( (px%2==0) && (py%2==0) )
              {
                pV++; pU++;
              }
            }
          }

          // unlock mutex
          pthread_mutex_unlock(m_mutex);

          // free PDP packet
          pdp_packet_mark_unused(m_packet0);
          m_packet0 = -1;
       }
       else
       {
          post("pdp2gem : unsupported image type", m_header->info.image.encoding );
       }
    }
  }

}

void pdp2gem :: startRendering()
{
  m_pixBlock.newimage = 1;
}

void pdp2gem :: render(GemState *state)
{
  if ( m_data )
  {
    // lock mutex
    pthread_mutex_lock(m_mutex);

    m_pixBlock.newimage = 1;
    state->image = &m_pixBlock;
  }
}

void pdp2gem :: postrender(GemState *state)
{
  m_pixBlock.newimage = 0;

  // unlock mutex
  pthread_mutex_unlock(m_mutex);
}

void pdp2gem :: obj_setupCallback(t_class *classPtr)
{
  post( "pdp2gem : a bridge between PDP/PiDiP and GEM v"GEM2PDP_VERSION" (ydegoyon@free.fr)" );
  class_addmethod(classPtr, (t_method)&pdp2gem::pdpCallback, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
  class_sethelpsymbol( classPtr, gensym("pdp2gem.pd") );
}

void pdp2gem :: pdpCallback(void *data, t_symbol *action, t_floatarg fpcktno)
{
  // post("pdp2gem : callback : action : %s : no : %f", action->s_name, fpcktno );
  GetMyClass(data)->pdpMess(action, (int)fpcktno);
}

