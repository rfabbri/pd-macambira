#include <gfsm.h>
#include <stdio.h>

int main (void) {
  gfsmAutomaton *fsm = gfsm_automaton_new();

  printf("##-- determinize()-1...\n");
  gfsm_automaton_determinize(fsm);
  gfsm_automaton_print_file(fsm,stdout,NULL);

  printf("##-- determinize()-2...\n");
  gfsm_automaton_determinize(fsm);
  gfsm_automaton_print_file(fsm,stdout,NULL);

  printf("##-- determinize()-3...\n");
  gfsm_automaton_determinize(fsm);
  gfsm_automaton_print_file(fsm,stdout,NULL);

  printf("done.\n");
  return 0;
}
