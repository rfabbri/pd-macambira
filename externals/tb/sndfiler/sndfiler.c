/* 
 * 
 * soundfiler based on libsndfile
 * Copyright (C) 2005  Tim Blechmann
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */



#include "stdlib.h"
#include "m_pd.h"
#include "g_canvas.h"
#include "m_fifo.h"
#include "pthread.h" /* for helper thread */
#include "sched.h" /* for thread priority */
#include "alloca.h"

#include "sndfile.h"

#if (_POSIX_MEMLOCK - 0) >=  200112L
#include <sys/mman.h>
#endif /* _POSIX_MEMLOCK */

static t_class *sndfiler_class;


typedef struct _sndfiler
{
    t_object x_obj;
    t_canvas *x_canvas;
} t_sndfiler;

typedef struct _sfprocess
{
	void* padding;
	void (* process) (t_sndfiler *, 
		int, t_atom *); /* callback function */
    t_sndfiler * x;   /* soundfiler */
    int argc;  
    t_atom * argv;
} t_sfprocess;


/* this is the queue for all soundfiler objects */
typedef struct _sfqueue
{
	t_fifo* x_jobs;
	
	pthread_mutex_t mutex;
    pthread_cond_t cond;
} t_sfqueue;


typedef struct _syncdata
{
	t_garray** arrays;
	t_float** helper_arrays;
	int argc;
	t_int frames;
} t_syncdata;


static t_sfqueue sndfiler_queue; 
static pthread_t sf_thread_id; /* id of soundfiler thread */


static t_sndfiler *sndfiler_new(void)
{
    t_sndfiler *x = (t_sndfiler *)pd_new(sndfiler_class);
    x->x_canvas = canvas_getcurrent();
    outlet_new(&x->x_obj, &s_float);
    return (x);
}


/* global soundfiler thread ... sleeping until signaled */
static void sndfiler_thread(void)
{
	while (1)
    {
		t_sfprocess * me;
		pthread_cond_wait(&sndfiler_queue.cond, &sndfiler_queue.mutex);
		
		while (me = (t_sfprocess *)fifo_get(sndfiler_queue.x_jobs))
		{
			(me->process)(me->x, me->argc, me->argv);

			/* freeing the argument vector */
			freebytes(me->argv, sizeof(t_atom) * me->argc);
			freebytes(me, sizeof(t_sfprocess));
		}
	}
}

static void sndfiler_start_thread(void)
{
	pthread_attr_t sf_attr;
    struct sched_param sf_param;
	int status;

	//initialize queue
	sndfiler_queue.x_jobs = fifo_init();
    pthread_mutex_init (&sndfiler_queue.mutex,NULL);
    pthread_cond_init (&sndfiler_queue.cond,NULL);
	
    // initialize thread
    pthread_attr_init(&sf_attr);
    
    sf_param.sched_priority=sched_get_priority_min(SCHED_OTHER);
    pthread_attr_setschedparam(&sf_attr,&sf_param);
	
#ifdef UNIX
	if (sys_hipriority == 1/*  && getuid() == 0 */)
	{
		sf_param.sched_priority=sched_get_priority_min(SCHED_RR);
		pthread_attr_setschedpolicy(&sf_attr,SCHED_RR);
	}
#endif /* UNIX */
	
	//start thread
	status = pthread_create(&sf_thread_id, &sf_attr, 
		(void *) sndfiler_thread,NULL);
    if (status != 0)
		error("Couldn't create sndfiler thread: %d",status);
    else
		post("global sndfiler thread launched, priority: %d", 
			sf_param.sched_priority);
}


static void sndfiler_read_cb(t_sndfiler * x, int argc, t_atom* argv);


/* syntax:
 * read soundfile array0..n
 * if the soundfile has less channels than arrays are given, these arrays are
 * set to zero
 * if there are too little arrays given, only the first n channels will be used
 * */

static void sndfiler_read(t_sndfiler * x, t_symbol *s, int argc, t_atom* argv)
{
	t_sfprocess * process = getbytes(sizeof(t_sfprocess));

	
	process->process = &sndfiler_read_cb;
	process->x = x;
	process->argc = argc;
	process->argv = (t_atom*) copybytes(argv, sizeof(t_atom) * argc);

	fifo_put(sndfiler_queue.x_jobs, process);
	
	pthread_cond_signal(&sndfiler_queue.cond);
}

