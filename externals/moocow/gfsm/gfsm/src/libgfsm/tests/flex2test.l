/*======================================================================
 * Flex Options
 */
%option outfile="flex2test.lex.c"
%option header-file="flex2test.lex.h"
%option prefix="testme_yy"
%option reentrant
%option 8bit
%option yylineno

%{
/*======================================================================
 * User C Header
 */

#include <gfsm.h>

typedef enum {
  T_EOF,
  T_CHAR,
  T_STRING,
  T_WEIGHT,

  T_OP_PROJ_1,
  T_OP_PROJ_2,
  T_OP_STAR,
  T_OP_PLUS,
  T_OP_POWER, //-- "^n"
  T_OP_MINUS,
  T_OP_OPTIONAL,
  T_OP_UNION,
  T_OP_COMPLEMENT,
  T_OP_INTERSECT,
  T_OP_PRODUCT,
  T_OP_COMPOSE,

  T_LPAREN,
  T_RPAREN,
  T_LBRACKET,
  T_RBRACKET,

  T_EQUAL,
  T_SEMICOLON, //-- semicolon

  T_OTHER
} TokenType;

#define GSTR_BUF ((GString*)((gfsmScanner*)yyextra)->data)

%}

/*======================================================================
 * Flex Definitions
 */

DIGIT    [0-9]
SPACE    [[:space:]]
WCHAR    [^\\\[ \t\n\r\#\<]
SCHAR    [^\\\]]

%x ESCAPE STRING SESCAPE COMMENT WEIGHT

/*======================================================================
 * Rules
 */
%%

<INITIAL>{WCHAR} { return T_CHAR; }

<INITIAL>{SPACE} { /* ignore */ }

<INITIAL>#       { BEGIN(COMMENT); }

<INITIAL>\\      { BEGIN(ESCAPE); }

<INITIAL>\[      { BEGIN(STRING); GSTR_BUF->len = 0; }

<INITIAL>\<      { BEGIN(WEIGHT); }



<WEIGHT>[^\>]+   { return WEIGHT; /* hack */ }

<WEIGHT>\>       { BEGIN(INITIAL); }



<COMMENT>[^\n]*\n  { BEGIN(INITIAL); }




<ESCAPE>.        { BEGIN(INITIAL); return T_CHAR; }




<STRING>\]       { BEGIN(INITIAL); return T_STRING; }

<STRING>{SCHAR}  { g_string_append_c(GSTR_BUF, yytext[0]); }

<STRING>\\       { BEGIN(SESCAPE); }



<SESCAPE>.       { BEGIN(STRING); g_string_append_c(GSTR_BUF, yytext[0]); }



<<EOF>>          { return T_EOF; }


<*>. {
  gfsm_scanner_carp(yyextra, "bad character '%s'", yytext);
  return T_OTHER;
}

%%

/*======================================================================
 * User C Code
 */

void testme(gfsmScanner *scanner) {
  TokenType tok;
  double weight;

  while ((tok=testme_yylex(scanner->yyscanner)) != T_EOF) {
    switch (tok) {
    case T_CHAR:
      printf("(char) '%s'\n", testme_yyget_text(scanner->yyscanner));
      break;
    case STRING:
      printf("(string) \"%s\"\n", ((GString*)scanner->data)->str);
      break;
    case WEIGHT:
      weight = strtod(testme_yyget_text(scanner->yyscanner),NULL);
      printf("(weight) %g\n", weight);
      break;
    default:
      printf("(other=%d): (%s)\n", tok, testme_yyget_text(scanner->yyscanner));
      break;
    }

    if (scanner->err) {
      fprintf(stderr, "Error: %s\n", scanner->err->message);
      g_clear_error(&(scanner->err));
      break;
    }
  }
}

int main(void) {
  gfsmScanner *scanner = gfsm_scanner_new("myScanner",testme_yy);
  scanner->data = g_string_new("");

  //-- first, scan a string
  /*
  gfsm_scanner_scan_string(scanner, "line 1\nline 2\nline 3.");
  scanner->filename = g_strdup("string");
  testme(scanner);
  */

  //-- now scan stdin
  gfsm_scanner_scan_filename(scanner, "-");
  testme(scanner);

  gfsm_scanner_free(scanner);
  
  return 0;
}

//int testme_yywrap(yyscan_t yyscanner) { return 1; }
GFSM_SCANNER_YYWRAP(testme_yy)
