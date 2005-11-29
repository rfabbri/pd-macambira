
#include "common.h"

t_duration int2duration(int n)
{
	t_duration dur;
	int curr, i, j, ok;
	curr=0;
	ok=0;
	for (i=0; i<num_possible_denominators; i++)
	{
		for (j=0; j<i; j++)
		{
			if (curr==n)
			{
				dur.numerator=j;
				dur.denominator=i;
				j=i;
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