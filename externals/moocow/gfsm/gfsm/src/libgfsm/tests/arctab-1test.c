#include <gfsm.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
  const char *infilename = "-";
  const char *prog = argv[0];
  gfsmAutomaton *fsm=NULL;
  gfsmError     *err=NULL;
  gfsmArcTableIndex *ix=NULL;
  gfsmArcLabelIndex *lx=NULL;

  if (argc > 1) { infilename = argv[1]; }

  //-- load automaton
  fprintf(stderr, "%s: loading file: '%s'... ", prog,infilename); fflush(stderr);
  fsm = gfsm_automaton_new();
  if ( !(gfsm_automaton_load_bin_filename(fsm,infilename,&err)) ) {
    fprintf(stderr, "FAILED: %s\n", (err ? err->message : "?"));
    exit(1);
  }
  fprintf(stderr, "loaded.\n"); fflush(stderr);

  //-- ArcTableIndex
  fprintf(stderr, "%s: creating ArcTableIndex... ", prog); fflush(stderr);
  if ( !(ix = gfsm_automaton_to_arc_table_index(fsm,ix)) ) {
    fprintf(stderr, "FAILED\n");
    exit(2);
  }
  fprintf(stderr, "created.\n"); fflush(stderr);
  //
  //-- sort table (todo: check for existing sort mode?!)
  fprintf(stderr, "%s: sorting ArcTableIndex (priority sort)... ", prog); fflush(stderr);
  gfsm_arc_table_index_priority_sort(ix,gfsmASP_LW,fsm->sr);
  fprintf(stderr, "sorted.\n"); fflush(stderr);

  //-- ArcLabelIndex
  fprintf(stderr, "%s: creating ArcLabelIndex... ", prog); fflush(stderr);
  if ( !(lx = gfsm_automaton_to_arc_label_index_lower(fsm,lx)) ) {
    fprintf(stderr, "FAILED\n");
    exit(3);
  }
  fprintf(stderr, "created.\n"); fflush(stderr);

  //-- cleanup
  fprintf(stderr, "%s: cleanup... ", prog); fflush(stderr);
  gfsm_automaton_free(fsm);
  gfsm_arc_table_index_free(ix);
  gfsm_arc_label_index_free(lx);
  fprintf(stderr, "done.\n"); fflush(stderr);

  return 0;
}
