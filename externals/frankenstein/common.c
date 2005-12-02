
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

void setFirstBeat(t_rhythm_event **firstEvent, unsigned short int voice, float fduration, unsigned short int played)
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
	newElement->played=played;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;
	*firstEvent = newElement;
}

void concatenateBeat(t_rhythm_event *currentEvent, unsigned short int voice, float fduration, unsigned short int played)
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
	newElement->played=played;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;

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

void add_t_rhythm_memory_arc(t_rhythm_memory_node *srcNode, t_rhythm_memory_node *dstNode)
{
	t_rhythm_memory_arc *newArc;
	t_rhythm_memory_arc *lastArc;

	// create a new arc
	newArc = (t_rhythm_memory_arc *) malloc(sizeof(t_rhythm_memory_arc));
	newArc->to_node = dstNode;
	newArc->weight = 1;
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
void init_rhythm_memory_representation(t_rhythm_memory_representation *this_rep)
{
	this_rep->transitions = (t_rhythm_memory_node *) malloc(sizeof(t_rhythm_memory_node) * possible_durations());
	this_rep->main_rhythm = 0;
	this_rep->similar_rhythms = 0;
}

// add a new rhythm in the list of similar thythms related to one main rhythm
void add_t_rhythm_memory_element(t_rhythm_memory_representation *this_rep, t_rhythm_event *new_rhythm)
{
	t_rhythm_memory_element *curr;
	t_rhythm_memory_element *newElement;
	// creates a new element of the list of similar rhythms
	newElement = (t_rhythm_memory_element *) malloc(sizeof(t_rhythm_memory_element));
	newElement->rhythm = new_rhythm;
	newElement->next = 0;
	// finds the last element and adds itself
	curr = this_rep->similar_rhythms;
	if (curr)
	{
		while (curr->next)
			curr=curr->next;
		curr->next = newElement;

	} else
	{
		this_rep->similar_rhythms = newElement;
	}


}

// from (duration) to (start) style
void swap_rhythm_to_start(t_rhythm_event *oldStyle, t_rhythm_event **newStyle)
{
	t_rhythm_event *oldCurr, *newElement, *oldPrev;
	float diff, currMoment;
	t_duration dur_tmp;

	// allocate the first event
	oldCurr = oldStyle;
	oldPrev = 0;
	currMoment = 0;
	
	// look for the first beat played in old rhythm
	while (oldCurr && (! (oldCurr->played)))
	{
		// prepare for the next event
		dur_tmp.numerator = oldCurr->duration.numerator;
		dur_tmp.denominator = oldCurr->duration.denominator;
		currMoment = duration2float(dur_tmp);
		oldPrev = oldCurr;
		oldCurr = oldCurr->next;
	}

	if (currMoment == 0)
		return; // empty rhythm?!?

	// now in currMoment there is the moment of the first played beat
	// i can set the first one and initialize the new style
	newElement = (t_rhythm_event *) malloc(sizeof(t_rhythm_event));
	*newStyle = newElement;
	dur_tmp = float2duration(currMoment);
	newElement->duration.numerator = dur_tmp.numerator;
	newElement->duration.denominator = dur_tmp.denominator;
	newElement->previous = oldPrev;
	if (oldPrev)
		oldPrev->next = newElement;

	while (oldCurr)
	{
		if (oldCurr->played)
		{
			// allocate a new element
			newElement = (t_rhythm_event *) malloc(sizeof(t_rhythm_event));
			// set its value
			dur_tmp = float2duration(currMoment);
			newElement->duration.numerator = dur_tmp.numerator;
			newElement->duration.denominator = dur_tmp.denominator;
			newElement->previous = oldPrev;
			if (oldPrev)
				oldPrev->next = newElement;
		}
		// prepare for the next event
		dur_tmp.numerator = oldCurr->duration.numerator;
		dur_tmp.denominator = oldCurr->duration.denominator;
		currMoment = duration2float(dur_tmp);
		oldPrev = oldCurr;
		oldCurr = oldCurr->next;
	}


}

// from (start) to (duration) style
void swap_rhythm_to_duration(t_rhythm_event *oldStyle, t_rhythm_event **newStyle)
{

}


// ------------------- themes manipulation functions

// set the first note of a sequence
void setFirstNote(t_note_event **firstEvent, unsigned short int voice, float fduration, t_note note)
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
	newElement->note.played = note.played;
	newElement->note.chord = note.chord;
	newElement->note.diatonic = note.diatonic;
	newElement->note.interval = note.interval;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;
	*firstEvent = newElement;
}

//adds a note at the end of this list
void concatenateNote(t_note_event *currentEvent, unsigned short int voice, float fduration, t_note note)
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
	newElement->note.played = note.played;
	newElement->note.chord = note.chord;
	newElement->note.diatonic = note.diatonic;
	newElement->note.interval = note.interval;
	newElement->duration.numerator = res.numerator;
	newElement->duration.denominator = res.denominator;

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