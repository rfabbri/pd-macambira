
#include "tof.h"
#include "param.h"
#include <stdio.h>
#include <fcntl.h>



static t_class *paramFile_class;
static t_class *paramFile_inlet2_class;
struct _paramFile_inlet2;

typedef struct _paramFile
{
  t_object                    x_obj;
  //t_outlet					  *outlet;
  t_canvas			          *canvas;
  t_symbol 					*basename;
  struct _paramFile_inlet2	  *inlet2;
} t_paramFile;

typedef struct _paramFile_inlet2 {
	t_object                    x_obj;
	t_paramFile					*x;
} t_paramFile_inlet2;



static t_symbol* paramFile_makefilename(t_symbol* basename, t_float f) {
	
	if (f < 0) f = 0;
	if ( f > 127) f = 127;
	
	int i = (int) f;
	int length = strlen(basename->s_name)+11;
	char* buf = getbytes( length * sizeof (*buf));	
	sprintf(buf,"%s-%03d.param",basename->s_name,i);
	//strcpy(buf,basename->s_name);
	//strcat(buf,".param");
	t_symbol* filename = gensym(buf);
	freebytes(buf, length * sizeof (*buf));
	//post("File name:%s",filename->s_name);
	return filename;
}


static void paramFile_write(t_paramFile *x, t_float f) {
	
	
	t_symbol* filename = paramFile_makefilename(x->basename,f);
	post("Writing: %s",filename->s_name);
	if ( param_write(x->canvas,filename) ) pd_error(x,"%s: write failed", filename->s_name);

}


static void paramFile_read(t_paramFile *x, t_float f) {
	
	
	t_symbol* filename = paramFile_makefilename(x->basename,f);
	post("Reading: %s",filename->s_name);
	if (param_read(x->canvas, filename)) pd_error(x, "%s: read failed", filename->s_name);

	
}



static void paramFile_bang(t_paramFile *x) {
	
	paramFile_write(x,0);	
}


static void paramFile_float(t_paramFile *x, t_float f) {
	
	paramFile_write(x,f);
		
}

static void paramFile_inlet2_bang(t_paramFile_inlet2 *inlet2) {
	
	paramFile_read(inlet2->x,0);
	
}

static void paramFile_inlet2_float(t_paramFile_inlet2 *inlet2,t_float f) {
	
	paramFile_read(inlet2->x,f);
	
}

static void paramFile_free(t_paramFile *x)
{
	
	if(x->inlet2) pd_free((t_pd *)x->inlet2);
	
    
}


static void* paramFile_new(t_symbol *s, int ac, t_atom *av) {
  t_paramFile *x = (t_paramFile *)pd_new(paramFile_class);
  t_paramFile_inlet2 *inlet2 = (t_paramFile_inlet2 *)pd_new(paramFile_inlet2_class);
  
  inlet2->x = x;
  x->inlet2 = inlet2;
  
  t_canvas* canvas = tof_get_canvas();
  x->canvas = tof_get_root_canvas(canvas);
  t_symbol* canvasname = tof_get_canvas_name(x->canvas);
  
  // remove the .pd (actually removes everything after the .)
  int length = strlen(canvasname->s_name) + 1;
  char* buf = getbytes( (length ) * sizeof (*buf));	
  strcpy(buf,canvasname->s_name);
  char* lastperiod = strrchr(buf,'.');
  if ( lastperiod != NULL ) {
	  *lastperiod = '\0';
	  x->basename = gensym(buf);
  } else {
	x->basename = canvasname;
  }
  freebytes(buf, (length ) * sizeof (*buf));
  
   //x->outlet = outlet_new(&x->x_obj, &s_list);
   
   inlet_new((t_object *)x, (t_pd *)inlet2, 0, 0);
   
  return (x);
  
}


void paramFile_setup(void) {
  paramFile_class = class_new(gensym("paramFile"),
    (t_newmethod)paramFile_new, (t_method)paramFile_free,
    sizeof(t_paramFile), 0, A_GIMME, 0);
  
 class_addbang(paramFile_class, paramFile_bang);
 class_addfloat(paramFile_class, paramFile_float);
 
 paramFile_inlet2_class = class_new(gensym("paramFile_inlet2"),
    0, 0, sizeof(t_paramFile_inlet2), CLASS_PD | CLASS_NOINLET, 0);
 
 class_addbang(paramFile_inlet2_class, paramFile_inlet2_bang);
 class_addfloat(paramFile_inlet2_class, paramFile_inlet2_float);
 
 class_addmethod(paramFile_class, (t_method) paramFile_read, gensym("load"), A_DEFFLOAT,0);
 class_addmethod(paramFile_class, (t_method) paramFile_write, gensym("save"), A_DEFFLOAT,0);
 
}