static t_int sndfiler_synchonize(t_int * w);

static void sndfiler_read_cb(t_sndfiler * x, int argc, t_atom* argv)
{
	int resize = 0;
	int frames = -1;
	int i, j;
	int counter=0;
	int channel_count;
	t_float** helper_arrays;

	t_symbol* file;
	t_garray ** arrays;
	
	SNDFILE* sndfile;
	SF_INFO info;

	if (argc < 2)
	{
		pd_error(x, "Invalid arguments");
		return;
	}
	
	file = atom_getsymbolarg(0, argc, argv);

	channel_count = argc - 1;
	helper_arrays = getbytes(channel_count * sizeof(t_float*));

	arrays = getbytes(channel_count * sizeof(t_garray*));
	for (i = 0; i != channel_count; ++i)
	{
		t_garray * array = (t_garray *)pd_findbyclass(atom_getsymbolarg(i+1, argc, argv), garray_class);
		
		if (array)
			arrays[i] = array;
		else
		{
			pd_error(x, "%s: no such array", atom_getsymbolarg(i+1, argc, argv)->s_name);
			return;
		}
	}

	sndfile = sf_open(file->s_name, SFM_READ, &info);

	if (sndfile)
	{
		int minchannels = (channel_count < info.channels) ?
			channel_count : info.channels;

		int maxchannels = (channel_count < info.channels) ?
			channel_count : info.channels;

		t_float * item = alloca(maxchannels * sizeof(t_float));

		t_int ** syncdata = getbytes(sizeof(t_int*) * 5);
		
#if (_POSIX_MEMLOCK - 0) >=  200112L
		munlockall();
#endif

		for (i = 0; i != channel_count; ++i)
		{
			helper_arrays[i] = getalignedbytes(info.frames * sizeof(t_float));
		}
	

		for (i = 0; i != info.frames; ++i)
		{
			sf_read_float(sndfile, item, 1);

			for (j = 0; j != channel_count; ++j)
			{
				if (j < minchannels)
					helper_arrays[j][i] = item[j];
				else
					helper_arrays[j][i] = 0;
			}
		}
#if (_POSIX_MEMLOCK - 0) >=  200112L
		mlockall(MCL_FUTURE);
#endif

		sf_close(sndfile);
		
		syncdata[0] = (t_int*)arrays;
		syncdata[1] = (t_int*)helper_arrays;
		syncdata[2] = (t_int*)channel_count;
		syncdata[3] = (t_int*)(long)info.frames;
		syncdata[4] = (t_int*)x;

		sys_callback(sndfiler_synchonize, (t_int*)syncdata, 5);
	}
	else
	{
		pd_error(x, "Error opening file");
	}
}


static t_int sndfiler_synchonize(t_int * w)
{
	int i;
	t_garray** arrays = (t_garray**) w[0];
	t_float** helper_arrays = (t_float**) w[1];
	t_int channel_count = (t_int)w[2];
	t_int frames = (t_int)w[3];
	t_sndfiler* x = (t_sndfiler*)w[4];
	
	for (i = 0; i != channel_count; ++i)
	{
		t_garray * garray = arrays[i];
		t_array * array = garray->x_scalar->sc_vec[0].w_array;
		t_glist * gl = garray->x_glist;;

		
		freealignedbytes(array->a_vec, array->a_n);
		array->a_vec = (char*)helper_arrays[i];
		array->a_n = frames;

		if (gl->gl_list == &garray->x_gobj && !garray->x_gobj.g_next)
		{
			vmess(&gl->gl_pd, gensym("bounds"), "ffff",
				0., gl->gl_y1, (double)(frames > 1 ? frames-1 : 1), gl->gl_y2);
			/* close any dialogs that might have the wrong info now... */
			gfxstub_deleteforkey(gl);
		}
		else 
			garray_redraw(garray);
	}

	free(arrays);
	free(helper_arrays);

	canvas_update_dsp();

	outlet_float(x->x_obj.ob_outlet, frames);

	return 0;
}

	
void sndfiler_setup(void)
{
	sndfiler_class = class_new(gensym("sndfiler"), (t_newmethod)sndfiler_new, 
		0, sizeof(t_sndfiler), 0, 0);
    class_addmethod(sndfiler_class, (t_method)sndfiler_read,
		gensym("read"), A_GIMME, 0);
	
	sndfiler_start_thread();
}


