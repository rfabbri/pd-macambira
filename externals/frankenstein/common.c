
#include "common.h"

#include <math.h>
#include <stdlib.h>

#include "m_pd.h" // to post errors and debug messages

t_duration int2duration(int n)
{
	t_duration dur;
	int curr, i, j, ok, currden;
	curr=0;
	ok=0;
	for (i=0; i<num_possible_denominators; i++)
	{
		currden = possible_denominators[i];
		for (j=0; j<currden; j++)
		{
			if (curr==n)
			{
				dur.numerator=j;
				dur.denominator=currden;
				j=currden;
				i=num_possible_denominators;
				ok=1;
			} else
				curr++;
		}
	}
	if (ok)
		return dur;
	else
	{
		dur.numerator=1;
		dur.denominator=1;
		return dur;
	}
}

unsigned short int duration2int(t_duration dur)
{
	unsigned short int curr, i, j;
	curr=0;
	for (i=0; i<num_possible_denominators; i++)
	{
		for (j=0; j<i; j++)
		{
			if ((dur.numerator==j) && (dur.denominator==i))
			{
				return curr;
			} else
				curr++;
		}
	}
	return curr+1;	
}

int possible_durations()
{
	int ris, i;
	ris = 0;
	for (i=0; i<num_possible_denominators; i++)
	{
		ris += possible_denominators[i]-1;
	}
	ris += 1;
	return ris;
}

t_duration float2duration(float fduration)
{
	float minDiff, currfduration, currDiff;
	int i, maxi;
	t_duration currDur, bestDur;

	bestDur.numerator=1;
	bestDur.denominator=1;
	minDiff = 1;
	maxi = possible_durations();
	for (i=0; i<maxi; i++)
	{
	//	post("i=%i", i);
		currDur = int2duration(i);
	//	post("currDur=%i/%i", currDur.numerator, currDur.denominator);
		currfduration = duration2float(currDur);
	//	post("currfduration=%f", currfduration);
		currDiff = (float) fabs(fduration - currfduration);
		if (currDiff<=minDiff)
		{
			minDiff = currDiff;
			bestDur = currDur;
		}
	}
	return bestDur;
}

float duration2float(t_duration duration)
{
	return (float) (((float)duration.numerator) / ((float)duration.denominator));
}

void setFirstBeat(t_rhythm_event **firstEvent, unsigned short int voice, float fstart, float fduration)
{
	t_duration res;
	t_rhythm_event *newElement;
	// convert from float to duration
	res = float2duration(fduration);
	// allocate a new element of the list
	newElement = malloc(sizeof(t_rhythm_event));
	// set the pointers
	newElement->previous = 0;
	newElement->next = 0;
	newElement->voice=voice;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;
	res = float2duration(fstart);
	newElement->start.numerator = res.numerator;
	newElement->start.denominator = res.denominator;
	*firstEvent = newElement;
}

void concatenateBeat(t_rhythm_event *currentEvent, unsigned short int voice, float fstart, float fduration)
{
	t_duration res;
	t_rhythm_event *newElement, *lastElement;
	lastElement = currentEvent;
	while(lastElement->next)
		lastElement = lastElement->next;
	// convert from float to duration
	res = float2duration(fduration);
	// allocate a new element of the list
	newElement = (t_rhythm_event *) malloc(sizeof(t_rhythm_event));
	// set the pointers
	newElement->previous = lastElement;
	newElement->next = 0;
	lastElement->next = newElement;
	newElement->voice=voice;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;
	res = float2duration(fstart);
	newElement->start.numerator = res.numerator;
	newElement->start.denominator = res.denominator;
}

void freeBeats(t_rhythm_event *currentEvent)
{
	t_rhythm_event *prev;
	t_rhythm_event *next;

	// go to the first element of the list
	while(currentEvent->previous)
		currentEvent = currentEvent->previous;

	// now free each element
	next=currentEvent->next;
	do
	{
		prev = currentEvent;
		next = currentEvent->next;
		free(currentEvent);
	} while(next);

}

