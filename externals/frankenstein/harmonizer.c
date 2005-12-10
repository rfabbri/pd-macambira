/* 
harmonizer:
this external build voicing from a chord to another.
takes n voices (midi) as input
you can set the current chord
you can ask for next note of each voice to get to a target chord

usefull to create chorals

voicing is built using traditional GA (not co-evolving GA)

voicing rules are hardcoded and are:
- no parallel 8ths nor 5ths
- better no hidden 8ths nor 5ths
- better if uniform voice spacing (except for the bass)
- better if little intervals
- no all voices same direction
- no voices outside limits
- better complete chords

TODO:
would be nice to be able so set some rule at runtime 
or at least set the importance of rules in realtime..


*/
#include <stdlib.h>
#include <math.h>
#include <time.h>
// for string manipulation
#include <string.h>
#include <ctype.h>
#include "m_pd.h"

// to sort arrays
#include "sglib.h"

#define MAX_POPULATION 100

#define DEF_PROB_MUTATION 0.03f

#define VOICES 5

#define NOTES_RANGE 80 // this should be multiple of 16
#define LOWER_POSSIBLE_NOTE 24 // lower note possible, it should be a C
#define POSSIBLE_NOTES (NOTES_RANGE/12*4) // 4 is the max number of notes in a chord

// testing i noticed that we don't need more than 1 generation..
// this is because we create an initial population that is really good
// we may not need to crossover at all!
#define GENERATIONS 1

#define DEBUG 0 // messaggi di debug
#define DEBUG_VERBOSE 0 // messaggi di debug

static t_class *harmonizer_class;

typedef enum {
			kMaj=0, 
			kMin=1, 
			kDim=2, 
			kAug=3, 
			kDom7=4, 
			kMaj7=5,
			kMin7=6,
			kMinMaj7=7,
			kDim7=8,
			kHalfDim7=9
			} chordmode_t;

typedef enum {C=0,
			Db=1,
			D=2,
			Eb=3,
			E=4,
			F=5,
			Gb=6,
			G=7,
			Ab=8,
			A=9,
			Bb=10,
			B=11			
			} note_t;



// this defines a chord in a tonality
typedef struct _chord
{
	chordmode_t mode;
	note_t note;
} chord_t;


typedef struct _harmonizer
{
    t_object x_obj; // myself
	// genotypes
	int population[MAX_POPULATION][VOICES];
	int current_voices[VOICES];
	chord_t current_chord;
	chord_t target_chord;
	int target_notes[POSSIBLE_NOTES];
	t_outlet *l_out;
	
} t_harmonizer;

// I build a table of possible notes
// are the notes (midi)that form target_chord
void build_possible_notes_table(t_harmonizer *x)
{
	int i, octave, basenote;
	int n1, n2, n3, n4;
	n1=n2=n3=n4=0; // there always is the fundamental
	if (DEBUG_VERBOSE)
		post("build_possible_notes_table target_chord.mode=%i target_chord.note=%i", x->target_chord.mode, x->target_chord.note);
	switch (x->target_chord.mode)
	{
		case kMaj:		n2=4; n3=7; n4=0;break;
		case kMin:		n2=3; n3=7; n4=0;break;
		case kDim:		n2=3; n3=6; n4=0;break;
		case kAug:		n2=4; n3=8; n4=0;break;
		case kMaj7:		n2=4; n3=7; n4=11;break;
		case kDom7:		n2=4; n3=7; n4=10;break;
		case kMin7: 	n2=3; n3=7; n4=10;break;
		case kHalfDim7:	n2=3; n3=6; n4=10;break;
		case kDim7:		n2=3; n3=6; n4=9;break;
		case kMinMaj7:	n2=4; n3=7; n4=11;break;
	}
	if (DEBUG_VERBOSE)
		post("build_possible_notes_table n2=%i n3=%i n4=%i", n2, n3, n4);

	basenote=0;
	switch (x->target_chord.note)
	{
		case C:		basenote=0;break;
		case Db:	basenote=1;break;
		case D:		basenote=2;break;
		case Eb:	basenote=3;break;
		case E:		basenote=4;break;
		case F:		basenote=5;break;
		case Gb:	basenote=6;break;
		case G:		basenote=7;break;
		case Ab:	basenote=8;break;
		case A:		basenote=9;break;
		case Bb:	basenote=10;break;
		case B:		basenote=11;break;
	}
	if (DEBUG_VERBOSE)
		post("build_possible_notes_table basenote=%i", basenote);
	i=0;
	octave=0;
	while (i<(POSSIBLE_NOTES-3))
	{
		x->target_notes[i++]=octave*12 + LOWER_POSSIBLE_NOTE + basenote + n1;
		x->target_notes[i++]=octave*12 + LOWER_POSSIBLE_NOTE + basenote + n2;
		x->target_notes[i++]=octave*12 + LOWER_POSSIBLE_NOTE + basenote + n3;
		x->target_notes[i++]=octave*12 + LOWER_POSSIBLE_NOTE + basenote + n4;
		octave++;
	}
	if (DEBUG_VERBOSE)
	{
		i=0;
		while (i<(POSSIBLE_NOTES))
		{
			post("x->target_notes[%i]=%i", i, x->target_notes[i++]);
		}
	}
}

