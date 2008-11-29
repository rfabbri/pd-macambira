#include <gfsm.h>

gfsmAlphabet *alph;

int main (int argc, char **argv)
{
  char *infilename = (argc > 1 ? argv[1] : "-");

  g_mem_set_vtable(glib_mem_profiler_table);

  alph = gfsm_string_alphabet_new();
  //g_mem_profile();

  gfsm_alphabet_load_filename(alph,infilename,NULL);
  //gfsm_alphabet_save_file(alph,stdout,NULL);


  gfsm_alphabet_free(alph);

  g_blow_chunks();
  g_mem_profile();
  return 0;
}