void add_t_rhythm_memory_arc(t_rhythm_memory_node *srcNode, unsigned short int dstNode)
{
	t_rhythm_memory_arc *newArc;
	t_rhythm_memory_arc *lastArc;

	// create a new arc
	newArc = (t_rhythm_memory_arc *) malloc(sizeof(t_rhythm_memory_arc));
	newArc->to_node_index = dstNode;
	// go to the last arc in the list
	// and add this arc as the last
	lastArc = srcNode->arcs;
	if (lastArc)
	{
		// this is not the first arc
		while(lastArc->next_arc)
			lastArc = lastArc->next_arc;
		lastArc->next_arc = newArc;
	} else
	{
		// this is the first arc
		srcNode->arcs = newArc;
	}
}

// initialize this representation, allocates memory for the pointers
void create_rhythm_memory_representation(t_rhythm_memory_representation **this_rep, unsigned short int id)
{
	int i;
	// allocate space for the data structure
	*this_rep = (t_rhythm_memory_representation *)malloc(sizeof(t_rhythm_memory_representation));
	// space for transitions
	(*this_rep)->transitions = (t_rhythm_memory_node *) malloc(sizeof(t_rhythm_memory_node) * possible_durations());
	// initialize transitions
	for (i=0; i<possible_durations(); i++)
	{
		(*this_rep)->transitions[i].first=0;
		(*this_rep)->transitions[i].weight=0;
		(*this_rep)->transitions[i].arcs=0;
	}
	(*this_rep)->rhythms = 0;
	// the naming variables
	(*this_rep)->id = id; // the main id
	(*this_rep)->last_sub_id = 0; // the sub id
}

// add a new rhythm in the list of similar thythms related to one main rhythm
unsigned short int add_t_rhythm_memory_element(t_rhythm_memory_representation *this_rep, t_rhythm_event *new_rhythm)
{
	t_rhythm_memory_element *curr;
	t_rhythm_memory_element *newElement;
	t_rhythm_event *currEvent;
	t_rhythm_memory_arc *currArc, *newArc, *prevArc;
	unsigned short int last, sub_id;
	int i, arcFound;
	// creates a new element of the list of similar rhythms
	newElement = (t_rhythm_memory_element *) malloc(sizeof(t_rhythm_memory_element));
	newElement->rhythm = new_rhythm;
	newElement->next = 0;
	sub_id = this_rep->last_sub_id;
	newElement->id = sub_id;
	this_rep->last_sub_id++;
	// finds the last element and adds itself
	curr = this_rep->rhythms;
	if (curr)
	{
		while (curr->next)
			curr=curr->next;
		curr->next = newElement;

	} else
	{
		this_rep->rhythms = newElement;
	}
	// now update the transition table..
	currEvent = new_rhythm;
	// set the first event..
	i = duration2int(new_rhythm->start);
	this_rep->transitions[i].first++;
	last = 0;
	while (currEvent)
	{
		// get the duration and translate into an int
		i = duration2int(currEvent->start);
		if (last) // NB if last==0 then last is not set
		{
			// add an arc between last and i
			currArc = this_rep->transitions[last].arcs;
			arcFound=0;
			// is this arc rpesent?
			// also i need to get to the last element of the lsit
			while (currArc)
			{
				// for each arc in the list
				if (currArc->to_node_index == i)
				{
					// this arc is already present
					arcFound=1;
				}
				prevArc = currArc; // last valid arc
				currArc = currArc->next_arc;
			} 
			if (!arcFound)
			{
				// this arc was not present, add it!
				newArc = (t_rhythm_memory_arc *) malloc(sizeof(t_rhythm_memory_arc));
				newArc->next_arc = 0; 
				newArc->to_node_index = i; // set my destination
				if (this_rep->transitions[last].arcs)
				{
					// this is not the first arc
					// then prevArc is set and valid
					prevArc->next_arc = newArc;
				} else
				{
					// this is the first arc
					this_rep->transitions[last].arcs = newArc;
				}
			}
		}
		// increment the weight
		this_rep->transitions[i].weight++;
		if (this_rep->transitions[i].weight > this_rep->max_weight)
		{
			// a new champion!
			this_rep->max_weight = this_rep->transitions[i].weight;
		}
		last = i;
		currEvent = currEvent->next;
	}
	return sub_id;
}

