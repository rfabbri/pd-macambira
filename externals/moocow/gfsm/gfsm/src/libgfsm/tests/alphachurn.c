#include <gfsmAlphabet.h>
#include <stdlib.h>

int main (void) {
  gfsmStringAlphabet *a = gfsm_string_alphabet_new();

  gfsm_string_alphabet_load_filename(a,"test.lab");
  gfsm_string_alphabet_save_filename(a,"-");

  return 0;
}
