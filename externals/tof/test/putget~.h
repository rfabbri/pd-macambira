#include "m_pd.h"

/*
 typedef struct _param_inlet2
{
  t_object        x_obj;
  t_param  *p_owner;
} t_param_inlet2;

*/ 

struct putget {
	t_symbol*			name;
	t_sample 			a[64]; 
	t_sample 			b[64];
	t_sample*		w;
	t_sample* 		r; 
	int				writers;
	//int				written;
	int 			users;
	struct putget* 	next; 
    struct putget* 	previous; 
	t_clock*		clock;
	int 			armed;
};


struct putget* PUTGETS;

// This should be triggered by the clock
static void putget_swap(struct putget* pg) {
	//post("clock");
	
	t_sample* temp = pg->r;
	pg->r = pg->w;
	pg->w = temp;
	
	int i;
	t_sample* samples = pg->w;
	for (i=0;i<64;i++) {
		*samples++ = 0;
		
	}
	
	pg->armed = 0;
	
	//if (pg == NULL) post("ouc");
	//pg->written = 0;
	
}

static struct putget* putget_register(t_symbol* name, int is_writer) {
  
  struct putget* new_putget;
  
  is_writer = is_writer ? 1 : 0;

  struct putget* pg = PUTGETS;
  
  if ( pg != NULL) {
		
	// Search for previous putget
	while( pg ) {
			if ( pg->name == name) {
				//#ifdef PARAMDEBUG
				 // post("Found put/get with same name");
				//#endif
				pg->writers = pg->writers + is_writer;
				pg->users = pg->users + 1;
				return pg;
			}
			if ( pg->next == NULL ) break;
			pg = pg->next; 
		}
	}
	
	
	//post("Appending new put/get");
	// Append new putget
    new_putget = getbytes(sizeof(*new_putget));
	new_putget->name = name;
	new_putget->writers = is_writer;
	new_putget->users = 1; 
    new_putget->armed = 0;
	
	new_putget->clock = clock_new(new_putget, (t_method)putget_swap);
	
	new_putget->r = new_putget->a;
	new_putget->w = new_putget->b;
	
	new_putget->previous = pg;
	if ( pg) {
		pg->next = new_putget;
	} else {
		PUTGETS = new_putget;
	}
	
	return new_putget;
	
}


static void putget_unregister(struct putget* pg, int is_writer) {
	
	//post("Trying to remove %s",pg->name->s_name);
	
	if ( is_writer) pg->writers = pg->writers - 1;
	pg->users = pg->users - 1;
	if ( pg->users <= 0) {
		//post("Removing last put/get of this name");
		if (pg->previous) {
				pg->previous->next = pg->next;
				if (pg->next) pg->next->previous = pg->previous;
			} else {
				PUTGETS = pg->next;
				if ( pg->next != NULL) pg->next->previous = NULL;
			}
		clock_free(pg->clock);
		freebytes(pg, sizeof *(pg) );
	}
}


static void putget_arm(struct putget* pg) {
	
	if (!pg->armed) {
		pg->armed = 1;
		clock_delay(pg->clock, 0);
	}
	
}


