
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

void setFirstBeat(t_rhythm_event **firstEvent, unsigned short int voice, float fduration)
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
	*firstEvent = newElement;
}

void concatenateBeat(t_rhythm_event *currentEvent, unsigned short int voice, float fduration)
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