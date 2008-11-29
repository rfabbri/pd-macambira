/*======================================================================
 * Bison Options
 */
%pure_parser

%{
/*======================================================================
 * Bison C Header
 */
#include <stdio.h>
#include <stdarg.h>
#include "calctest.tab.h"
#include "calctest.lex.h"

typedef struct _yyparse_param {
  char     *name;
  char     *filename;
  yyscan_t  scanner;
  float     val;
} yyparseParam;

#define YYLEX_PARAM   ((yyparseParam*)pparam)->scanner
#define YYPARSE_PARAM pparam

extern void calctest_yyerror(const char *msg);
extern void calctest_yycarp(yyparseParam *pparams, const char *fmt, ...);

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
		{ printf("%g\n", $2); ((yyparseParam*)pparam)->val=$$=$2; }
	;

expr:		LPAREN expr RPAREN  { $$=$2; }
	|	MINUS expr          { $$=-$2; }
	|	expr TIMES expr     { $$=$1*$3; }
	|	expr DIV expr       { $$=$1/$3; }
	|	expr PLUS expr      { $$=$1+$3; }
	|	expr MINUS expr  %prec UMINUS { $$=$1-$3; } 
	|	NUMBER              { $$=$1; }
	|	OTHER
		{
		    calctest_yycarp((yyparseParam*)pparam, "Failed to parse expression");
                    YYABORT;
		}
	;

%%

/*======================================================================
 * User C Code
 */

void calctest_yyerror(const char *msg)
{
    fprintf(stderr, "yyerror: %s\n", msg);
}

void calctest_yycarp(yyparseParam *pparams, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s: ", (pparams->name ? pparams->name : "calctest_parser"));

    va_start(ap,fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, " in %s%s%s at line %u, column %u.\n",
	    (pparams->filename ? "file \"" : ""),
	    (pparams->filename ? pparams->filename : "input"),
	    (pparams->filename ? "\"" : ""),
	    calctest_yyget_lineno(pparams->scanner),
	    calctest_yyget_column(pparams->scanner));
}

int main (void) {
  yyscan_t scanner;
  yyparseParam pparams;

  calctest_yylex_init(&scanner); //-- initialize reentrant flex scanner

  pparams.name = NULL;
  pparams.filename = NULL;
  //--
  //pparams.name = "myParser";
  //pparams.filename = "(stdin)";

  pparams.scanner = scanner;

  calctest_yyparse(&pparams);

  calctest_yylex_destroy(pparams.scanner);   //-- cleanup reentrant flex scanner

  printf("Final calctest value=%g\n", pparams.val);
  
  return 0;
}
