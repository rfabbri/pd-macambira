#include <gfsm.h>
#include <stdlib.h>

//char *fsmfile = "prtest.tfst";
char *fsmfile = "prtest.gfst";
gfsmError *errp = NULL;

int main (int argc, char **argv) {
  gfsmAutomaton *fsm=NULL;

  if (argc > 1) fsmfile = argv[1];

  fsm = gfsm_automaton_new();

  //gfsm_automaton_compile_filename(fsm,fsmfile,&errp);
  gfsm_automaton_load_bin_filename(fsm,fsmfile,&errp);
  if (errp) { g_printerr("error: %s\n", errp->message); exit(1); }

  gfsm_automaton_prune(fsm);
  gfsm_automaton_renumber_states(fsm);

  gfsm_automaton_print_file(fsm,stdout,&errp);
  if (errp) { g_printerr("error: %s\n", errp->message); exit(1); }

  if (fsm) gfsm_automaton_free(fsm);

  return 0;
}
