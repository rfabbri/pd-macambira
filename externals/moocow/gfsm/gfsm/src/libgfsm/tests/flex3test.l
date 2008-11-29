/*======================================================================
 * Flex Options
 */
%option outfile="flex3test.lex.c"
%option header-file="flex3test.lex.h"
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
  T_EOF = 256,
  T_CHAR = 257,
  T_BRACKETED = 258,
  T_BSPACE = 259,
  T_WEIGHT = 260,
  T_UINT = 261,
  T_UTF8 = 262,

  T_LPAREN = '(',
  T_RPAREN = ')',
  T_LBRACKET = '[',
  T_RBRACKET = ']',
  T_LANGLE = '<',
  T_RANGLE = '>',

  T_BEQUAL = '=',

  T_OTHER = 65535
} TokenType;

#define GSTR_BUF ((GString*)((gfsmScanner*)yyextra)->data)

%}

/*======================================================================
 * Flex Definitions
 */

DIGIT    [0-9]
SPACE    [[:space:]]
WCHAR    [^\<\>\[\]\\ \t\n\r\#]
BCHAR    [^\\\]\ \t\n\r\=]
UTF8PREFIX "Ã"

%x ESCAPE BRACKETED BESCAPE COMMENT WEIGHT UINT UTF8 BUTF8

/*======================================================================
 * Rules
 */
%%

{UTF8PREFIX} { BEGIN(UTF8); g_string_set_size(GSTR_BUF,1); GSTR_BUF->str[0] = yytext[0]; }

"\\"     { BEGIN(ESCAPE); }

"#"      { BEGIN(COMMENT); }

"["      { BEGIN(BRACKETED); g_string_truncate(GSTR_BUF,0); return T_LBRACKET; }

"<"      { BEGIN(WEIGHT); return T_LANGLE; }


"("      { return '('; }
")"      { return ')'; }

"*"      { return '*'; }
"+"      { return '+'; }
"^"      { BEGIN(UINT); return '|'; }
"?"      { return '?'; }
"!"      { return '!'; }
"|"      { return '|'; }
"&"      { return '&'; }
":"      { return ':'; }
"@"      { return '@'; }
"-"      { return '-'; }

{SPACE}  { /* ignore */ }

{WCHAR}  { return T_CHAR; }



<UTF8>{UTF8PREFIX}+ { g_string_append(GSTR_BUF, yytext); }

<UTF8>.          { BEGIN(INITIAL); g_string_append_c(GSTR_BUF, yytext[0]); return T_UTF8; }


<WEIGHT>([\+\-])?({DIGIT}*\.)?({DIGIT}+)([Ee]([\+\-]?){DIGIT}+)? { return T_WEIGHT; /* hack */ }

<WEIGHT>\>       { BEGIN(INITIAL); return T_RANGLE; }



<UINT>{DIGIT}+ { BEGIN(INITIAL); return T_UINT; }


<COMMENT>[^\n]*\n  { BEGIN(INITIAL); /* ignore comments */ }




<ESCAPE>.        { BEGIN(INITIAL); return T_CHAR; }



<BRACKETED>{UTF8PREFIX} { BEGIN(BUTF8); g_string_append_c(GSTR_BUF, yytext[0]); }

<BRACKETED>\]       { BEGIN(INITIAL); return T_RBRACKET; }

<BRACKETED>{BCHAR}  { g_string_append_c(GSTR_BUF, yytext[0]); }

<BRACKETED>{SPACE}  { return T_BSPACE; }

<BRACKETED>"="      { return T_BEQUAL; }

<BRACKETED>\\       { BEGIN(BESCAPE); }


<BUTF8>{UTF8PREFIX}+ { g_string_append(GSTR_BUF, yytext); }

<BUTF8>.             { BEGIN(BRACKETED); g_string_append_c(GSTR_BUF, yytext[0]); }


<BESCAPE>.           { BEGIN(BRACKETED); g_string_append_c(GSTR_BUF, yytext[0]); }



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
  unsigned int uint;

  while ((tok=testme_yylex(scanner->yyscanner)) != T_EOF) {
    switch (tok) {
    case T_CHAR:
      printf("(char) '%s'\n", testme_yyget_text(scanner->yyscanner));
      break;

    case T_UTF8:
      printf("(utf8) '%s'\n", ((GString*)scanner->data)->str);
      break;

    case T_LBRACKET:
      printf("(lbracket)\n");
      break;

    case T_RBRACKET:
      printf("(rbracket) gstr=\"%s\"\n", ((GString*)scanner->data)->str);
      g_string_truncate(((GString*)scanner->data),0);
      break;
    case T_BSPACE:
      printf("(bspace) gstr=\"%s\"\n", ((GString*)scanner->data)->str);
      g_string_truncate(((GString*)scanner->data),0);
      break;
    case T_BEQUAL:
      printf("(bequal) gstr=\"%s\"\n", ((GString*)scanner->data)->str);
      g_string_truncate(((GString*)scanner->data),0);
      break;

    case T_LANGLE:
      printf("(langle)\n");
      break;
    case T_RANGLE:
      printf("(rangle)\n");
      break;
    case T_WEIGHT:
      weight = strtod(testme_yyget_text(scanner->yyscanner),NULL);
      printf("(weight) =%g\n", weight);
      break;

    case T_UINT:
      uint = strtol(testme_yyget_text(scanner->yyscanner),NULL,0);
      printf("(uint) =%d\n", uint);
      break;

    default:
      printf("(other=%d~'%c'): (%s)\n", tok, tok, testme_yyget_text(scanner->yyscanner));
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
