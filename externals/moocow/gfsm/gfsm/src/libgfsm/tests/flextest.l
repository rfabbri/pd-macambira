/*======================================================================
 * Flex Options
 */
%option outfile="flextest.c"
%option header-file="flextest.h"
%option prefix="testme_yy"
%option reentrant
%option 8bit

%{
/*======================================================================
 * User C Header
 */

int nlines=0;

%}

/*======================================================================
 * Flex Definitions
 */


/*======================================================================
 * Rules
 */
%%

\n { return 1; }

<<EOF>> { return 0; }

.  { /* do nothing */; }

%%

/*======================================================================
 * User C Code
 */

int main(void) {
  yyscan_t scanner;
  int nlines=0;
  int tok;

  testme_yylex_init(&scanner); //-- initialize reentrant flex scanner

  while ((tok=testme_yylex(scanner))) {
      //printf("tok=%d\n", tok);
      nlines++;
  }

  testme_yylex_destroy(scanner);   //-- cleanup reentrant flex scanner
  
  printf("%d\n", nlines);
  return 0;
}

int testme_yywrap(yyscan_t yyscanner) { return 1; }
