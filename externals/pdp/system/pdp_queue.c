/*
 *   Pure Data Packet - processor queue module.
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



/* 
   this is a the processor queue pdp system module 
   it receives tasks from objects that are schedules to 
   be computed in another thread. the object is signalled back
   when the task is completed.

   this is not a standard pd class. it is a sigleton class
   using a standard pd clock to poll for compleded methods on 
   every scheduler run. this is a hack to do thread synchronization 
   in a thread unsafe pd.

 */

#include "pdp.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C"
{
#endif

#define PDP_QUEUE_SIZE 1024
#define PDP_QUEUE_DELTIME 1.0f;




/********************* pdp process queue data *********************/

typedef void (*t_pdpmethod)(void *client);

/* the process queue data record */
typedef struct process_queue_struct
{
  void *x_owner;             /* the object we are dealing with */
  t_pdpmethod x_process;     /* the process method */
  t_pdpmethod x_callback;    /* the function to be called when finished */
  int *x_queue_id;           /* place to store the queue id for task */
} t_process_queue;



/* clock members */
static t_clock *pdp_clock;
static double deltime;

/* some bookkeeping vars */
static long long ticks;
static long long packets;

/* queue members */
static t_process_queue *q;    /* queue */
static int mask;
static int head;              /* last entry in queue + 1 */
static int tail;              /* first entry in queque */
static int curr;              /* the object currently processed in other thread */

/* pthread vars */
static pthread_mutex_t mut;
static pthread_cond_t cond_dataready;
static pthread_cond_t cond_processingdone;
static pthread_t thread_id;
    
/* synchro pipes */
static int pipe_fd[2];

/* toggle for thread usage */
static int use_thread;



/* the methods */
void pdp_queue_wait()
{
    //post("pdp_pq_wait: waiting for pdp_queue_thread to finish processing");
    pthread_mutex_lock(&mut);
    while(((curr - head) & mask) != 0){

	  pthread_cond_wait(&cond_processingdone, &mut);
    }
    pthread_mutex_unlock(&mut);
    //post("pdp_pq_wait: pdp_queue_thread has finished processing");

}
void pdp_queue_finish(int index)
{

  if (-1 == index) {
      //post("pdp_pq_remove: index == -1");
      return;
  }
  /* wait for processing thread to finish*/
  pdp_queue_wait();

  /* invalidate callback at index */
  q[index & mask].x_callback = 0;
  q[index & mask].x_queue_id = 0;

}

static void pdp_queue_signal_processor(void)
{

    pthread_mutex_lock(&mut);
    //post("signalling process thread");
    pthread_cond_signal(&cond_dataready);
    pthread_mutex_unlock(&mut);
    //post("signalling process thread done");

}

static void pdp_queue_wait_for_feeder(void)
{


    /* only use locking when there is no data */
    if(((curr - head) & mask) == 0){
	pthread_mutex_lock(&mut);

	/* signal processing done */
	//post("pdp_queue_thread: signalling processing is done");
	pthread_cond_signal(&cond_processingdone);

	/* wait until there is an item in the queue */
	while(((curr - head) & mask) == 0){
	    //post("waiting for feeder");
	    pthread_cond_wait(&cond_dataready, &mut);
	    //post("waiting for feeder done");
	}

	pthread_mutex_unlock(&mut);

    }
}

void pdp_queue_add(void *owner, void *process, void *callback, int *queue_id)
{
    int i;

    /* if processing is in not in thread, just call the funcs */
    if (!use_thread){
	//post("pdp_queue_add: calling processing routine directly");
	*queue_id = -1;
	((t_pdpmethod) process)(owner);
	((t_pdpmethod) callback)(owner);
	return;
    }
	


    /* schedule method in thread queue */
    if (1 == ((tail - head) & mask)) {
	post("pdp_queue_add: WARNING: processing queue is full.\n");
	post("pdp_queue_add: WARNING: skipping process method, calling callback directly.\n");
	*queue_id = -1;
	((t_pdpmethod) callback)(owner);
    }



    i = head & mask;
    q[i].x_owner = owner;
    q[i].x_process = process;
    q[i].x_callback = callback;
    q[i].x_queue_id = queue_id;
    *queue_id = i;
    //post("pdp_queue_add: added method to queue, index %d", i);

      
    // increase the packet count
    packets++;
  
    // move head forward
    head++;

    pdp_queue_signal_processor();

}


/* processing thread */
static void *pdp_queue_thread(void *dummy)
{
  while(1){


      /* wait until there is data available */
      pdp_queue_wait_for_feeder();      


      //post("pdp_queue_thread: processing %d", curr);


      /* call the process routine */
      (q[curr & mask].x_process)(q[curr & mask].x_owner);

      /* advance */
      curr++;


    }
}


/* call back all the callbacks */
static void pdp_queue_callback (void)
{

  /* call callbacks for finished packets */
  while(0 != ((curr - tail) & mask))
    {
      int i = tail & mask;
      /* invalidate queue id */
      if(q[i].x_queue_id) *q[i].x_queue_id = -1;
      /* call callback */
      if(q[i].x_callback) (q[i].x_callback)(q[i].x_owner);
      //else post("pdp_pq_tick: callback %d is disabled",i );
      tail++;
    }

}

/* the clock method */
static void pdp_queue_tick (void)
{
  /* do work */
  //if (!(ticks % 1000)) post("pdp tick %d", ticks);

  if (!use_thread) return;

  /* call callbacks */
  pdp_queue_callback();

  /* increase counter */
  ticks++;

  /* set clock for next update */
  clock_delay(pdp_clock, deltime);
}


void pdp_queue_use_thread(int t)
{
    /* if thread usage is being disabled, 
       wait for thread to finish processing first */
    if (t == 0) {
	pdp_queue_wait();
	use_thread = 0;
	pdp_queue_callback();
	clock_unset(pdp_clock);
    }
    else {
	clock_unset(pdp_clock);
	clock_delay(pdp_clock, deltime);
	use_thread = 1;
    }

}

void pdp_queue_setup(void)
{
  pthread_attr_t attr;

  /* setup pdp queue processor object */
  ticks = 0;
  deltime = PDP_QUEUE_DELTIME;

  /* setup queue data */
  mask = PDP_QUEUE_SIZE - 1;
  head = 0;
  tail = 0;
  curr = 0;
  q = getbytes(PDP_QUEUE_SIZE * sizeof(*q));

  /* enable threads */
  use_thread = 1;

  /* setup synchro stuff */
  pthread_mutex_init(&mut, NULL);
  pthread_cond_init(&cond_dataready, NULL);
  pthread_cond_init(&cond_processingdone, NULL);

 
  /* allocate the clock */
  pdp_clock = clock_new(0, (t_method)pdp_queue_tick);

  /* set the clock */
  clock_delay(pdp_clock, 0);

  /* start processing thread */

  /* glibc doc says SCHED_OTHER is default,
     but it seems not to be when initiated from a RT thread
     so we explicitly set it here */
  pthread_attr_init (&attr);
  //pthread_attr_setschedpolicy(&attr, SCHED_FIFO); 
  pthread_attr_setschedpolicy(&attr, SCHED_OTHER); 
  pthread_create(&thread_id, &attr, pdp_queue_thread, (void *)0);



  /* set default disable/enable thread here */
  pdp_queue_use_thread(0);

}







#ifdef __cplusplus
}
#endif
