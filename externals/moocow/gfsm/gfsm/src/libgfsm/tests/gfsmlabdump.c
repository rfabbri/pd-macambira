#include <gfsm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv) {
  gfsmStringAlphabet *sa = (gfsmStringAlphabet*)gfsm_string_alphabet_new();
  gfsmAlphabet        *a = (gfsmAlphabet*)sa;
  gfsmError          *err = NULL;
  char               *filename= NULL;
  gfsmLabelVal lv1,lv2;
  char *key;

  /*-- test: insert --*/
  printf("testing insert: ");
  lv1 = gfsm_alphabet_insert(a,"foo",gfsmNoLabel);
  printf("%s\n", lv1 != gfsmNoLabel ? "ok" : "FAILED");

  /*-- test: find --*/
  printf("testing find_label: ");
  lv2 = gfsm_alphabet_find_label(a,"foo");
  printf("%s\n", lv1==lv2 ? "ok" : "FAILED");

  /*-- test: find key --*/
  printf("testing find_key: ");
  key = gfsm_alphabet_find_key(a,lv1);
  printf("%s\n", key != NULL && strcmp(key,"foo")==0 ? "ok" : "FAILED");

  /*-- clear test --*/
  printf("testing clear: ");
  gfsm_alphabet_clear(a);
  printf("%s\n", gfsm_alphabet_size(a)==0 ? "ok" : "FAILED");

  /*-- load labels file --*/
  if (argc > 0) { filename=argv[1]; }
  else { filename="-"; }
  printf("\nLoading alphabet from file %s: ", argc==0 ? "(stdin)" : filename);
  if (!a || !gfsm_alphabet_load_filename(a, filename, &err)) {
    g_printerr("couldn't load labels from stdin: %s\n", err->message);
    exit(1);
  }
  printf("loaded.\n\n");

  /*-- get some basic information --*/
  printf("Basic Information:\n");
  printf(" + gfsmAlphabet:\n");
  printf("    type   : %u (%s)\n", a->type,
	 (a->type==gfsmATString ? "string keys" : "weird type: tell moocow"));
  printf("    lab_min: %u\n", a->lab_min);
  printf("    lab_max: %u\n", a->lab_max);
  printf(" + gfsmPointerAlphabet:\n");
  printf("    labels2keys: %p [size=%u]\n",
	 sa->labels2keys, sa->labels2keys->len);
  printf("    keys2labels: %p [size=%u]\n",
	 sa->keys2labels, g_hash_table_size(sa->keys2labels));
  printf("    keydupfunc : %p (%s)\n",
	 sa->key_dup_func, (sa->key_dup_func == NULL
			    ? "no key copying: tell moocow"
			    : ((void*)sa->key_dup_func == (void*)gfsm_alphabet_strdup
			       ? "keys are copied: ok"
			       : "strangeness: tell moocow")));
  

  /*-- dump it --*/
  printf("\n--BEGIN dump--\n");
  if (!gfsm_alphabet_save_file(a,stdout,&err)) {
    g_printerr("couldn't save labels to stdout: %s\n", err->message);
    exit(1);
  }
  printf("--END dump--\n\n");

  return 0;
}
