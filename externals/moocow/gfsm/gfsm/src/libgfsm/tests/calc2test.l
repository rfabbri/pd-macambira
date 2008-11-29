/*======================================================================
 * Flex Options
 */
%option outfile="calc2test.lex.c"
%option header-file="calc2test.lex.h"
%option prefix="calc2test_yy"
%option reentrant
%option 8bit
%option yylineno

%option bison-bridge

%{
/*======================================================================
 * User C Header
 */

/* bison stuff */
#include <gfsmScanner.h>
#include "calc2test.tab.h"

%}

/*======================================================================
 * Flex Definitions
 */


/*======================================================================
 * Flex Rules
 */
%%

([[:digit:]]*\.?)[[:digit:]]+  { yylval->dbl = strtod(yytext,NULL); return NUMBER;}

\( { return LPAREN; }
\) { return RPAREN; }
\+ { return PLUS; }
\- { return MINUS; }
\* { return TIMES; }
\/ { return DIV; }

\n { return NEWLINE; }

[[:space:]] { /* ignore */ }

.  { return OTHER; /* ignore */ }

%%

/*======================================================================
 * User C Code
 */

//-- wrapper: use default
GFSM_SCANNER_YYWRAP(calc2test_yy)
