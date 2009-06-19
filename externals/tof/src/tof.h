
#ifndef TOF
#define TOF
#include "m_pd.h"
#include "g_canvas.h" 
#include <string.h>


#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)
#endif


char tof_buf_temp_a[MAXPDSTRING];
char tof_buf_temp_b[MAXPDSTRING];


/* 
 * ALLOCATE WITH THE FOLLOWING LINE
 * t_atom *at = getbytes((SIZE)*sizeof(*at));	
 * FREE WITH THIS ONE
 * freebytes(at, (SIZE)*sizeof(*at));
*/
static void tof_copy_atoms(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static t_symbol* tof_get_dollar(t_canvas* canvas, t_symbol* dollar) {
	return canvas_realizedollar(canvas, dollar);
}



static int tof_path_is_absolute(char *dir, int length)
{
    if ((length && dir[0] == '/') || (dir[0] == '~')
#ifdef MSW
        || dir[0] == '%' || (length > 2 && (dir[1] == ':' && dir[2] == '/'))
#endif
        )
    {
        return 1;
    } else {
        return 0;            
    }
}


static int tof_canvas_is_not_subpatch(t_canvas* canvas) {
	
	return canvas_isabstraction(canvas);
}


static t_canvas* tof_get_canvas(void) 
{
    return glist_getcanvas((t_glist *)canvas_getcurrent());
}


static t_symbol* tof_get_dir(t_canvas* canvas) {
	
	return canvas_getdir(canvas);
}

static t_symbol* tof_get_canvas_name(t_canvas* canvas) {
	
    return	canvas->gl_name;
    
}

// Use &s_list selector if you do not want to process the selector

static int tof_anything_to_string( t_symbol* s, int ac, t_atom* av,char* buffer ) {

    
	
	if ( s == &s_bang) {
		buffer[0] = '\0';
		return 0;
	} else if ( s == &s_symbol) {
		if ( ac ) {
			 strcpy(buffer,atom_getsymbol(av)->s_name);
		 } else {
			 buffer[0] = '\0';
		 }
		 return strlen(buffer);
		 
	} else {
		
		char* buf;
		t_binbuf* binbuf = binbuf_new();
		
		if ( !(s == &s_list || s == &s_float )  ) {
			t_atom selector;
			SETSYMBOL(&selector,s);
			binbuf_add(binbuf, 1, &selector);
		
		}
		
		binbuf_add(binbuf, ac, av);
	
		int length;
		
		binbuf_gettext(binbuf, &buf, &length);
		
		
		int i;
		for (i = 0; i < length; i++ ) {
			buffer[i] = buf[i];
		}
		
		
		buffer[length] = '\0';
		
		freebytes(buf, length);
		binbuf_free(binbuf);
		
		return length;
    }
	buffer[0] = '\0';
	return 0;

}


static t_canvas* tof_get_root_canvas(t_canvas* canvas) 
{
    // Find the proper parent canvas
    while ( canvas->gl_owner) {
       
        canvas = canvas->gl_owner;
    }
    return canvas;
}

static void tof_get_canvas_arguments(t_canvas *canvas, int *ac_p, t_atom **av_p) {
	canvas_setcurrent(canvas);
    canvas_getargs(ac_p , av_p);
	canvas_unsetcurrent(canvas);
}



// set selector
static void tof_set_selector(t_symbol** selector_sym_p,int* ac_p, t_atom** av_p ) {
	 if(!(*ac_p)) {
         *selector_sym_p = &s_bang;
      } else if(IS_A_SYMBOL(*av_p, 0)) {
			*selector_sym_p = atom_getsymbol(*av_p);
			*ac_p = (*ac_p)-1, 
			*av_p = (*av_p)+1;
		} else{
			*selector_sym_p = &s_list;
		}
}

static int tof_get_tagged_argument(char tag, int ac, t_atom *av, int *start, int *count) {
	int i;
	
    if ( ac == 0 || *start >= ac) return 0;

    for ( i= *start + 1; i < ac; i++ ) {
        if ( (av+i)->a_type == A_SYMBOL 
          && (atom_getsymbol(av+i))->s_name[0] == tag) break;
     }
     *count = i - *start;

     return (i-*start);     
}



