// here i put common data structures and functions

// ------------------------------------------------ data structures


// --------- theme notation


typedef struct t_note t_note;
typedef struct t_duration t_duration;
typedef struct t_note_event t_note_event;

typedef struct t_note
{
	unsigned short int played; // 0 if is a rest, 1 if played
	unsigned short int chord; // 0 if is not a chord (strong) note, 1 if it is
	unsigned short int diatonic; // 0 if is a note not belonging to this tonality
	int interval; // semitones from prefious note
};
typedef struct t_duration
{
	int numerator; // like in music notation: in a 1/4 note the numerator is 1
	int denominator; // like in music notation: in a 1/4 note the denominator is 4
};
struct t_note_event
{
	unsigned short int voice;
	t_note note;
	t_duration duration;
	t_note_event *previous; // this is a list, link to the previous element
	t_note_event *next;  // this is a list, link to the next element
};


// --------- rhythm notation

// data structures

// this describes a rhythm
typedef struct t_rhythm_event t_rhythm_event;
struct t_rhythm_event
{
	unsigned short int voice;
	unsigned short int played; // 0 if is a rest, 1 if played
	t_duration duration;
	t_rhythm_event *previous; // this is a list, link to the previous element
	t_rhythm_event *next;  // this is a list, link to the next element
};

// rhythms memory graph

// list implementation
// this implements a graph that stores the memory of the current rhythm sub-elements
// list of links
// the actual implementation will be an array of nodes, each node 

// this describes a probability transition table
typedef struct t_rhythm_memory_arc t_rhythm_memory_arc;
typedef struct t_rhythm_memory_node t_rhythm_memory_node;
// graph node
struct t_rhythm_memory_node
{
	t_duration duration;
	t_rhythm_memory_arc *arcs; // the list of arcs to other nodes
} ;
// graph arc: related to t_rhythm_memory_node
struct t_rhythm_memory_arc
{
	unsigned short int weight;
	t_rhythm_memory_node *to_node; // the target of this link (arc)
	t_rhythm_memory_arc *next_arc; // next link in the list
} ;
// it will be arranged in a heap list.. ?

#define num_possible_denominators 11
static unsigned short int possible_denominators[] = {1,2,3,4,6,8,12,16,18,24,32};

// this defines a space for rhythms, variations, transitions and representations
typedef struct t_rhythm_memory_representation t_rhythm_memory_representation;
typedef struct t_rhythm_memory_element t_rhythm_memory_element;
// element of a list of rhythms
struct t_rhythm_memory_element
{
	t_rhythm_event *rhythm; // this rhythm
	t_rhythm_memory_element *next; // next element of the list
} ;
// a rhythm in memory, each rhythm is :
// - a main rhythm
// - its probability transition table
// - similar rhythms played
struct t_rhythm_memory_representation
{
	t_rhythm_memory_element *main_rhythm;
	t_rhythm_memory_node *transitions;
	t_rhythm_memory_element *similar_rhythms;
} ;

// ------------------------------------------------ functions

// ----------- rhythm manipolation functions

// TODO: 
// - from data structures to lists of numbers and vice versa
// - from a (voice, duration) representation to (voice, start, duration) and viceversa

// converts from integer to duration: used to know this table index
// what corresponds in terms of duration
t_duration int2duration(int n);
// converts from duration to integer: used to know this duration
// what corresponds in terms table index
unsigned short int duration2int(t_duration dur);
// tells you how many durations there are
int possible_durations();

// converts from float (0-1) to duration. it performs quantization
t_duration float2duration(float fduration);

// converts from numerator/denominator to a float (0-1)
float duration2float(t_duration duration);

// --- rhythms creation and manupulation functions

// set the first beat of a sequence
void setFirstBeat(t_rhythm_event **firstEvent, unsigned short int voice, float fduration, unsigned short int played);

//adds a beat at the end of this list
void concatenateBeat(t_rhythm_event *currentEvent, unsigned short int voice, float fduration, unsigned short int played);

// used to free the memory allocated by this list
void freeBeats(t_rhythm_event *currentEvent);

// change rhythm description protocol

// from (duration) to (start) style
// the (start) style is a list of noteon events, the order is not important
void swap_rhythm_to_start(t_rhythm_event *oldStyle, t_rhythm_event **newStyle);

// from (start) to (duration)
// the (duration) style is a linked list of duration of both notes or rests, the order is important
void swap_rhythm_to_duration(t_rhythm_event *oldStyle, t_rhythm_event **newStyle);

// --- memory representation of rhythms

// add an arc to this node
void add_t_rhythm_memory_arc(t_rhythm_memory_node *srcNode, t_rhythm_memory_node *dstNode);

// initialize this representation, allocates memory for the pointers
void init_rhythm_memory_representation(t_rhythm_memory_representation *this_rep);

// add a new rhythm in the list of similar rhythms related to one main rhythm
void add_similar_rhythm(t_rhythm_memory_representation *this_rep, t_rhythm_event *new_rhythm);

// free the array with pointers and lists
void free_memory_representation(t_rhythm_memory_representation *this_rep);

// functions needed to fill and use the memory table



// -------- notes manipulation functions

// set the first beat of a sequence
void setFirstNote(t_note_event **firstEvent, unsigned short int voice, float fduration, t_note note);

//adds a beat at the end of this list
void concatenateNote(t_note_event *currentEvent, unsigned short int voice, float fduration, t_note note);

// used to free the memory allocated by this list
void freeNotes(t_note_event *currentEvent);

