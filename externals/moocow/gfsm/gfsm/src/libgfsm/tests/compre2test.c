#include <gfsmRegexCompiler.h>
#include <gfsmAutomatonIO.h>


/*======================================================================
 * User C Code
 */

int main (int argc, char **argv) {
  gfsmRegexCompiler *rec = gfsm_regex_compiler_new();
  gfsmAutomaton     *fsm = NULL;

  //-- initialization
  rec->abet   = gfsm_string_alphabet_new();
  if (!gfsm_alphabet_load_filename(rec->abet, "test.lab", &(rec->scanner.err))) {
      g_printerr("%s: load failed for labels file '%s': %s\n",
		 *argv, "test.lab", (rec->scanner.err ? rec->scanner.err->message : "?"));
      exit(2);
  }

  //-- debug: lexer
  rec->scanner.emit_warnings = TRUE;

  //-- parse
  fsm = gfsm_regex_compiler_parse(rec);

  //-- sanity check
  if (rec->scanner.err) {
    fprintf(stderr, "%s: %s\n", *argv, rec->scanner.err->message);
  }

  if (fsm) {
    gfsm_automaton_save_bin_file(fsm, stdout, NULL);
  } else {
    fprintf(stderr, "%s: Error: no fsm!\n", *argv);
  }

  gfsm_regex_compiler_free(rec,TRUE,TRUE);
 
  return 0;
}