// free the list of representations
void free_memory_representations(t_rhythm_memory_representation *this_rep)
{
	int i, maxi;
	t_rhythm_memory_representation *currRep, *oldRep;
	t_rhythm_memory_element *currElement, *tmpElement;
	t_rhythm_memory_arc *currArc, *tmpArc;
	currRep = this_rep;
	while(currRep)
	{
		// free the table
		maxi = possible_durations();
		for (i=0; i<maxi; i++)
		{
			currArc = currRep->transitions[i].arcs;
			while (currArc)
			{
				tmpArc = currArc;
				currArc = currArc->next_arc;
				free(tmpArc);
			}
		}
		free(currRep->transitions);

		// free the list of similar rhythms
		currElement = currRep->rhythms;
		while (currElement)
		{
			freeBeats(currElement->rhythm);
			tmpElement = currElement;
			currElement = currElement->next;
			free(tmpElement);
		}
		oldRep = currRep;
		currRep = currRep->next;
		free(oldRep);
	}

}

// compares this rhythm to this representation
// and tells you how close it is to it
void find_similar_rhythm_in_memory(t_rhythm_memory_representation *this_rep, 
						 t_rhythm_event *src_rhythm, // the src rhythm 
						 unsigned short int *sub_id, // the sub-id of the closest sub-rhythm 
						 float *root_closeness, // how much this rhythm is close to the root (1=identical, 0=nothing common)
						 float *sub_closeness // how much this rhythm is close to the closest sub-rhythm (1=identical, 0=nothing common)
						 )
{
	// check that the return values have been allocated
	if ((sub_id==0)||(root_closeness==0)||(sub_closeness==0))
	{
		post("error in find_similar_rhythm(): return values not allocated");
		return;
	}

	// look the main table for closeness to the main rhythm

	// TODO

	// for each rhythm in the list
	// count the number of identical nodes
	// cound thenumber of different nodes

	// TODO

	// return the better matching rhythm id
	// and the closeness floats

	// TODO
}

void find_rhythm_in_memory(t_rhythm_memory_representation *rep_list, 
						 t_rhythm_event *src_rhythm, // the src rhythm 
						 unsigned short int *id, // the id of the closest rhythm
						 unsigned short int *sub_id, // the sub-id of the closest sub-rhythm 
						 float *root_closeness, // how much this rhythm is close to the root (1=identical, 0=nothing common)
						 float *sub_closeness // how much this rhythm is close to the closest sub-rhythm (1=identical, 0=nothing common)
						 )
{
	// for each element of the rep_list

	// TODO

	// invoke find_similar_rhythm_in_memory

	// TODO

	// return the cosest representation with its subrhythm and closeness values
	
	// TODO

}


// ------------------- themes manipulation functions

// set the first note of a sequence
void setFirstNote(t_note_event **firstEvent, unsigned short int voice, float fstart, float fduration, t_note note)
{
	t_duration res;
	t_note_event *newElement;
	// convert from float to duration
	res = float2duration(fduration);
	// allocate a new element of the list
	newElement = malloc(sizeof(t_note_event));
	// set the pointers
	newElement->previous = 0;
	newElement->next = 0;
	newElement->voice=voice;
	newElement->note.chord = note.chord;
	newElement->note.diatonic = note.diatonic;
	newElement->note.interval = note.interval;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;
	res = float2duration(fstart);
	newElement->start.numerator = res.numerator;
	newElement->start.denominator = res.denominator;
	*firstEvent = newElement;
}

//adds a note at the end of this list
void concatenateNote(t_note_event *currentEvent, unsigned short int voice, float fstart, float fduration, t_note note)
{
	t_duration res;
	t_note_event *newElement, *lastElement;
	lastElement = currentEvent;
	while(lastElement->next)
		lastElement = lastElement->next;
	// convert from float to duration
	res = float2duration(fduration);
	// allocate a new element of the list
	newElement = (t_note_event *) malloc(sizeof(t_note_event));
	// set the pointers
	newElement->previous = lastElement;
	newElement->next = 0;
	lastElement->next = newElement;
	newElement->voice=voice;
	newElement->note.chord = note.chord;
	newElement->note.diatonic = note.diatonic;
	newElement->note.interval = note.interval;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;
	res = float2duration(fstart);
	newElement->start.numerator = res.numerator;
	newElement->start.denominator = res.denominator;
}

// used to free the memory allocated by this list
void freeNotes(t_note_event *currentEvent)
{
	t_note_event *prev;
	t_note_event *next;

	// go to the first element of the list
	while(currentEvent->previous)
		currentEvent = currentEvent->previous;

	// now free each element
	next=currentEvent->next;
	do
	{
		prev = currentEvent;
		next = currentEvent->next;
		free(currentEvent);
	} while(next);

}