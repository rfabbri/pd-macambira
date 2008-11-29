#include <gfsm.h>


//#define USE_VTABLE 1
//#define USE_ALLOCATORS 1
//#define DELAY_ALLOCATOR_FREE 1

#define NEW_FST     1

//#define MAKE_SLIST 1
//#define MAKE_LIST 1
//#define MAKE_FST    1
//#define COMPILE_FST    1
#define LOAD_FST    1
//#define LOAD_EMPTY
//#define LOAD_NOFINAL

#define LOAD_NITERS 1
//#define LOAD_NITERS 10
//#define LOAD_NITERS 32768
//#define LOAD_NITERS 65535
//#define LOAD_NITERS 131072
//#define LOAD_NITERS 262144
//#define LOAD_NITERS 524288
//#define LOAD_NITERS 1048576


#define NEW_ABET    1
#define LOAD_ABET   1

//#define DO_INPUT    1
//#define DO_LOOKUP   1
//#define DO_PATHS    1
//#define DO_STRINGS  1
//#define DO_PTRARRAY 1

#define NITERS 0
//#define NITERS 1
//#define NITERS 10
//#define NITERS 1024
//#define NITERS 65536
//#define NITERS 131072
//#define NITERS 262144
//#define NITERS 524288
//#define NITERS 1048576

//#define PRINT_CHUNK_INFO 1
//#define DO_PROFILE 1

const char *progname = "pathtest";
const char *labfile  = "test.lab";

#if defined(LOAD_EMPTY)
const char *fstfile  = "empty.gfst";
const char *tfstfile  = "empty.tfst";
#elif defined(LOAD_NOFINAL)
const char *fstfile  = "nofinal.gfst";
const char *tfstfile  = "nofinal.tfst";
#else
const char *fstfile  = "lkptest.gfst";
const char *tfstfile  = "lkptest.tfst";
#endif


gfsmLabelVector *input = NULL;
gfsmAlphabet  *abet = NULL;
gfsmAutomaton *fst = NULL;
gfsmAutomaton *result = NULL;
gfsmSet       *paths = NULL;
GSList        *strings = NULL;
GPtrArray     *ptrarray = NULL;
gfsmError     *err = NULL;
GSList       *sltmp = NULL;
gfsmState     *st = NULL;
gfsmArc       *arc=NULL;
gfsmArcList   *al=NULL;
char line[256];


gpointer my_malloc(gsize n_bytes)
{
  return (gpointer)malloc(n_bytes);
}

gpointer my_realloc(gpointer mem, gsize n_bytes)
{
  return (gpointer)realloc(mem, n_bytes);
}

void my_free(gpointer mem)
{
  free(mem);
}

GMemVTable my_vtable = 
  {
    my_malloc,
    my_realloc,
    my_free,
    NULL,
    NULL,
    NULL
  };

