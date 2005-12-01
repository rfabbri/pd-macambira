// here i put common data structures and functions

// --------- theme notation

// data structures

typedef struct _note
{
	unsigned short int played; // 0 if is a rest, 1 if played
	unsigned short int chord; // 0 if is not a chord (strong) note, 1 if it is
	unsigned short int diatonic; // 0 if is a note not belonging to this tonality
	int interval; // semitones from prefious note
} t_note;

typedef struct _duration
{
	int numerator; // like in music notation: in a 1/4 note the numerator is 1
	int denominator; // like in music notation: in a 1/4 note the denominator is 4
} t_duration;

typedef struct _note_event
{
	unsigned short int voice;
	t_note note;
	t_duration duration;
	struct t_note_event *previous; // this is a list, link to the previous element
	struct t_note_event *next;  // this is a list, link to the next element
} t_note_event;


// --------- rhythm notation

// data structures

typedef struct t_rhythm_event t_rhythm_event;
struct t_rhythm_event
{
	unsigned short int voice;
	t_duration duration;
	t_rhythm_event *previous; // this is a list, link to the previous element
	t_rhythm_event *next;  // this is a list, link to the next element
};

// rhythms memory graph

/*
// list implementation
// this implements a graph that stores the memory of the current rhythm sub-elements
// list of links
typedef struct _rhythm_memory_arc
{
	float weight;
	struct t_rhythm_memory_node *to_node; // the target of this link (arc)
	struct t_rhythm_memory_arc *next_arc; // next link in the list
} t_rhythm_memory_arc;
// graph node
typedef struct _rhythm_memory_node
{
	t_duration duration;
	struct t_rhythm_memory_arc *arcs; // the list of arcs to other nodes
} t_rhythm_memory_node;
*/
// with a table
// simpler and most of all non recursive when searching nodes!
#define num_possible_denominators 11
static unsigned short int possible_denominators[] = {1,2,3,4,6,8,12,16,18,24,32};

// functions needed to fill and use the memory table

// converts from integer to duration: used to know this table index
// what corresponds in terms of duration
t_duration int2duration(int n);
// converts from duration to integer: used to know this duration
// what corresponds in terms table index
unsigned short int duration2int(t_duration dur);
// tells you how many durations there are
int possible_durations();

// ----------- rhythm manipolation functions

// TODO: 
// - from data structures to lists of numbers and vice versa
// - from a (voice, duration) representation to (voice, start, duration) and viceversa

// converts from float (0-1) to duration. it performs quantization
t_duration float2duration(float fduration);

// converts from numerator/denominator to a float (0-1)
float duration2float(t_duration duration);

// set the first beat of a sequence
void setFirstBeat(t_rhythm_event **firstEvent, unsigned short int voice, float fduration);

//adds a beat at the end of this list
void concatenateBeat(t_rhythm_event *currentEvent, unsigned short int voice, float fduration);

// used to free the memory allocated by this list
void freeBeats(t_rhythm_event *currentEvent);

// -------- notes manipulation functions

// set the first beat of a sequence
void setFirstNote(t_note_event **firstEvent, unsigned short int voice, float fduration, t_note note);

//adds a beat at the end of this list
void concatenateNote(t_note_event *currentEvent, unsigned short int voice, float fduration, t_note note);

// used to free the memory allocated by this list
void freeNotes(t_note_event *currentEvent);

