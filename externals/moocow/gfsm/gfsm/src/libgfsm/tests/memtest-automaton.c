#include <glib.h>
#include <gfsm.h>

void dofree (gpointer p) { g_free(p); }

int main (void) {
  gfsmAutomaton *fsm;
  g_mem_set_vtable(glib_mem_profiler_table);

  fsm = gfsm_automaton_new();
  gfsm_automaton_free(fsm);

  //printf("<CHUNKS:1>--------\n");
  //g_mem_chunk_info();

  printf("<PROF:1>--------\n");
  g_blow_chunks();
  g_mem_profile();
  return 0;
}
