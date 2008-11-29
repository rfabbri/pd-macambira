/*======================================================================
 * Bison Options
 */
%pure_parser

%{
/*======================================================================
 * Bison C Header
 */
#include <stdio.h>
#include <gfsmScanner.h>
#include "calc2test.tab.h"
#include "calc2test.lex.h"

typedef struct calc2testDataS {
  gfsmScanner scanner;
  double      val;
} calc2testData;

#define YYLEX_PARAM   ((calc2testData*)pparam)->scanner.yyscanner
#define YYPARSE_PARAM pparam

#define YYERROR_VERBOSE 1
#define calc2test_yyerror(msg) \
  gfsm_scanner_carp((gfsmScanner*)pparam, (msg));  

%}

/*======================================================================
 * Bison Definitions
 */
%union {
    double dbl;
}

%token <dbl> NUMBER PLUS MINUS TIMES DIV LPAREN RPAREN NEWLINE OTHER
%type  <dbl> expr exprs

%left PLUS MINUS
%left TIMES DIV
%nonassoc UMINUS

/*======================================================================
 * Bison Rules
 */
%%

exprs:	        /* empty */
		{ $$=0; }
	|	exprs expr NEWLINE
		{ printf("%g\n", $2); ((calc2testData*)pparam)->val=$$=$2; }
	;

expr:		LPAREN expr RPAREN  { $$=$2; }
	|	MINUS expr          { $$=-$2; }
	|	expr TIMES expr     { $$=$1*$3; }
	|	expr DIV expr       { $$=$1/$3; }
	|	expr PLUS expr      { $$=$1+$3; }
	|	expr MINUS expr  %prec UMINUS { $$=$1-$3; } 
	|	NUMBER              { $$=$1; }
	;

%%

/*======================================================================
 * User C Code
 */

void calc2test_yyerror_func(const char *msg)
{
    fprintf(stderr, "yyerror: %s\n", msg);
}


int main (void) {
  calc2testData *pparams = g_new0(calc2testData,1);
  gfsm_scanner_init((gfsmScanner*)pparams, "calctest2Scanner", calc2test_yy);

  calc2test_yyparse(pparams);
  if (pparams->scanner.err) {
    fprintf(stderr, "Error: %s\n", pparams->scanner.err->message);
  }

  printf("Final calctest value=%g\n", pparams->val);

  gfsm_scanner_free((gfsmScanner*)pparams);
 
  return 0;
}
