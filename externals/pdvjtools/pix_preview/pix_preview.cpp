////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
//  pix_preview
//
//  0409:forum::für::umläute:2000
//  IOhannes m zmoelnig
//  mailto:zmoelnig@iem.kug.ac.at
//
/////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <sstream>
using namespace std;
#include "stdio.h"


#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"
#include "t_tk.h"

#include "pix_preview.h"

int guidebug=0;

#define COLORGRID_SYS_VGUI2(a,b) if (guidebug) \
                         post(a,b);\
                         sys_vgui(a,b)

#define COLORGRID_SYS_VGUI3(a,b,c) if (guidebug) \
                         post(a,b,c);\
                         sys_vgui(a,b,c)

#define COLORGRID_SYS_VGUI4(a,b,c,d) if (guidebug) \
                         post(a,b,c,d);\
                         sys_vgui(a,b,c,d)

#define COLORGRID_SYS_VGUI5(a,b,c,d,e) if (guidebug) \
                         post(a,b,c,d,e);\
                         sys_vgui(a,b,c,d,e)

#define COLORGRID_SYS_VGUI6(a,b,c,d,e,f) if (guidebug) \
                         post(a,b,c,d,e,f);\
                         sys_vgui(a,b,c,d,e,f)

#define COLORGRID_SYS_VGUI7(a,b,c,d,e,f,g) if (guidebug) \
                         post(a,b,c,d,e,f,g );\
                         sys_vgui(a,b,c,d,e,f,g)

#define COLORGRID_SYS_VGUI8(a,b,c,d,e,f,g,h) if (guidebug) \
                         post(a,b,c,d,e,f,g,h );\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define COLORGRID_SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)


    char *fdata="R0lGODlhHAAcAIABAAAAAP///ywAAAAAHAAcAAACGoSPqcvtD6OctNqLs968+w+G4kiW5omm6ooUADs=";


/* base64 conversion*/

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}















