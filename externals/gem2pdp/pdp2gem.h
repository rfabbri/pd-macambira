/*
 *  pdp2gem : pdp to gem bridge
 *
 *  Holds the contents of a PDP packet and introduce it in the GEM rendering chain
 *
 *  Copyright (c) 2003 Yves Degoyon
 *
 */


#ifndef __PDP_2_GEM_H
#define __PDP_2_GEM_H

#define GEM_MOVIE_NONE 0
#define GEM_MOVIE_AVI  1
#define GEM_MOVIE_MPG  2
#define GEM_MOVIE_MOV  3

#include <string.h>
#include <stdio.h>

#include "Base/GemBase.h"
#include "Base/GemPixUtil.h"
#include "Base/config.h"
#include "pdp-light.h"
#include <pthread.h>

class GEM_EXTERN pdp2gem : public GemBase
{
  CPPEXTERN_HEADER(pdp2gem, GemBase)
    
 public:
  pdp2gem(void);

 protected:
  virtual ~pdp2gem(void);
  virtual void createBuffer(void);
  virtual void deleteBuffer(void);
  virtual void render(GemState *state);
  virtual void postrender(GemState *state);
  virtual void startRendering(void);
  virtual void stopRendering(void) {}
  static void pdpCallback(void *data, t_symbol *action, t_floatarg fpcktno);
  void pdpMess(t_symbol *action, t_int fpcktno);
	
  unsigned char *m_data;
  pixBlock    	m_pixBlock;
  t_int		m_xsize;
  t_int		m_ysize;
  t_int         m_csize;
  t_int         m_format;
  pthread_mutex_t *m_mutex;

  // PDP data
  t_int         m_packet0;
  t_int         m_dropped;
  t_pdp         *m_header;
  short int     *m_pdpdata;
};

#endif	// for header file