// tries to find out absolute tones names in this string
note_t string2note(const char *substr)
{
	if (strstr(substr, "C"))
		return C;
	if (strstr(substr, "Db"))
		return Db;
	if (strstr(substr, "D"))
		return D;
	if (strstr(substr, "Eb"))
		return Eb;
	if (strstr(substr, "E"))
		return E;
	if (strstr(substr, "F"))
		return F;
	if (strstr(substr, "Gb"))
		return Gb;
	if (strstr(substr, "G"))
		return G;
	if (strstr(substr, "Ab"))
		return Ab;
	if (strstr(substr, "A"))
		return A;
	if (strstr(substr, "Bb"))
		return Bb;
	if (strstr(substr, "B"))
		return B;
	return C;
}

chordmode_t string2mode(const char *substr)
{
	if (strstr(substr, "minor/major 7th"))
		return kMinMaj7;
	if (strstr(substr, "major 7th"))
		return kMaj7;
	if (strstr(substr, "major"))
		return kMaj;
	if (strstr(substr, "minor 7th"))
		return kMin7;
	if (strstr(substr, "minor"))
		return kMin;
	if (strstr(substr, "half diminished 7th"))
		return kHalfDim7;
	if (strstr(substr, "diminished 7th"))
		return kDim7;
	if (strstr(substr, "diminished"))
		return kDim;
	if (strstr(substr, "augmented"))
		return kAug;
	if (strstr(substr, "dominant 7th"))
		return kDom7;
	// TODO: other chords
	// beware when adding new chords
	// put shorter names at end of this function!
	return C;
}

// -----------------  normal external code ...

void harmonizer_init_pop(t_harmonizer *x)
{
	int i, j, tmp, tmp2, k, steps, note, insertpoint;
	double rnd;
	for (i=0; i<MAX_POPULATION; i++)
	{
		for (j=0; j<VOICES; j++)
		{
			/*
			// totally randome version
			rnd = rand()/((double)RAND_MAX + 1);
			tmp = rnd * POSSIBLE_NOTES;
			x->population[i][j] = x->target_notes[tmp];
			*/

			// not totally random: i start from currend chord's notes
			// and randomly go up or down
			insertpoint = 0;
			while ((insertpoint < POSSIBLE_NOTES) && (x->target_notes[insertpoint] < x->current_voices[j]))
				insertpoint++;
			if (insertpoint >= POSSIBLE_NOTES)
			{ 
				// i didn't find my insert point, possible?
				// i pick a random one
				rnd = rand()/((double)RAND_MAX + 1);
				tmp = rnd * POSSIBLE_NOTES;
				x->population[i][j] = x->target_notes[tmp];
			} else
			{
				// insert point found
				rnd = rand()/((double)RAND_MAX + 1);
				if (rnd < 0.5)
				{
					// i go up
					rnd = rand()/((double)RAND_MAX + 1);
					steps = rnd * 5; // how many steps (good notes) will I ignore?
					note = insertpoint + steps;
					if (note >= POSSIBLE_NOTES)
						note = POSSIBLE_NOTES-1;
					
				} else
				{
					// i go down
					rnd = rand()/((double)RAND_MAX + 1);
					steps = rnd * 5; // how many steps (good notes) will I ignore?
					note = insertpoint - steps;
					if (note < 0)
						note = 0;
				}
				// finally assign the note
				x->population[i][j] = x->target_notes[note];
			}
		}
	}
}


