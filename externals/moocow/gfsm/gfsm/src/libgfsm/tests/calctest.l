/*======================================================================
 * Flex Options
 */
%option outfile="calctest.lex.c"
%option header-file="calctest.lex.h"
%option prefix="calctest_yy"
%option reentrant
%option 8bit

%option bison-bridge

%{
/*======================================================================
 * User C Header
 */

/* bison stuff */
#include "calctest.tab.h"

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

.  { return OTHER; /* ignore */ }

%%

/*======================================================================
 * User C Code
 */

//-- wrapper: don't
int calctest_yywrap(yyscan_t yyscanner) { return 1; }
