
/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) Franz Zotter
 *
 *   2105:forum::f�r::uml�ute:2007
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

#include "zexy.h"

#ifdef __WIN32__
# define snprintf _snprintf
#endif

#include <stdio.h>
#include <string.h>

/* fwriteln: writes messages continuously into a file that
 * doesn't necessarily need to fit into the RAM of your system
 *
 * Franz Zotter zotter@iem.at, 2007
 * Institute of Electronic Music and Acoustics
 *
 * parts of this externals were copied from Iohannes zmoelnig's 
 * iemmatrix
 */

static t_class *fwriteln_class;

typedef struct fwriteln
{
   t_object x_ob;
   FILE *x_file;
   char *x_filename;
   char *x_textbuf;
} t_fwriteln;


static void fwriteln_close (t_fwriteln *x)
{
   if(x->x_file) 
      fclose(x->x_file);
   x->x_file=0;
   if(x->x_filename)
      freebytes(x->x_filename, sizeof(char)*MAXPDSTRING);
   x->x_filename=0;
   if(x->x_textbuf)
      freebytes(x->x_textbuf, sizeof(char)*MAXPDSTRING);
   x->x_textbuf=0;
}

static void fwriteln_open (t_fwriteln *x, t_symbol *s, t_symbol*type)
{
   char filename[MAXPDSTRING];
   sys_bashfilename (s->s_name, filename);
   filename[MAXPDSTRING-1]=0;
   fwriteln_close (x);

   if(0==type || type!=gensym("cr")) {
     pd_error(x, "unknown type '%s'", (type)?type->s_name:"");
     return;
   }

   if (!(x->x_file=fopen(filename, "w"))) {
      pd_error(x, "failed to open %128s",filename);
      return;
   }
   x->x_filename=(char*)getbytes(sizeof(char)*strlen(filename));
   strcpy(x->x_filename,filename);
   x->x_textbuf = (char *) getbytes (MAXPDSTRING * sizeof(char));
}

static void fwriteln_write (t_fwriteln *x, t_symbol *s, int argc, t_atom *argv)
{
   int length=0;
   char *text=x->x_textbuf;
   if (x->x_file) {
      if ((s!=gensym("list"))||(argv->a_type==A_SYMBOL)) {
         snprintf(text,MAXPDSTRING,"%s ", s->s_name);
         text[MAXPDSTRING-1]=0;
         length=strlen(text);
         if (fwrite(text, length*sizeof(char),1,x->x_file) < 1) {
            pd_error(x, "failed to write %128s",x->x_filename);
            freebytes (text, MAXPDSTRING * sizeof(char));
            fwriteln_close(x);
            return;
         }
      }
      while (argc--)
      {
         switch (argv->a_type) {
            case A_FLOAT:
               snprintf(text,MAXPDSTRING,"%.16g ", atom_getfloat(argv));
               text[MAXPDSTRING-1]=0;
               length=strlen(text);
               if (fwrite(text, length*sizeof(char),1,x->x_file) < 1) {
                  pd_error(x, "failed to write %128s",x->x_filename);
                  freebytes (text, MAXPDSTRING * sizeof(char));
                  fwriteln_close(x);
                  return;
               }
               break;
            case A_SYMBOL:
               snprintf(text,MAXPDSTRING,"%s ", atom_getsymbol(argv)->s_name);
               text[MAXPDSTRING-1]=0;
               length=strlen(text);
               if (fwrite(text, length*sizeof(char),1,x->x_file) < 1) {
                  pd_error(x, "failed to write %128s",x->x_filename);
                  freebytes (text, MAXPDSTRING * sizeof(char));
                  fwriteln_close(x);
                  return;
               }
               break;
            case A_COMMA:
               snprintf(text,MAXPDSTRING,", ");
               length=strlen(text);
               if (fwrite(text, length*sizeof(char),1,x->x_file) < 1) {
                  pd_error(x, "failed to write %128s",x->x_filename);
                  freebytes (text, MAXPDSTRING * sizeof(char));
                  fwriteln_close(x);
                  return;
               }
               break;
            case A_SEMI:
               snprintf(text,MAXPDSTRING,"; ");
               length=strlen(text);
               if (fwrite(text, length*sizeof(char),1,x->x_file) < 1) {
                  pd_error(x, "failed to write %128s",x->x_filename);
                  freebytes (text, MAXPDSTRING * sizeof(char));
                  fwriteln_close(x);
                  return;
               }
               break;
         }
         argv++;
      }

      snprintf(text,MAXPDSTRING,"\n");
      length=strlen(text);
      if (fwrite(text, length*sizeof(char),1,x->x_file) < 1) {
         pd_error(x, "failed to write %128s",x->x_filename);
         freebytes (text, MAXPDSTRING * sizeof(char));
         fwriteln_close(x);
         return;
      }
   }
   else {
      pd_error(x, "no file opened for writing");
   }
}
static void fwriteln_free (t_fwriteln *x)
{
   fwriteln_close(x);
}

static void *fwriteln_new(void)
{
   t_fwriteln *x = (t_fwriteln *)pd_new(fwriteln_class);
   x->x_filename=0;
   x->x_file=0;
   x->x_textbuf=0;
   return (void *)x;
}

void fwriteln_setup(void)
{
   fwriteln_class = class_new(gensym("fwriteln"), (t_newmethod)fwriteln_new, 
         (t_method) fwriteln_free, sizeof(t_fwriteln), 0, 0);
   class_addmethod(fwriteln_class, (t_method)fwriteln_open, gensym("open"), A_SYMBOL, A_DEFSYM, 0);
   class_addmethod(fwriteln_class, (t_method)fwriteln_close, gensym("close"), A_NULL, 0);
   class_addanything(fwriteln_class, (t_method)fwriteln_write);

   zexy_register("fwriteln");
}