void harmonizer_free(t_harmonizer *x)
{
//	freebytes(x->buf_strum1, sizeof(x->buf_strum1));
//	freebytes(x->buf_strum2, sizeof(x->buf_strum2));
}

// here i evaluate this voicing
int fitness(t_harmonizer *x, int *candidate)
{
	int i, j, tmp, res, last, avgHI, avgLOW;
	short int chord_notes[4];
	short int chord_notes_ok[4];
	short int transitions[VOICES];
	short int directions[VOICES];
	// intervals between voices
	// for parallel and hidden 5ths
	// voices spacing etc..
	short int intervals[VOICES][VOICES]; 
	short int notes[VOICES];
	res=50; // starting fitness

	if (DEBUG_VERBOSE)
		post("evaluating fitness of %i %i %i %i", candidate[0], candidate[1], candidate[2], candidate[3]);

 	// shared objects
	for (i=0; i<VOICES; i++)
	{
		notes[i]=candidate[i];
		transitions[i] = candidate[i] - x->current_voices[i];
		if (transitions[i]!=0)
			directions[i] = transitions[i]/abs(transitions[i]);
		else
			directions[i] = 0;
		if (DEBUG_VERBOSE)
			post("directions[%i]=%i", i, directions[i]);

	}
	for (i=0; i<VOICES; i++)
	{
		for (j=i+1; j<VOICES; j++)
		{
			intervals[i][j] = (candidate[i]-candidate[j])%12 ;
			if (DEBUG_VERBOSE)
				post("intervals[%i][%i]=%i", i, j, intervals[i][j]);
		}
	}
	SGLIB_ARRAY_SINGLE_QUICK_SORT(short int, notes, VOICES, SGLIB_NUMERIC_COMPARATOR)

	// all same direction? 
	if ( directions[0]==directions[1] &&
	directions[1]==directions[2] &&
	directions[2]==directions[3])
	{
		// bad!
		res -= 10;
		if (DEBUG_VERBOSE)
			post("same direction!");
	}
	
	// parallel 5ths or octaves? (if yes return 0)
	// how?
	// hidden 8ths nor 5ths ?
	for (i=0; i<VOICES; i++)
	{
		for (j=i+1; j<VOICES; j++)
		{
			if (intervals[i][j]==7 || intervals[i][j]==0)
			{
				// hidden or parallel 5th,octave or unison
				// bad!
				if (directions[i]==directions[j])
				{
					res -= 10;
					if (DEBUG_VERBOSE)
						post("hidden or parallel consonance!");
				}
			}
		}
	}

	// is voice spacing uniform ?(except for the bass)
	// TODO: use notes[]
	// are voices average centered?
	tmp=0;
	for (i=1; i<VOICES; i++)
	{
		tmp+=notes[i];
		if (DEBUG_VERBOSE)
			post("average note is %i at passage %i", tmp, i);
	}
	// this is the average note
	tmp = tmp/(VOICES-1);
	if (DEBUG_VERBOSE)
		post("average note is %i after division by (VOICES-1)", tmp);
	tmp = abs((LOWER_POSSIBLE_NOTE + NOTES_RANGE)*2/3 - tmp); // how much average is far from 72
	res += 30; 
	res -= tmp;
	
	if (DEBUG_VERBOSE)
		post("average note is %i far from 2/3 of notes range", tmp);

	tmp=0;
	/*
	// are voices average centered?
	for (i=0; i<VOICES; i++)
	{
		tmp+=notes[i];
	}
	// this is the average note
	tmp = tmp/VOICES;
	tmp = abs(72-tmp); // how much average is far from 72
	res += 30; 
	res -= tmp;
	*/

	// are intervals small?
	//res+=50;
	if (DEBUG_VERBOSE)
		post("res before transitions %i", res);
	for (i=0; i<VOICES; i++)
	{
		if (DEBUG_VERBOSE)
			post("transitions[%i] = %i",i, transitions[i]);
		res-=abs(transitions[i]);
		// give an incentive for semitones etc..
		if (transitions[i]==0)
			res += 5;
		if (abs(transitions[i]==1))
			res += 5;
		if (abs(transitions[i]==2))
			res += 5;
		if (abs(transitions[i]==3))
			res += 2;
		if (abs(transitions[i]==4))
			res += 2;
		if (abs(transitions[i]==5))
			res += 1;
		if (abs(transitions[i]==6))
			res += 1;
		if (abs(transitions[i]>11))
			res -= 2;
		if (abs(transitions[i]>15))
			res -= 5;

	}
	if (DEBUG_VERBOSE)
		post("res after transitions %i", res);

	// TODO: too many near limits?
	
	// TODO: is a complete chord?
	// does this voicing have all 5 notes?
	// first build a table for comparision
	for (i=0; i<4; i++)
	{
		chord_notes[i] = (x->target_notes[i]) % 12;
		chord_notes_ok[i] = 0;
	}
	for (i=0; i<VOICES; i++)
	{
		tmp = notes[i] % 12;
		for (j=0; j<4; j++)
		{
			if (chord_notes[j] == tmp)
				chord_notes_ok[j]++;
		}
	}
	// now in chord_notes_ok i have the number of times each note is present
	if (chord_notes_ok[0] == 0)
	{
		// no fundamental! this is bad!!
		res -= 5;
	}
	if ((chord_notes_ok[0] != 0) &&
		(chord_notes_ok[2] != 0) &&
		(chord_notes_ok[3] != 0) && 
		(chord_notes_ok[4] != 0))
	{
		// complete chord! this is good
		res += 5;
	}
	for (j=0; j<4; j++)
	{
		res -= 2^chord_notes_ok[j];
	}
	res += 2*VOICES;

	// penalize too many basses
	tmp = 0;
	for (i=0; i<VOICES; i++)
	{
		if (notes[i]<48)
			tmp++;
	}
	switch (tmp)
	{
	case 0: res -= 5; break;
	case 1: res += 10; break;
	case 2: res -= 10; break;
	case 3: res -= 20; break;
	case 4: res -= 30; break;
	}


	if (DEBUG_VERBOSE)
		post("fitness is %i", res);

	return res;
}

