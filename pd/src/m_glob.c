/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_imp.h"

static t_class *pdclass;
static t_class *maxclass;

/* These "glob" routines, which implement messages to Pd, are from all
over.  Some others are prototyped in m_imp.h as well. */

void glob_setfilename(void *dummy, t_symbol *name, t_symbol *dir);
void glob_quit(void *dummy);
void glob_dsp(void *dummy, t_symbol *s, int argc, t_atom *argv);
void glob_meters(void *dummy, t_floatarg f);
void glob_key(void *dummy, t_symbol *s, int ac, t_atom *av);
void glob_audiostatus(void *dummy);
void glob_finderror(t_pd *dummy);
void glob_ping(t_pd *dummy);

#if 0	    /* this doesn't work for OSS or for ALSA...  discouraged... */

#ifdef UNIX
#include <unistd.h>
#endif

void glob_restart_audio(t_pd *dummy)
{
    int inchans = sys_get_inchannels();
    int outchans = sys_get_outchannels();
    post("1");
    sys_close_audio();
    post("2");
#ifdef UNIX
    sleep(2);
#endif
    post("3");
    sys_open_audio(1, 0, 1, 0, /* IOhannes */
		   1, &inchans, 1, &outchans, sys_dacsr);
     post("4");
}
#endif

void alsa_resync( void);


#ifdef ALSA01
void glob_restart_alsa(t_pd *dummy)
{
    alsa_resync();
}
#endif

#ifdef NT
void glob_audio(void *dummy, t_floatarg adc, t_floatarg dac);
#endif

/* a method you add for debugging printout */
void glob_foo(void *dummy, t_symbol *s, int argc, t_atom *argv);

#if 0
void glob_foo(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    *(int *)1 = 3;
}
#endif

void max_default(t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    char str[80];
    startpost("%s: unknown message %s ", class_getname(pd_class(x)),
    	s->s_name);
    for (i = 0; i < argc; i++)
    {
    	atom_string(argv+i, str, 80);
    	poststring(str);
    }
    endpost();
}

void glob_init(void)
{
    maxclass = class_new(gensym("max"), 0, 0, sizeof(t_pd),
    	CLASS_DEFAULT, A_NULL);
    class_addanything(maxclass, max_default);
    pd_bind(&maxclass, gensym("max"));

    pdclass = class_new(gensym("pd"), 0, 0, sizeof(t_pd),
    	CLASS_DEFAULT, A_NULL);
    class_addmethod(pdclass, (t_method)glob_initfromgui, gensym("init"),
    	A_GIMME, 0);
    class_addmethod(pdclass, (t_method)glob_setfilename, gensym("filename"),
    	A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(pdclass, (t_method)glob_evalfile, gensym("open"),
    	A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(pdclass, (t_method)glob_quit, gensym("quit"), 0);
    class_addmethod(pdclass, (t_method)glob_foo, gensym("foo"), A_GIMME, 0);
    class_addmethod(pdclass, (t_method)glob_dsp, gensym("dsp"), A_GIMME, 0);
    class_addmethod(pdclass, (t_method)glob_meters, gensym("meters"),
    	A_FLOAT, 0);
    class_addmethod(pdclass, (t_method)glob_key, gensym("key"), A_GIMME, 0);
    class_addmethod(pdclass, (t_method)glob_audiostatus,
    	gensym("audiostatus"), 0);
    class_addmethod(pdclass, (t_method)glob_finderror,
    	gensym("finderror"), 0);
#ifdef UNIX
    class_addmethod(pdclass, (t_method)glob_ping, gensym("ping"), 0);
#endif
#ifdef NT
    class_addmethod(pdclass, (t_method)glob_audio, gensym("audio"),
    	A_DEFFLOAT, A_DEFFLOAT, 0);
#endif
#ifdef ALSA01
    class_addmethod(pdclass, (t_method)glob_restart_alsa,
    	gensym("restart-audio"), 0);
#endif
    class_addanything(pdclass, max_default);
    pd_bind(&pdclass, gensym("pd"));
}