CPPEXTERN_NEW_WITH_TWO_ARGS(pix_preview, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT)


  /////////////////////////////////////////////////////////
  //
  // pix_preview
  //
  /////////////////////////////////////////////////////////
  // Constructor
  //
  /////////////////////////////////////////////////////////
  pix_preview :: pix_preview(t_floatarg fx, t_floatarg fy)
{
  #include "pix_preview.tk2c"
  xsize = (int)fx;
  ysize = (int)fy;
  m_csize = 3;


    image_widgetbehavior.w_getrectfn =     image_getrect;
    image_widgetbehavior.w_displacefn =    image_displace;
    image_widgetbehavior.w_selectfn =   image_select;
    image_widgetbehavior.w_activatefn =   image_activate;
    image_widgetbehavior.w_deletefn =   image_delete;
    image_widgetbehavior.w_visfn =   image_vis;
#if (PD_VERSION_MINOR > 31) 
    image_widgetbehavior.w_clickfn = NULL;
    image_widgetbehavior.w_propertiesfn = NULL; 
#endif
#if PD_MINOR_VERSION < 37
    image_widgetbehavior.w_savefn =   image_save;
#endif
    

  class_setwidget(pix_preview_class,&image_widgetbehavior);

  if (xsize < 0) xsize = 0;
  if (ysize < 0) ysize = 0;

  m_xsize = xsize;
  m_ysize = ysize;

  oldimagex = xsize;
  oldimagey = ysize;

  m_bufsize = m_xsize * m_ysize * m_csize;

  m_buffer = new t_atom[m_bufsize];

  //m_dataOut = outlet_new(this->x_obj, &s_list);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_preview :: ~pix_preview()
{
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_preview :: processImage(imageStruct &image)
{
  int x = m_xsize, y = m_ysize, c = m_csize;

  if (image.xsize != oldimagex) {
    oldimagex = image.xsize;
    m_xsize = ((!xsize) || (xsize > oldimagex))?oldimagex:xsize;
  }
  if (image.ysize != oldimagey) {
    oldimagey = image.ysize;
    m_ysize = ((!ysize) || (ysize > oldimagey))?oldimagey:ysize;
  }

  if (image.csize != m_csize) m_csize = image.csize;

  if ( (m_xsize != x) || (m_ysize != y) || (m_csize != c) ) {
    // resize the image buffer
    if(m_buffer)delete [] m_buffer;
    m_bufsize = m_xsize * m_ysize * m_csize;
    m_buffer = new t_atom[m_bufsize];

    m_xstep = m_csize * ((float)image.xsize/(float)m_xsize);
    m_ystep = m_csize * ((float)image.ysize/(float)m_ysize) * image.xsize;
  }

  m_data = image.data;
}

/////////////////////////////////////////////////////////
// processYUVImage
//
/////////////////////////////////////////////////////////
void pix_preview :: processYUVImage(imageStruct &image)
{
    int x = m_xsize, y = m_ysize, c = m_csize;

  if (image.xsize != oldimagex) {
    oldimagex = image.xsize;
    m_xsize = ((!xsize) || (xsize > oldimagex))?oldimagex:xsize;
  }
  if (image.ysize != oldimagey) {
    oldimagey = image.ysize;
    m_ysize = ((!ysize) || (ysize > oldimagey))?oldimagey:ysize;
  }

  if (image.csize != m_csize) m_csize = image.csize;

  if ( (m_xsize != x) || (m_ysize != y) || (m_csize != c) ) {
    // resize the image buffer
    if(m_buffer)delete [] m_buffer;
    m_bufsize = m_xsize * m_ysize * m_csize;
    m_buffer = new t_atom[m_bufsize];

    m_xstep = m_csize * ((float)image.xsize/(float)m_xsize);
    m_ystep = m_csize * ((float)image.ysize/(float)m_ysize) * image.xsize;
  }

  m_data = image.data;
}

/////////////////////////////////////////////////////////
// trigger
//
/////////////////////////////////////////////////////////
void pix_preview :: trigger()
{
  if (!m_data) return;
  
  int n = m_ysize, m = 0;
  int i = 0;

  unsigned char *data, *line;
  stringstream sx,sy;

	fprintf (stderr,"%d %d %d %d %d %d \n",xsize, ysize,m_xsize,  m_ysize,oldimagex,oldimagey);

	std::string pnm;
	std::string pnm64;
	pnm += "P6\n";
		sx << m_xsize;
	pnm += sx.str();
	pnm += " ";
		sy << m_ysize;
	pnm += sy.str();
	pnm += "\n255\n";

	//fprintf (stderr,"%s",pnm.c_str());
	
        /*/escriu el contingut de data a un arxiu.*/
	char* ig_path = "/tmp/pixdump01.pnm";
  	FILE * fp = fopen(ig_path, "w");
        fprintf (fp, "P6\n%d %d\n255\n", m_xsize, m_ysize);
  data = line = m_data;
  switch(m_csize){
  case 4:
/*
    while (n > 0) {
      while (m < m_xsize) {
	int r, g, b, a;
	r = (int)data[chRed];
                fprintf (fp, "%c", (char)r);
	i++;
	g = (int)data[chGreen];
                fprintf (fp, "%c", (char)g);
	i++;
	b = (int)data[chBlue];
                fprintf (fp, "%c", (char)b);
	i++;
	a = (int)data[chAlpha];
	i++;
	m++;
	data = line + (int)(m_xstep * (float)m);
      }
      m = 0;
      n--;
      line = m_data + (int)(m_ystep*n);
      data = line;
    }
        fclose (fp);
*/
    while (n > 0) {
      while (m < m_xsize) {
	int r, g, b, a;
	r = (int)data[chRed];
                pnm += (char)r;
                fprintf (fp, "%c", (char)r);
	i++;
	g = (int)data[chGreen];
                pnm += (char)g;
                fprintf (fp, "%c", (char)g);
	i++;
	b = (int)data[chBlue];
                pnm += (char)b;
                fprintf (fp, "%c", (char)b);
	i++;
	a = (int)data[chAlpha];
	i++;
	m++;
	data = line + (int)(m_xstep * (float)m);
      }
      m = 0;
      n--;
      line = m_data + (int)(m_ystep*n);
      data = line;
    }
        fclose (fp);

	
	//std::cout << "NOT encoded: " << pnm << std::endl;

	//pnm64 = base64_encode(reinterpret_cast<const unsigned char*>(pnm.c_str()), pnm.length());
	//std::cout << "encoded: " << pnm64 << std::endl;


		
	//m_glist = (t_glist *) canvas_getcurrent();
	

	//sys_vgui("img%x put {%s}\n", this->x_obj, reinterpret_cast<const unsigned char*>(pnm64.c_str()) );
    	image_drawme((pix_preview *)this->x_obj, (t_glist *) this->getCanvas(), 0, m_xsize, m_ysize);
//	sys_vgui(".x%x.c coords %xS %d %d\n",
//		   this->getCanvas(), this->x_obj,
//		   text_xpix(this->x_obj, (t_glist *)this->getCanvas()) + (m_xsize/2), text_ypix(this->x_obj, (t_glist *)this->getCanvas()) + (m_ysize/2));
		   //fprintf (stderr, "%x %x - %d %d\n",x,(t_object*)x, text_xpix((t_object*)x, glist), text_ypix((t_object*)x, glist));
	
    break;
  case 2:
    while (n < m_ysize) {
      while (m < m_xsize/2) {
	float y,u,y1,v;
	u = (float)data[0] / 255.f;
	SETFLOAT(&m_buffer[i], u);
	i++;
	y = (float)data[1] / 255.f;
	SETFLOAT(&m_buffer[i], y);
	i++;
	v = (float)data[2] / 255.f;
	SETFLOAT(&m_buffer[i], v);
	i++;
	y1 = (float)data[3] / 255.f;
	SETFLOAT(&m_buffer[i], y1);
	i++;
	m++;
	data = line + (int)(m_xstep * (float)m);
      }
      m = 0;
      n++;
      line = m_data + (int)(m_ystep*n);
      data = line;
    }
  case 1:  default:
    int datasize=m_xsize*m_ysize*m_csize/4;
      while (datasize--) {
	float v;
	v = (float)(*data++) / 255.f;	  SETFLOAT(&m_buffer[i], v);
	v = (float)(*data++) / 255.f;	  SETFLOAT(&m_buffer[i+1], v);
	v = (float)(*data++) / 255.f;	  SETFLOAT(&m_buffer[i+2], v);
	v = (float)(*data++) / 255.f;	  SETFLOAT(&m_buffer[i+3], v);
	i+=4;
      }
  }
  //outlet_list(m_dataOut, gensym("list"), i, m_buffer);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_preview :: obj_setupCallback(t_class *classPtr)
{
  class_addbang(classPtr, (t_method)&pix_preview::triggerMessCallback);
}

void pix_preview :: triggerMessCallback(void *data)
{
  GetMyClass(data)->trigger();
}




/* widget helper functions */




void pix_preview :: image_drawme(pix_preview *x, t_glist *glist, int firsttime, int m_xsize, int m_ysize)
{
	char* ig_path = "/tmp/pixdump01.pnm";
       if (firsttime) {

	  sys_vgui("image create photo img%x -data {%s}\n",x,fdata);
	  sys_vgui(".x%x.c create image %d %d -image img%x -tags %xS\n", 
		   glist_getcanvas(glist),text_xpix((t_object*)x, glist)+14, text_ypix((t_object*)x, glist)+14,x,x);
	  //fprintf (stderr, "%x %x - %d %d \n",x,(t_object*)x, text_xpix((t_object*)x, glist), text_ypix((t_object*)x, glist));
     }     
     else {
            sys_vgui(".x%x.c delete %xS\n", glist_getcanvas(glist), x);
            sys_vgui(".x%x.c delete img%x\n", glist_getcanvas(glist), x);
        sys_vgui("image create photo img%x -file %s\n",x,ig_path);
        sys_vgui(".x%x.c create image %d %d -image img%x -tags %xS\n",
		   glist_getcanvas(glist),text_xpix((t_object*)x, glist)+(m_xsize/2), text_ypix((t_object*)x, glist)+(m_ysize/2),x,x);
	  //sys_vgui(".x%x.c coords %xS \
%d %d\n",
	//	   glist_getcanvas(glist), x,
	//	   text_xpix((t_object*)x, glist), text_ypix((t_object*)x, glist));
		   //fprintf (stderr, "%x %x - %d %d\n",x,(t_object*)x, text_xpix((t_object*)x, glist), text_ypix((t_object*)x, glist));
     }

}

void pix_preview :: image_erase(pix_preview* x,t_glist* glist)
{
     int n;
     sys_vgui(".x%x.c delete %xS\n",
	      glist_getcanvas(glist), x);

}
	

/* ------------------------ image widgetbehaviour----------------------------- */


void pix_preview :: image_getrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{    
    int width, height;
    pix_preview* x = (pix_preview*)z;


    width = 250;
    height = 50;
    *xp1 = text_xpix((t_object*)x, glist);
    *yp1 = text_ypix((t_object*)x, glist);
    *xp2 = text_xpix((t_object*)x, glist) + width;
    *yp2 = text_ypix((t_object*)x, glist) + height;
}

void pix_preview :: image_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_object *x = (t_object *)z;
    x->te_xpix += dx;
    x->te_ypix += dy;
    sys_vgui(".x%x.c coords %xSEL %d %d %d %d\n",
		   glist_getcanvas(glist), x,
		   text_xpix(x, glist), text_ypix(x, glist),
		   text_xpix(x, glist) + 25, text_ypix(x, glist) + 25);

    //image_drawme((pix_preview *)x, glist, 0);
    canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
}

void pix_preview :: image_select(t_gobj *z, t_glist *glist, int state)
{     t_object *x = (t_object *)z;
     if (state) {
	  sys_vgui(".x%x.c create rectangle \
%d %d %d %d -tags %xSEL -outline blue\n",
		   glist_getcanvas(glist),
		   text_xpix(x, glist), text_ypix(x, glist),
		   text_xpix(x, glist) + 25, text_ypix(x, glist) + 25,
		   x);
     }
     else {
	  sys_vgui(".x%x.c delete %xSEL\n",
		   glist_getcanvas(glist), x);
     }
}


void pix_preview :: image_activate(t_gobj *z, t_glist *glist, int state)
{
}

void pix_preview :: image_delete(t_gobj *z, t_glist *glist)
{

    t_text *x = (t_text *)z;
    pix_preview* s = (pix_preview*)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
    image_erase(s, glist);
}

       
void pix_preview :: image_vis(t_gobj *z, t_glist *glist, int vis)
{
    pix_preview* s = (pix_preview*)z;
    if (vis)
	 image_drawme(s, glist, 1, 28, 28);
}

