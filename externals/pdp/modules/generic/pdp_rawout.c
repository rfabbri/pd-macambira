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
#include <errno.h>
#include <signal.h>
#include "pdp_pd.h"
#include "pdp_debug.h"
#include "pdp_list.h"
#include "pdp_comm.h"
#include "pdp_post.h"
#include "pdp_packet.h"


#define D if (1)
#define MAX_QUEUESIZE 4
#define PIPE_BLOCKSIZE 4096




/* raw input from a unix pipe */

typedef struct rawout_struct
{
    /* pd */
    t_object x_obj;
    //t_outlet *x_outlet;
    t_outlet *x_sync_outlet;

    /* comm */
    t_pdp_list *x_queue; // packet queue

    /* thread */
    pthread_mutex_t x_mut;
    pthread_attr_t x_attr;
    pthread_t x_thread;

    /* sync */
    int x_giveup;  // 1-> terminate writer thread
    int x_active;  // 1-> writer thread is launched
    int x_done;    // 1-> writer thread has exited

    /* config */
    t_symbol *x_pipe;
    t_pdp_symbol *x_type;

} t_rawout;


static inline void lock(t_rawout *x){pthread_mutex_lock(&x->x_mut);}
static inline void unlock(t_rawout *x){pthread_mutex_unlock(&x->x_mut);}

static void rawout_close(t_rawout *x);
static void pdp_in(t_rawout *x, t_symbol *s, t_float f)
{
    /* save packet to pdp queue, if size is smaller than maxsize */
    if (s == S_REGISTER_RO){
	if (x->x_queue->elements < MAX_QUEUESIZE){
	    int p = (int)f;
	    p = pdp_packet_copy_ro(p);
	    if (p != -1){
		lock(x);
		pdp_list_add_back(x->x_queue, a_packet, (t_pdp_word)p);
		unlock(x);
	    }
	}
	else {
	    pdp_post("pdp_rawout: dropping packet: (queue full)", MAX_QUEUESIZE);
	}
	    
    }

    /* check if thread is done */
    if (x->x_done) rawout_close(x);

}



static void *rawout_thread(void *y)
{
    int pipe;
    int packet = -1;
    t_rawout *x = (t_rawout *)y;
    int period_sec;
    int period_usec;
    sigset_t sigvec; /* signal handling  */

    /* ignore pipe signal */
    sigemptyset(&sigvec);
    sigaddset(&sigvec,SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &sigvec, 0);

    //D pdp_post("pipe: %s", x->x_pipe->s_name);
    //D pdp_post("type: %s", x->x_type->s_name);

    /* open pipe */
    if (-1 == (pipe = open(x->x_pipe->s_name, O_WRONLY|O_NONBLOCK))){
	perror(x->x_pipe->s_name);
	goto exit;
    }

    /* main loop (packets) */
    while(1){
	void *data = 0;
	int left = -1;

	/* try again if queue is empty */
	if (!x->x_queue->elements){
	    /* check if we need to stop */
	    if (x->x_giveup){
		goto close;
	    }
	    else {
		usleep(1000.0f); // sleep before polling again
		continue;
	    }
	}
	/* get packet from queue */
	lock(x);
	packet = pdp_list_pop(x->x_queue).w_packet;
	unlock(x);
	
	/* send packet */
	data = pdp_packet_data(packet);
	left = pdp_packet_data_size(packet);

	/* inner loop: pipe reads */
	while(left){

	    fd_set outset;
	    struct timeval tv = {0,10000};

	    /* check if we need to stop */
	    if (x->x_giveup){
		pdp_packet_mark_unused(packet);
		goto close;
	    }

	    /* select, with timeout */
	    FD_ZERO(&outset);
	    FD_SET(pipe, &outset);
	    if (-1 == select(pipe+1, NULL, &outset, NULL, &tv)){
		pdp_post("select error");
		goto close;
	    }

	    /* if ready, read, else retry */
	    if (FD_ISSET(pipe, &outset)){
		int bytes = write(pipe, data, left);
		/* handle errors */
		if (bytes <= 0){
		    perror(x->x_pipe->s_name);
		    if (bytes != EAGAIN) goto close;
		}
		/* or update pointers */
		else{
		    data += bytes;
		    left -= bytes;
		    //pdp_post("left %d", left);
		}
	    }
	    else {
		//pdp_post("retrying write");
	    }
	}
		   
	/* discard packet */
	pdp_packet_mark_unused(packet);

    
    }

  close:
    /* close pipe */
    close(pipe);
	
	
  exit:
    x->x_done = 1;
    return 0;
}



static void rawout_type(t_rawout *x, t_symbol *type)
{
    x->x_type = pdp_gensym(type->s_name);
}

static void rawout_open(t_rawout *x, t_symbol *pipe)
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
    pthread_create(&x->x_thread, &x->x_attr, rawout_thread , x);
    x->x_active = 1;
}

static void rawout_close(t_rawout *x)
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

static void rawout_free(t_rawout *x)
{
    rawout_close(x);
    pdp_tree_strip_packets(x->x_queue);
    pdp_tree_free(x->x_queue);
}

t_class *rawout_class;


static void *rawout_new(t_symbol *pipe, t_symbol *type)
{
    t_rawout *x;

    pdp_post("%s %s", pipe->s_name, type->s_name);

    /* allocate & init */
    x = (t_rawout *)pd_new(rawout_class);
    //x->x_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_sync_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_queue = pdp_list_new(0);
    x->x_active = 0;
    x->x_giveup = 0;
    x->x_done = 0;
    x->x_type = pdp_gensym("image/YCrCb/320x240"); //default
    x->x_pipe = gensym("/tmp/pdpraw"); // default
    pthread_attr_init(&x->x_attr);
    pthread_mutex_init(&x->x_mut, NULL);

    /* args */
    rawout_type(x, type);
    if (pipe->s_name[0]) x->x_pipe = pipe; 

    return (void *)x;

}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_rawout_setup(void)
{
    int i;

    /* create a standard pd class: [pdp_rawout pipe type] */
    rawout_class = class_new(gensym("pdp_rawout"), (t_newmethod)rawout_new,
   	(t_method)rawout_free, sizeof(t_rawout), 0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);

    /* add global message handler */
    class_addmethod(rawout_class, (t_method)pdp_in, 
		    gensym("pdp"), A_SYMBOL, A_FLOAT, A_NULL);

    class_addmethod(rawout_class, (t_method)rawout_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(rawout_class, (t_method)rawout_open, gensym("open"), A_DEFSYMBOL, A_NULL);
    class_addmethod(rawout_class, (t_method)rawout_close, gensym("close"), A_NULL);


}

#ifdef __cplusplus
}
#endif
