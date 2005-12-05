
// here i put common data structures and functions

// ------------------------------------------------ data structures


// --------- theme notation


typedef struct t_note t_note;
typedef struct t_duration t_duration;
typedef struct t_note_event t_note_event;

typedef struct t_note
{
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
	t_duration start; // moment of the noteon event
	t_duration duration; // duration of this event
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
	t_duration start; // moment of the noteon event
	t_duration duration; // duration of this event
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
	// start is implicit:
	// this is in an array
	// use int2duration and duration2int 
	// against the index in the array to get its value
	unsigned short int first; // how many times this has been the first event
	unsigned short int weight; // how many times this has been an event
	t_rhythm_memory_arc *arcs; // the list of arcs to other nodes
} ;
// graph arc: related to t_rhythm_memory_node
struct t_rhythm_memory_arc
{
	unsigned short int to_node_index; // the target of this link (arc)
	t_rhythm_memory_arc *next_arc; // next link in the list
} ;
// it will be arranged in a heap list.. ?

#define num_possible_denominators 11
static unsigned short int possible_denominators[] = {1,2,3,4,6,8,12,16,18,24,32};

// this defines a space for rhythms, variations, transitions and representations
typedef struct t_rhythm_memory_representation t_rhythm_memory_representation;
typedef struct t_rhythm_memory_element t_rhythm_memory_element;
typedef struct t_rhythm_memory_first_node t_rhythm_memory_first_node;
// element of a list of rhythms
struct t_rhythm_memory_element
{
	t_rhythm_event *rhythm; // this rhythm
	t_rhythm_memory_element *next; // next element of the list
	unsigned short int id; // its sub id
} ;
// a rhythm in memory, each rhythm is :
// - its probability transition table
// - similar rhythms played
// - each one has its main id and each different played rhythm its sub-id
struct t_rhythm_memory_representation
{
	t_rhythm_memory_node *transitions;
	unsigned short int max_weight;
	t_rhythm_memory_element *rhythms;
	unsigned short int id; // its main id
	unsigned short int last_sub_id; // last sub assigned
	// I can express a list of representations with this data structure
	t_rhythm_memory_representation *next;
} ;

// ------------------------------------------------ functions

// ----------- rhythm manipolation functions

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
// this also creates a new rhythm
void setFirstBeat(t_rhythm_event **firstEvent, unsigned short int voice, float fstart, float fduration);

//adds a beat at the end of this list
void concatenateBeat(t_rhythm_event *currentEvent, unsigned short int voice, float fstart, float fduration);

// used to free the memory allocated by this list
void freeBeats(t_rhythm_event *currentEvent);


// --- memory representation of rhythms

// add an arc to this node
void add_t_rhythm_memory_arc(t_rhythm_memory_node *srcNode, unsigned short int dstNode);

// create and initialize this representation, allocate memory for the pointers
// I must pass its id also
void create_rhythm_memory_representation(t_rhythm_memory_representation **this_rep, unsigned short int id);

// define a return value to express "rhythm not found in this representation"
#define INVALID_RHYTHM 65535

// add a new rhythm in the list of similar rhythms related to one main rhythm
// the sub id is auto-generated and returned
unsigned short int add_similar_rhythm(t_rhythm_memory_representation *this_rep, t_rhythm_event *new_rhythm);

// free the list of representations
void free_memory_representations(t_rhythm_memory_representation *this_rep);

// compares this rhythm to this representation
// and tells you how close it is to it
// I return values using pointers
// the unsigned short and the 2 floats should be already allocated
void compare_rhythm_vs_representation(t_rhythm_memory_representation *this_rep, 
						 t_rhythm_event *src_rhythm, // the src rhythm 
						 unsigned short int *sub_id, // the sub-id of the closest sub-rhythm 
						 float *root_closeness, // how much this rhythm is close to the root (1=identical, 0=nothing common)
						 float *sub_closeness // how much this rhythm is close to the closest sub-rhythm (1=identical, 0=nothing common)
						 );

// same as before but search all available representations
void find_rhythm_in_memory(t_rhythm_memory_representation *rep_list, 
						 t_rhythm_event *src_rhythm, // the src rhythm 
						 unsigned short int *id, // the id of the closest rhythm
						 unsigned short int *sub_id, // the sub-id of the closest sub-rhythm 
						 float *root_closeness, // how much this rhythm is close to the root (1=identical, 0=nothing common)
						 float *sub_closeness // how much this rhythm is close to the closest sub-rhythm (1=identical, 0=nothing common)
						 );

// -------- notes manipulation functions

// set the first beat of a sequence
void setFirstNote(t_note_event **firstEvent, unsigned short int voice, float fstart, float fduration, t_note note);

//adds a beat at the end of this list
void concatenateNote(t_note_event *currentEvent, unsigned short int voice, float fstart, float fduration, t_note note);

// used to free the memory allocated by this list
void freeNotes(t_note_event *currentEvent);

