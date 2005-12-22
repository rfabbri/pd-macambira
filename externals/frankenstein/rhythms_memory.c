/* 
just a dummy rhythms_memory patch

*/

#include "m_pd.h"

#include "common.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>

static t_class *rhythms_memory_class;

typedef struct event event;
typedef struct event
{
	unsigned short int voice;
	double when;
	event *next;
};

typedef struct _rhythms_memory
{
    t_object x_obj; // myself
	t_outlet *l_out;
	t_rhythm_event *curr_seq;
	int seq_initialized;
	t_rhythm_memory_representation *rhythms_memory;
	double measure_start_time;
	double measure_length;
	event *events;
	
} t_rhythms_memory;

void rhythms_memory_free(t_rhythms_memory *x)
{
	if (x->curr_seq)
		freeBeats(x->curr_seq);	
	if (x->rhythms_memory)
		rhythm_memory_free(x->rhythms_memory);
}

void start_measure(t_rhythms_memory *x)
{
	// I call the pd functions to get a representation
	// of this very moment
	x->measure_start_time = clock_getlogicaltime();
}

void add_event(t_rhythms_memory *x, unsigned short int voice)
{
	event *newEvent, *currEvent, *lastEvent;
	double when;
	when = clock_gettimesince(x->measure_start_time);
	newEvent = (event *) malloc(sizeof(event));
	newEvent->when = when;
	newEvent->voice = voice;
	newEvent->next = 0;
	currEvent = x->events;
	if (currEvent)
	{
		// this is not the first event
		while(currEvent)
		{
			lastEvent = currEvent;
			currEvent = currEvent->next;
		}
		lastEvent->next = newEvent;
	} else
	{
		// this is the first event
		x->events = newEvent;
	}
	post("event added");
}

void end_measure(t_rhythms_memory *x)
{
	float fduration;
	event *currEvent, *lastEvent;
	unsigned short int is_it_a_new_rhythm;
	unsigned short int id, subid;
	float root_closeness, sub_closeness;
	int counter;
	t_atom *lista;
	// I call the pd functions to get a representation
	// of this very moment
	x->measure_length = clock_gettimesince(x->measure_start_time);
	// now that i know the exact length of the current measure
	// i can process the rhythm in the current measure
	// and evaluate it
	currEvent = x->events;
	// this is not the first event
	// now I translate events in rhythm
	counter = 0;
	while(currEvent)
	{
		fduration = (float) (((float) currEvent->when) / ((float) x->measure_length));
		if (x->seq_initialized)
		{
			concatenateBeat(x->curr_seq, currEvent->voice, fduration, 1);
		} else
		{
			setFirstBeat(&(x->curr_seq), currEvent->voice, fduration, 1);
			x->seq_initialized = 1;
		}
		currEvent = currEvent->next;
		counter++;
	}
	
	// delete events after having evaluated them
	currEvent = x->events;
	// this is not the first event
	while(currEvent)
	{
		lastEvent = currEvent;
		currEvent = currEvent->next;
		free(lastEvent);
	}
	x->events = 0;

	if (x->curr_seq)
	{
		// now I evaluate this rhythm with the memory
		rhythm_memory_evaluate(x->rhythms_memory, x->curr_seq, &is_it_a_new_rhythm,
								&id, &subid, &root_closeness, &sub_closeness);
		// tell out the answer
		// allocate space for the list
		lista = (t_atom *) malloc(sizeof(t_atom) * 5);
		SETFLOAT(lista, (float) is_it_a_new_rhythm);
		SETFLOAT(lista+1, (float) id);
		SETFLOAT(lista+2, (float) subid);
		SETFLOAT(lista+3, (float) root_closeness);
		SETFLOAT(lista+4, (float) sub_closeness);
		outlet_anything(x->l_out,
						gensym("list") ,
						5, 
						lista);
		free(lista);
		// rhythm_memory_evaluate freed the memory for the rhythm if needed
		x->seq_initialized = 0;
		x->curr_seq = 0;
	}
	// also start the new measure!
	start_measure(x);

	
}

static void rhythms_memory_bang(t_rhythms_memory *x) {

	// generate a random value
	float rnd;
	t_rhythm_event *events;
	t_duration dur;

	rnd = rand()/((double)RAND_MAX + 1);
	dur = float2duration(rnd);

	post("random value=%f duration.numerator=%i duration.denominator=%i", rnd, dur.numerator, dur.denominator);

	if (x->seq_initialized)
	{
		concatenateBeat(x->curr_seq, 0, rnd, 1);
	} else
	{
		setFirstBeat(&(x->curr_seq), 0, rnd, 1);
		x->seq_initialized = 1;
	}

	// print the sequence
	events = x->curr_seq;
	while(events)
	{
		post("event: numerator=%i, denominator=%i", events->duration.numerator, events->duration.denominator);
		events=events->next;
	}
	
}

void *rhythms_memory_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;
	time_t a;
    t_rhythms_memory *x = (t_rhythms_memory *)pd_new(rhythms_memory_class);
	//x->l_out = outlet_new(&x->x_obj, &s_list);
	x->l_out = outlet_new(&x->x_obj, "symbol");
	
	x->seq_initialized = 0;

	rhythm_memory_create(&(x->rhythms_memory));
	start_measure(x);

    return (x);
}

void rhythms_memory_setup(void)
{
    rhythms_memory_class = class_new(gensym("rhythms_memory"), (t_newmethod)rhythms_memory_new,
        (t_method)rhythms_memory_free, sizeof(t_rhythms_memory), CLASS_DEFAULT, A_GIMME, 0);
    class_addbang(rhythms_memory_class, (t_method)rhythms_memory_bang);
	class_addmethod(rhythms_memory_class, (t_method)end_measure, gensym("measure"), 0);
	class_doaddfloat(rhythms_memory_class, (t_method)add_event);

}
