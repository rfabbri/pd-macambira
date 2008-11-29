#include <gfsm.h>
#include <stdlib.h>


gfsmAutomaton *fsm;
gfsmAlphabet *ialph;
const char *tfstname = "lab2ary.tfst";
gfsmError *err = NULL;


int main (int argc, char **argv) {
  guint i;
  ialph = gfsm_identity_alphabet_new();
  //GArray *ary;
  GPtrArray *ary;

  fsm = gfsm_automaton_new();
  if (!gfsm_automaton_compile_filename(fsm,tfstname,&err)) {
    g_printerr("%s: compile failed for '%s': %s\n", *argv, tfstname, err->message);
    exit(255);
  }
  printf("%s: compiled test automaton from '%s'\n", *argv, tfstname);

  ialph = gfsm_automaton_get_alphabet(fsm, gfsmLSLower, ialph);

  printf("--\n");
  printf("alphabet size=%u ; min=%u ; max=%u\n",
	 gfsm_alphabet_size(ialph), ialph->lab_min, ialph->lab_max);

  printf("--\n");
  printf("alphabet array={");
  /*-- ok
  ary = g_array_new(FALSE,FALSE,sizeof(gfsmLabelVal));
  gfsm_alphabet_labels_to_array(ialph,ary);
  */
  /*-- ok
  ary = g_array_sized_new(FALSE,FALSE,sizeof(gfsmLabelVal),gfsm_alphabet_size(ialph));
  gfsm_alphabet_labels_to_array(ialph,ary);
  */
  /*-- ok */
  //ary = gfsm_alphabet_labels_to_array(ialph,NULL);

  /*-- ptr_array */
  ary = g_ptr_array_sized_new(gfsm_alphabet_size(ialph));
  gfsm_alphabet_labels_to_array(ialph,ary);

  for (i=0; i < ary->len; i++) {
    //printf(" %u", g_array_index(ary,gfsmLabelVal,i));
    printf(" %u", (gfsmLabelVal)g_ptr_array_index(ary,i));
  }
  printf(" }\n");

  //-- cleanup
  //g_array_free(ary,TRUE);
  g_ptr_array_free(ary,TRUE);

  gfsm_automaton_free(fsm);
  gfsm_alphabet_free(ialph);

  return 0;
}

