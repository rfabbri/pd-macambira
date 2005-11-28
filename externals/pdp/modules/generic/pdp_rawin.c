/*
 *   Pure Data Packet module. packet forth console
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include "pdp_pd.h"
#include "pdp_debug.h"
#include "pdp_list.h"
#include "pdp_comm.h"
#include "pdp_post.h"
#include "pdp_packet.h"


#define PERIOD 1.0f
#define D if (1)




/* raw input from a unix pipe */

typedef struct rawin_struct
{
    /* pd */
    t_object x_obj;
    t_outlet *x_outlet;
    t_outlet *x_sync_outlet;
    t_clock *x_clock;

    /* comm */
    t_pdp_list *x_queue; // packet queue

    /* thread */
    pthread_mutex_t x_mut;
    pthread_attr_t x_attr;
    pthread_t x_thread;

    /* sync */
    int x_giveup;  // 1-> terminate reader thread
    int x_active;  // 1-> reader thread is launched
    int x_done;    // 1-> reader thread has exited

    /* config */
    t_symbol *x_pipe;
    t_pdp_symbol *x_type;

} t_rawin;


static inline void lock(t_rawin *x){pthread_mutex_lock(&x->x_mut);}
static inline void unlock(t_rawin *x){pthread_mutex_unlock(&x->x_mut);}

static void rawin_close(t_rawin *x);
static void tick(t_rawin *x)
{
    /* send all packets in queue to outlet */
    lock(x);
    while (x->x_queue->elements){
	outlet_pdp_atom(x->x_outlet, x->x_queue->first);
	pdp_list_pop(x->x_queue); // pop stale reference
    }
    unlock(x);
    clock_delay(x->x_clock, PERIOD);

    /* check if thread is done */
    if (x->x_done) rawin_close(x);

}

static void move_current_to_queue(t_rawin *x, int packet)
{
    lock(x);
    pdp_list_add_back(x->x_queue, a_packet, (t_pdp_word)packet);
    unlock(x);
}

static void *rawin_thread(void *y)
{
    int pipe;
    int packet = -1;
    t_rawin *x = (t_rawin *)y;
    int period_sec;
    int period_usec;


    //D pdp_post("pipe: %s", x->x_pipe->s_name);
    //D pdp_post("type: %s", x->x_type->s_name);

    /* open pipe */
    if (-1 == (pipe = open(x->x_pipe->s_name, O_RDONLY|O_NONBLOCK))){
	perror(x->x_pipe->s_name);
	goto exit;
    }

    /* main loop (packets) */
    while(1){
	void *data = 0;
	int left = -1;

	/* create packet */
	if (-1 != packet){
	    pdp_post("WARNING: deleting stale packet");
	    pdp_packet_mark_unused(packet);
	}
	packet = pdp_factory_newpacket(x->x_type);
	if (-1 == packet){
	    pdp_post("ERROR: can't create packet. type = %s", x->x_type->s_name);
	    goto exit;
	}
	
	/* fill packet */
	data = pdp_packet_data(packet);
	left = pdp_packet_data_size(packet);
	// D pdp_post("packet %d, data %x, size %d", packet, data, left);

	/* inner loop: pipe reads */
	while(left){

	    fd_set inset;
	    struct timeval tv = {0,10000};

	    /* check if we need to stop */
	    if (x->x_giveup){
		pdp_packet_mark_unused(packet);
		goto close;
	    }
	    /* select, with timeout */
	    FD_ZERO(&inset);
	    FD_SET(pipe, &inset);
	    if (-1 == select(pipe+1, &inset, NULL,NULL, &tv)){
		pdp_post("select error");
		goto close;
	    }

	    /* if ready, read, else retry */
	    if (FD_ISSET(pipe, &inset)){
		int bytes = read(pipe, data, left);
		if (!bytes){
		    /* if no bytes are read, pipe is closed */
		    goto close;
		}
		data += bytes;
		left -= bytes;
	    }
	}
		   
	/* move to queue */
	move_current_to_queue(x, packet);
	packet = -1;


    
    }

  close:
    /* close pipe */
    close(pipe);
	
	
  exit:
    x->x_done = 1;
    return 0;
}



static void rawin_type(t_rawin *x, t_symbol *type)
{
    x->x_type = pdp_gensym(type->s_name);
}

static void rawin_open(t_rawin *x, t_symbol *pipe)
{
    /* save pipe name if not empty */
    if (pipe->s_name[0]) {x->x_pipe = pipe;}

    if (x->x_active) {
	pdp_post("already open");
	return;
    }
    /* start thread */
    x->x_giveup = 0;
    x->x_done = 0;
    pthread_create(&x->x_thread, &x->x_attr, rawin_thread , x);
    x->x_active = 1;
}

static void rawin_close(t_rawin *x)
{

    if (!x->x_active) return;

    /* stop thread: set giveup + wait */
    x->x_giveup = 1;
    pthread_join(x->x_thread, NULL);
    x->x_active = 0;

    /* notify */
    outlet_bang(x->x_sync_outlet);
    pdp_post("connection to %s closed", x->x_pipe->s_name);

    


    
}

static void rawin_free(t_rawin *x)
{
    rawin_close(x);
    clock_free(x->x_clock);
    pdp_tree_strip_packets(x->x_queue);
    pdp_tree_free(x->x_queue);
}

t_class *rawin_class;


static void *rawin_new(t_symbol *pipe, t_symbol *type)
{
    t_rawin *x;

    pdp_post("%s %s", pipe->s_name, type->s_name);

    /* allocate & init */
    x = (t_rawin *)pd_new(rawin_class);
    x->x_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_sync_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_clock = clock_new(x, (t_method)tick);
    x->x_queue = pdp_list_new(0);
    x->x_active = 0;
    x->x_giveup = 0;
    x->x_done = 0;
    x->x_type = pdp_gensym("image/YCrCb/320x240"); //default
    x->x_pipe = gensym("/tmp/pdpraw"); // default
    pthread_attr_init(&x->x_attr);
    pthread_mutex_init(&x->x_mut, NULL);
    clock_delay(x->x_clock, PERIOD);

    /* args */
    rawin_type(x, type);
    if (pipe->s_name[0]) x->x_pipe = pipe; 

    return (void *)x;

}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_rawin_setup(void)
{
    int i;

    /* create a standard pd class: [pdp_rawin pipe type] */
    rawin_class = class_new(gensym("pdp_rawin"), (t_newmethod)rawin_new,
   	(t_method)rawin_free, sizeof(t_rawin), 0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);

    /* add global message handler */
    class_addmethod(rawin_class, (t_method)rawin_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(rawin_class, (t_method)rawin_open, gensym("open"), A_DEFSYMBOL, A_NULL);
    class_addmethod(rawin_class, (t_method)rawin_close, gensym("close"), A_NULL);


}

#ifdef __cplusplus
}
#endif