#define MEMOP(code) \
  fprintf(stderr,"%s\n", #code); \
  code;

int main(int argc, char **argv) {
  int i;

  //-- memory debugging
#if defined(DO_PROFILE)
  g_mem_set_vtable(glib_mem_profiler_table);
#elif defined(USE_VTABLE)
  g_mem_set_vtable(&my_vtable);
#endif

  //-- setup gfsm allocators
#if defined(USE_ALLOCATORS)
  MEMOP(gfsm_allocators_enable());
#endif

  //-- load or make fst
#ifdef NEW_FST
  MEMOP(fst = gfsm_automaton_new(););

#if defined(MAKE_SLIST)
  //-- this is the culprit!
  MEMOP(al = g_slist_prepend(NULL,NULL));
  MEMOP(g_slist_free(al));
#elif defined(MAKE_LIST)
  {
    GList *l=NULL;
    MEMOP(l=g_list_prepend(NULL,NULL));
    MEMOP(g_list_free(l));
  }
#elif defined(MAKE_FST)
  MEMOP(st = gfsm_automaton_get_state(fst,0)); //-- ok
  MEMOP(gfsm_automaton_set_final_state(fst, 0, TRUE)); //-- ok

  MEMOP(gfsm_automaton_add_arc(fst,0,0,1,1,0)); //-- NOT ok!

  //-- alloc
  //MEMOP(arc=gfsm_arc_new_full(0,1,1,0)); //--ok
  //MEMOP(st->arcs = g_slist_prepend((gpointer)arc,st->arcs)); //-- ok w/ allocator

  //-- free
  //MEMOP(g_slist_free(st->arcs); st->arcs=NULL;); //-- ok w/ allocator
  //MEMOP(gfsm_arc_free(arc)); //-- /ok

#elif defined(COMPILE_FST)
  fprintf(stderr,"gfsm_automaton_compile_filename(\"%s\");\n", tfstfile);
  if (!gfsm_automaton_compile_filename(fst,tfstfile,&err)) {
    fprintf(stderr,"%s: compile failed for '%s': %s\n", progname, tfstfile, (err ? err->message : "?"));
    exit(3);
  }
  //g_mem_profile();
#elif defined(LOAD_FST)
  fprintf(stderr,"gfsm_automaton_load_bin_filename(\"%s\"); //---[x %d]---\n", fstfile, LOAD_NITERS);
  for (i=0; i < LOAD_NITERS; i++) {
    if (fst) gfsm_automaton_free(fst);
    fst = gfsm_automaton_new();
    if (!gfsm_automaton_load_bin_filename(fst,fstfile,&err)) {
      fprintf(stderr,"%s: load failed for '%s': %s\n", progname, fstfile, (err ? err->message : "?"));
      exit(3);
    }
    g_blow_chunks();
  }
  //g_mem_profile();
#endif // make or load FST
#endif // NEW_FST

  //-- load labels
#ifdef NEW_ABET
  MEMOP(abet = gfsm_string_alphabet_new(););
#ifdef LOAD_ABET
  fprintf(stderr,"gfsm_alphabet_load_filename(\"%s\");\n", labfile);
  if (!gfsm_alphabet_load_filename(abet,labfile,&err)) {
    fprintf(stderr,"%s: load failed for labels file '%s': %s\n",
	       progname, labfile, (err ? err->message : "?"));
    exit(2);
  }
  //g_mem_profile();
#endif //-- LOAD_ABET
#endif //-- NEW_ABET

  //-- setup input vector
#ifdef DO_INPUT
  MEMOP(input = g_ptr_array_new());
  MEMOP(g_ptr_array_add(input,(gpointer)2));
  MEMOP(g_ptr_array_add(input,(gpointer)2));
  MEMOP(g_ptr_array_add(input,(gpointer)3));
#endif //-- DEFINE_INPUT

  //-- guts
  fprintf(stderr, "\n--bench[%d] :lookup=%d, paths=%d, strings=%d, ptrarray=%d--\n\n",
	  NITERS,
#ifdef DO_LOOKUP
	  1
#else
	  0
#endif
	  ,
#ifdef DO_PATHS
	  1
#else
	  0
#endif
	  ,
#ifdef DO_STRINGS
	  1
#else
	  0
#endif
	  ,
#ifdef DO_PTRARRAY
	  1
#else
	  0
#endif
	  );

  for (i=0; i < NITERS; i++) {
#ifdef DO_LOOKUP
    result  = gfsm_automaton_lookup(fst, input, result);
#endif
#ifdef DO_PATHS
    paths   = gfsm_automaton_paths(result,paths);
#endif
#ifdef DO_STRINGS
    strings = gfsm_paths_to_strings(paths, abet, NULL, fst->sr, TRUE, TRUE, strings);
#endif
#ifdef DO_PTRARRAY
    ptrarray=g_ptr_array_sized_new(gfsm_set_size(paths));
    gfsm_set_to_ptr_array(paths, ptrarray);
#endif

    //-- cleanup
    for (sltmp=strings; sltmp; sltmp=sltmp->next) { g_free(sltmp->data);  }
    if (ptrarray) g_ptr_array_free(ptrarray,TRUE);
    if (strings) g_slist_free(strings);
    if (paths)   gfsm_set_clear(paths);
    g_blow_chunks();
  }

  //-- pop gfsm allocators (too early: segfaults)
  /*
#if defined(USE_ALLOCATORS) && !defined(DELAY_ALLOCATOR_FREE)
  MEMOP(gfsm_allocators_disable());
#endif
  */

  //-- cleanup
  if (result) { MEMOP(gfsm_automaton_free(result)); }
  if (paths)  { MEMOP(gfsm_set_free(paths)); }
  if (input) { MEMOP(g_ptr_array_free(input,TRUE)); }
  if (fst) { MEMOP(gfsm_automaton_free(fst)); }
  if (abet) { MEMOP(gfsm_alphabet_free(abet);); }

  //-- pop gfsm allocators
#if defined(USE_ALLOCATORS) && !defined(DELAY_ALLOCATOR_FREE)
  MEMOP(gfsm_allocators_free());
#endif

  //-- memory debugging
#ifdef PRINT_CHUNK_INFO
  printf("\n<CHUNKS:1>--------\n");
  g_blow_chunks();
  g_mem_chunk_info();
#endif
  //
#ifdef DO_PROFILE
  printf("\n<PROF:1>--------\n");
  g_blow_chunks();
  g_mem_profile();
#endif

#if defined(USE_ALLOCATORS) && defined(DELAY_ALLOCATOR_FREE)
  MEMOP(gfsm_allocators_free());
#endif

 {
   printf("OK to exit? ");
   scanf("%s", &line);
 }

  return 0;
}
