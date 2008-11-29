#include <gfsm.h>
#include <stdio.h>
#include <stdlib.h>

const char *progname = "test-stateset";
const char *infilename = "statetest.tfst";

gfsmAutomaton *fsm;
gfsmError *err = NULL;

gboolean stateset_print_func(gfsmStateId id, gpointer data)
{
  printf(" %u", id);
  return FALSE;
}

void stateset_print(const char *label, gfsmStateSet *sset)
{
  gfsmStateSetIter ssi;
  gfsmStateId      ssid;

  printf("%s: {", label);

  //gfsm_stateset_foreach(sset, stateset_print_func, NULL);
  for (ssi = gfsm_stateset_iter_begin(sset); 
       (ssid=gfsm_stateset_iter_id(ssi)) != gfsmNoState;
       ssi = gfsm_stateset_iter_next(sset,ssi))
    {
      printf(" %u", ssid);
    }

  printf(" }\n");
}

int main (int argc, char **argv) {
  int i;
  gfsmStateId  id;
  gfsmStateSet *sset;

  fsm = gfsm_automaton_new();
  sset = gfsm_stateset_new();

  if (!gfsm_automaton_compile_filename(fsm,infilename,&err)) {
    g_printerr("%s: compile failed for '%s': %s\n", progname, infilename, err->message);
    exit(255);
  }
  printf("%s: compiled test automaton from '%s'\n", *argv, infilename);

  for (i=0; i < argc; i++) {
    id = strtol(argv[i],NULL,10);
    gfsm_stateset_clear(sset);
    gfsm_stateset_populate(sset,fsm,id, gfsmEpsilon, gfsmEpsilon);

    printf("--\nseed=%u\n", id);
    stateset_print("equiv", sset);
  }

  gfsm_stateset_free(sset);
  gfsm_automaton_free(fsm);
  return 0;
}
