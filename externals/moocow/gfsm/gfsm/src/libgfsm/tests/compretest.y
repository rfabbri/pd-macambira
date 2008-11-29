/*======================================================================
 * Bison Options
 */
%pure_parser

%{
/*======================================================================
 * Bison C Header
 */
#include <gfsmRegexCompiler.h>
#include <gfsmAutomatonIO.h>

#include "compretest.tab.h"
#include "compretest.lex.h"

#define my_compiler ((gfsmRegexCompiler*)reparser)

#define YYLEX_PARAM   ((gfsmRegexCompiler*)reparser)->scanner.yyscanner
#define YYPARSE_PARAM reparser

#define YYERROR_VERBOSE 1
#define compretest_yyerror(msg) \
  gfsm_scanner_carp((gfsmScanner*)reparser, (msg));  

%}

/*======================================================================
 * Bison Definitions
 */
%union {
   gfsmAutomaton *fsm; //-- automaton
   GString       *gs;  //-- needs to be freed by hand
   gchar          c;
   guint32        u;
   gfsmWeight     w;
}

%token <c>      TOK_UNKNOWN TOK_CHAR
%token <u>      TOK_UINT
%token <gs>     TOK_STRING
%token <w>      TOK_WEIGHT

%type  <u>      label
%type  <w>      weight
%type  <fsm>    regex

/*
empty { $$=gfsm_regex_automaton_epsilon(my_compiler); }
*/

/*
	|	gfsmRETChar %prec LAB
		{ $$=gfsm_regex_automaton_lab(my_compiler, $1); }
*/

// -- Operator precedence and associativity
%left CONCAT
%left LABCONCAT
%left WEIGHT
%right '%'             //-- non-AT&T: rmepsilon:   % REGEX
%right '$'             //-- non-AT&T: determinize: $ REGEX
%right '~'             //-- non-AT&T: connect:     ~ REGEX
%left '*' '+' '?' '^'
%right '!'
%left '@'
%left ':'
%left '-'
%left '&'
%left '|'

/*======================================================================
 * Bison Rules
 */
%%

regex:		'('regex ')'
		{ $$=$2; }

	|	label
		{ $$=gfsm_regex_compiler_label_fsm(my_compiler, $1); }

	|	label regex %prec LABCONCAT
		{ $$=gfsm_regex_compiler_prepend_lab(my_compiler, $1, $2); }

	|	regex regex %prec CONCAT
		{ $$=gfsm_regex_compiler_concat(my_compiler, $1, $2); }

	|	'%' regex
		{ $$=gfsm_regex_compiler_rmepsilon(my_compiler, $2); /* non-ATT */ }

	|	'$' regex
		{ $$=gfsm_regex_compiler_determinize(my_compiler, $2); /* non-ATT */ }

	|	'~' regex
		{ $$=gfsm_regex_compiler_connect(my_compiler, $2); /* non-ATT */ }

	|	regex '*'
		{ $$=gfsm_regex_compiler_closure(my_compiler,$1,FALSE); }

	|	regex '+'
		{ $$=gfsm_regex_compiler_closure(my_compiler,$1,TRUE); }

	|	regex '^' TOK_UINT
		{ $$=gfsm_regex_compiler_power(my_compiler,$1,$3); }

	|	regex '?'
		{ $$=gfsm_regex_compiler_optional(my_compiler,$1); }

	|	'!' regex
		{ $$=gfsm_regex_compiler_complement(my_compiler,$2); }

	|	regex '|' regex
		{ $$=gfsm_regex_compiler_union(my_compiler,$1,$3); }

	|	regex '&' regex
		{ $$=gfsm_regex_compiler_intersect(my_compiler,$1,$3); }

	|	regex ':' regex
		{ $$=gfsm_regex_compiler_product(my_compiler,$1,$3); }

	|	regex '@' regex
		{ $$=gfsm_regex_compiler_compose(my_compiler,$1,$3); }

	|	regex '-' regex
		{ $$=gfsm_regex_compiler_difference(my_compiler,$1,$3); }

	|	regex weight %prec WEIGHT
		{ $$=gfsm_regex_compiler_weight(my_compiler,$1,$2); }
		;

label:		TOK_CHAR
		{ $$=gfsm_regex_compiler_char2label(my_compiler, $1); }

	|	TOK_STRING
		{ $$=gfsm_regex_compiler_gstring2label(my_compiler, $1); }

	|	'[' TOK_STRING ']'
		{ $$=gfsm_regex_compiler_gstring2label(my_compiler, $2); }
	;

weight:		'<' TOK_WEIGHT '>' { $$=$2; }
	;

%%

/*======================================================================
 * User C Code
 */

int main (int argc, char **argv) {
  gfsmRegexCompiler *reparser = g_new0(gfsmRegexCompiler,1);
  gfsm_scanner_init((gfsmScanner*)reparser, "gfsmRegexCompiler", compretest_yy);


  //-- initialization
  reparser->srtype = gfsmSRTTropical;
  reparser->gstr   = g_string_new("");
  reparser->abet   = gfsm_string_alphabet_new();
  if (!gfsm_alphabet_load_filename(reparser->abet, "test.lab", &(reparser->scanner.err))) {
      g_printerr("%s: load failed for labels file '%s': %s\n",
		 *argv, "test.lab", (reparser->scanner.err ? reparser->scanner.err->message : "?"));
      exit(2);
  }

  //-- debug: lexer
  reparser->scanner.emit_warnings = TRUE;

  //-- parse
  compretest_yyparse(reparser);

  //-- sanity check
  if (reparser->scanner.err) {
    fprintf(stderr, "%s: %s\n", *argv, reparser->scanner.err->message);
  }

  if (reparser->fsm) {
    gfsm_automaton_save_bin_file(reparser->fsm, stdout, NULL);
  } else {
    fprintf(stderr, "%s: Error: no fsm!\n", *argv);
  }

  gfsm_scanner_free((gfsmScanner*)reparser);
 
  return 0;
}