void new_genotype(t_harmonizer *x, int *mammy, int *daddy, int *kid)
{
	int i, split;
	double rnd;
	// crossover
	rnd = rand()/((double)RAND_MAX + 1);
	split = rnd * VOICES;
	for (i=0; i<split; i++)
	{
		kid[i]=mammy[i];
	}
	for (i=split; i<VOICES; i++)
	{
		kid[i]=daddy[i];
	}

	//  mutation
	for (i=0; i<VOICES; i++)
	{
		rnd = rand()/((double)RAND_MAX + 1);
		if (rnd < DEF_PROB_MUTATION)
		{
			rnd = rand()/((double)RAND_MAX + 1) * POSSIBLE_NOTES;
			kid[i]=x->target_notes[(int)rnd];
		}
	}
}

typedef struct fitness_list_element_t 
{
	int index;
	int fitness;
} fitness_list_element;

#define FITNESS_LIST_COMPARATOR(e1, e2) (e1.fitness - e2.fitness)

void generate_voicing(t_harmonizer *x)
{
	fitness_list_element fitness_evaluations[MAX_POPULATION];
	int i, generation, mum, dad, winner;
	double rnd;
	t_atom lista[VOICES];
	// inizialize tables of notes
	build_possible_notes_table(x);
	// inizialize population
	harmonizer_init_pop(x);
	// GA code
	for (generation=0; generation<GENERATIONS; generation++)
	{
		// i compute all the fitness
		for (i=0; i<MAX_POPULATION; i++)
		{
			fitness_evaluations[i].index=i;
			fitness_evaluations[i].fitness = fitness(x, x->population[i]);
		}
		// i sort the array
		SGLIB_ARRAY_SINGLE_QUICK_SORT(fitness_list_element, fitness_evaluations, MAX_POPULATION, FITNESS_LIST_COMPARATOR)

		// i kill half population
		// and use the survivors to create new genotypes
		for (i=0; i<(MAX_POPULATION/2); i++)
		{
			// create a new genotype
			// parents chosen randomly
			rnd = rand()/((double)RAND_MAX + 1);
			mum = MAX_POPULATION/2 + rnd*MAX_POPULATION/2;
			rnd = rand()/((double)RAND_MAX + 1);
			dad = MAX_POPULATION/2 + rnd*MAX_POPULATION/2;
			new_genotype(x, x->population[mum], x->population[dad], x->population[i]);
		}
		// repeat the process
	}
	// finally look for the winner
	// i compute all the fitness
	for (i=0; i<MAX_POPULATION; i++)
	{
		fitness_evaluations[i].index=i;
		fitness_evaluations[i].fitness = fitness(x, x->population[i]);
	}
	// i sort the array
	SGLIB_ARRAY_SINGLE_QUICK_SORT(fitness_list_element, fitness_evaluations, MAX_POPULATION, FITNESS_LIST_COMPARATOR)
	
	winner = fitness_evaluations[MAX_POPULATION-1].index;

	if (DEBUG)
		post("winner fitness = %i", fitness_evaluations[MAX_POPULATION-1].fitness);

	for (i=0;i<VOICES;i++)
	{
		SETFLOAT(lista+i, x->population[winner][i]);
	}

	// send output array to outlet
	outlet_anything(x->l_out,
                     gensym("list") ,
					 VOICES, 
					 lista);
}

