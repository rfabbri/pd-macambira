#include <gfsm.h>
#include <gfsmDebug.h>
#include <stdlib.h>

//char *fsm1file = "rudtest.gfst";
char *fsm1file = "rudtest2.gfst";
char *fsm2file = "rudtest.gfst";

gfsmError *errp = NULL;

void hackme(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2, char *label)
{
  printf("<%s>.a : reverse(fsm1)\n", label);
  gfsm_automaton_reverse(fsm1);

  printf("<%s>.b : union(fsm1,fsm2)\n", label);
  gfsm_automaton_union(fsm1, fsm2);

  printf("<%s>.c : determinize(fsm1)\n", label);
  gfsm_automaton_determinize(fsm1);

  printf("<%s>   : done.\n", label);
}

void print_sizes(void)
{
  //-- show some memory sizes:
  fprintf(stderr, "%-32s | %6s\n", "TYPE", "SIZE");
#define PRINTSIZE(type) fprintf(stderr, "%-32s | %u\n", #type, sizeof(type))
  PRINTSIZE(gfsmAlphabet);
  PRINTSIZE(gfsmIdentityAlphabet);
  PRINTSIZE(gfsmRangeAlphabet);

  PRINTSIZE(gfsmArc);
  PRINTSIZE(gfsmArcIter);
  PRINTSIZE(gfsmArcList);
  PRINTSIZE(gfsmAutomaton);
  PRINTSIZE(gfsmAutomatonFlags);
  PRINTSIZE(gfsmAutomatonHeader);
  PRINTSIZE(gfsmSemiring);
  //PRINTSIZE(gfsmSet);
  //PRINTSIZE(gfsmSetUnionData);
  PRINTSIZE(gfsmWeightedStateSet);
  PRINTSIZE(gfsmState);
  PRINTSIZE(gfsmStatePair);
  PRINTSIZE(gfsmStateSet);
  PRINTSIZE(gfsmStateSetIter);
  PRINTSIZE(gfsmStoredState);
  PRINTSIZE(gfsmStoredArc);
  PRINTSIZE(gfsmVersionInfo);
#undef PRINTSIZE
}

void rudtest_construct(gfsmAutomaton *fsm)
{
  gfsm_automaton_clear(fsm);
  fsm->root_id = 0;
  gfsm_automaton_add_state_full(fsm,0);
  gfsm_automaton_add_state_full(fsm,1);
  gfsm_automaton_add_state_full(fsm,2);
  gfsm_automaton_set_final_state(fsm,2,TRUE);
  gfsm_automaton_add_arc(fsm,0,1,1,1,0);
}

int main (int argc, char **argv) {
  gfsmAutomaton *fsm1=NULL, *fsm2=NULL;

  //g_thread_init(NULL);
  GFSM_DEBUG_INIT();
  
  //print_sizes();

  if (argc > 1) fsm1file = argv[1];
  if (argc > 2) fsm2file = argv[2];

  fsm1 = gfsm_automaton_new();
  fsm2 = gfsm_automaton_new();

  gfsm_automaton_load_bin_filename(fsm1,fsm1file,&errp);
  //rudtest_construct(fsm1);
  if (errp) { g_printerr("error: %s\n", errp->message); exit(1); }

  if (fsm2) gfsm_automaton_load_bin_filename(fsm2,fsm1file,&errp);
  if (errp) { g_printerr("error: %s\n", errp->message); exit(1); }

  hackme(fsm1,fsm2,"1");
  hackme(fsm1,fsm2,"2");
  hackme(fsm1,fsm2,"3");
  hackme(fsm1,fsm2,"4");
  /*  */

  if (fsm1) gfsm_automaton_free(fsm1);
  if (fsm2) gfsm_automaton_free(fsm2);


  GFSM_DEBUG_FINISH();
  GFSM_DEBUG_PRINT();

  return 0;
}
