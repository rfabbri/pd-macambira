#include <glib.h>
#include <gfsmAlphabet.h>

void dofree (gpointer p) { g_free(p); }

int main (void) {
  gfsmAlphabet *a;
  g_mem_set_vtable(glib_mem_profiler_table);

  //a = (gfsmAlphabet*)g_new0(gfsmPointerAlphabet,1);
  a = gfsm_string_alphabet_new();

  gfsm_alphabet_insert(a,"foo",42);

  gfsm_alphabet_free(a);

  printf("<CHUNKS:1>--------\n");
  //g_mem_chunk_info();



  printf("<PROF:1>--------\n");
  g_mem_profile();
  g_blow_chunks();
  return 0;
}