// if i want another voicing i can send a bang
static void harmonizer_bang(t_harmonizer *x) {
	generate_voicing(x);
}

// called when i send a list (with midi values)
void set_current_voices(t_harmonizer *x, t_symbol *sl, int argc, t_atom *argv)
{
	int i=0;	
	
	if (argc<VOICES)
	{
		error("insufficient notes sent!");
		return;
	}
	// fill input array with actual data sent to inlet
	for (i=0;i<VOICES;i++)
	{
		x->current_voices[i] = atom_getint(argv++);
	}

	generate_voicing(x);


}
// set current chord
void set_current(t_harmonizer *x, t_symbol *s) {
	x->current_chord.mode = string2mode(s->s_name);
	x->current_chord.note = string2note(s->s_name);
	if (DEBUG)
		post("harmonizer: set_current %s",s->s_name); 
}

// set target chord
void set_target(t_harmonizer *x, t_symbol *s) {
	x->target_chord.mode = string2mode(s->s_name);
	x->target_chord.note = string2note(s->s_name);
	if (DEBUG)
		post("harmonizer: set_target %s",s->s_name); 
}


void *harmonizer_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;
	time_t a;
    t_harmonizer *x = (t_harmonizer *)pd_new(harmonizer_class);
	x->l_out = outlet_new(&x->x_obj, &s_list);
/*
	for (i=0; i<BUFFER_LENGHT; i++)
	{
		x->last[i] = harmonizer_note2gene(1,0,0,1);
	}
	*/
	srand(time(&a));

    return (x);
}

void harmonizer_setup(void)
{
    harmonizer_class = class_new(gensym("harmonizer"), (t_newmethod)harmonizer_new,
        (t_method)harmonizer_free, sizeof(t_harmonizer), CLASS_DEFAULT, A_GIMME, 0);
    class_addbang(harmonizer_class, (t_method)harmonizer_bang);
	class_addmethod(harmonizer_class, (t_method)set_current, gensym("current"),A_SYMBOL, 0);
	class_addmethod(harmonizer_class, (t_method)set_target, gensym("target"),A_SYMBOL, 0);
//	class_addmethod(harmonizer_class, (t_method)harmonizer_fitness1_set, gensym("fitness1"), A_DEFFLOAT, 0);
	class_addlist(harmonizer_class, (t_method)set_current_voices);


}
