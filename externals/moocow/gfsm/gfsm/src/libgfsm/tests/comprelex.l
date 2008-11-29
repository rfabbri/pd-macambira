/*======================================================================
 * Flex Options
 */
%option outfile="comprelex.lex.c"
%option header-file="comprelex.lex.h"
%option prefix="compretest_yy"
%option reentrant
%option 8bit
%option yylineno

%{
/*======================================================================
 * User C Header
 */

#include <gfsmRegexCompiler.h>
//#include "compretest.tab.h"

#define my_compiler ((gfsmRegexCompiler*)yyextra)
#define my_gstr     (my_compiler->gstr)
#define my_abet     (my_compiler->abet)

typedef enum {
  TOK_UNKNOWN =256,
  TOK_CHAR    =257,
  TOK_STRING  =258,
  TOK_WEIGHT  =259,
  TOK_UINT    =260,
  TOK_BRACKETED =261,
  TOK_EOF       =262
} TokenType;

%}

/*======================================================================
 * Flex Definitions
 */

DIGIT    [0-9]
SPACE    [[:space:]]
WCHAR    [^\<\[\\ \t\n\r\#]
BCHAR    [^\\\]\ \t\n\r\=]
UTF8PREFIX "Ã"

%x STATE_ESCAPE
%x STATE_BRACKETED
%x STATE_BRACKETED_SEP
%x STATE_BRACKETED_ESCAPE
%x STATE_COMMENT
%x STATE_WEIGHT
%x STATE_UINT

/*======================================================================
 * Rules
 */
%%

{UTF8PREFIX}+. {
  g_string_truncate(my_gstr,0);
  g_string_append_len(my_gstr,yytext,yyleng);
  return TOK_STRING;
}

"\\"     { BEGIN(STATE_ESCAPE); }

"#"      { BEGIN(STATE_COMMENT); }

"["      { 
  BEGIN(STATE_BRACKETED);
  return '[';
}

"<"      { BEGIN(STATE_WEIGHT); return '<'; }

"("      { return '('; }
")"      { return ')'; }

"*"      { return '*'; }
"+"      { return '+'; }
"^"      { BEGIN(STATE_UINT); return '|'; }
"?"      { return '?'; }
"!"      { return '!'; }
"|"      { return '|'; }
"&"      { return '&'; }
":"      { return ':'; }
"@"      { return '@'; }
"-"      { return '-';  /*"/1"{return PROJ1;}*/ /*"/2"{return PROJ2;}*/ }

"$"      { return '$'; /* non-AT&T: determinize */ }
"%"      { return '%'; /* non-AT&T: rmepsilon */ }
"~"      { return '~'; /* non-AT&T: connect */ }

{SPACE}  { /* ignore */ }

{WCHAR}  {
  //yylval->u = gfsm_alphabet_find_label(my_abet, yytext);
  return TOK_CHAR;
}



<STATE_WEIGHT>([\+\-])?({DIGIT}*\.)?({DIGIT}+)([Ee]([\+\-]?){DIGIT}+)? {
  //yylval->w = strtod(yytext,NULL);
  return TOK_WEIGHT;
}

<STATE_WEIGHT>\> {
  BEGIN(INITIAL);
  return '>';
}



<STATE_UINT>{DIGIT}+ {
  BEGIN(INITIAL);
  //yylval->u = strtol(yytext,NULL,0);
  return TOK_UINT;
}


<STATE_COMMENT>[^\n]*\n  { BEGIN(INITIAL); /* ignore comments */ }




<STATE_ESCAPE>. {
  BEGIN(INITIAL);
  return TOK_CHAR;
}



<STATE_BRACKETED>{UTF8PREFIX}+. {
  g_string_append_len(my_gstr,yytext,yyleng);
}
<STATE_BRACKETED>{SPACE}*"]" {
  unput(']');
  BEGIN(STATE_BRACKETED_SEP);
  if (my_gstr->len) return TOK_STRING;
}
<STATE_BRACKETED>{BCHAR}+ {
  g_string_append(my_gstr, yytext);
  //return TOK_STRING;
}
<STATE_BRACKETED>({SPACE}*)"="({SPACE}*) {
  unput('=');
  BEGIN(STATE_BRACKETED_SEP);
  if (my_gstr->len) return TOK_STRING;
}
<STATE_BRACKETED>{SPACE}+ {
  unput(' ');
  BEGIN(STATE_BRACKETED_SEP);
  if (my_gstr->len) return TOK_STRING;
}
<STATE_BRACKETED>\\ {
  BEGIN(STATE_BRACKETED_ESCAPE);
}


<STATE_BRACKETED_SEP>"]" {
  BEGIN(INITIAL);
  if (my_gstr->len) {
    g_string_truncate(my_gstr,0);
    return ']';
  }
}
<STATE_BRACKETED_SEP>"=" {
  BEGIN(STATE_BRACKETED);
  g_string_truncate(my_gstr,0);
  return '=';
}
<STATE_BRACKETED_SEP>" " {
  BEGIN(STATE_BRACKETED);
  if (my_gstr->len) {
    g_string_truncate(my_gstr,0);
    return ' ';
  }
}



<STATE_BRACKETED_ESCAPE>. {
  BEGIN(STATE_BRACKETED);
  g_string_append_c(my_gstr, yytext[0]);
  //return TOK_STRING;
}



<*>. {
  gfsm_scanner_carp(yyextra, "bad character '%s'", yytext);
  return TOK_UNKNOWN;
}

<<EOF>> { return TOK_EOF; }

%%

/*======================================================================
 * User C Code
 */

void testme(gfsmScanner *scanner) {
  TokenType tok;
  double weight;
  unsigned int uint;

  while ((tok=compretest_yylex(scanner->yyscanner)) != TOK_EOF) {
    switch (tok) {
    case TOK_CHAR:
      printf("(char) '%s'\n", compretest_yyget_text(scanner->yyscanner));
      break;

    case TOK_BRACKETED:
      printf("(bracketed) text=\"%s\" gstr=\"%s\"\n",
	     compretest_yyget_text(scanner->yyscanner),
	     ((gfsmRegexCompiler*)scanner)->gstr->str);
      break;

    case TOK_STRING:
      printf("(string) gstr=\"%s\"\n",
	     //compretest_yyget_text(scanner->yyscanner),
	     ((gfsmRegexCompiler*)scanner)->gstr->str
	     );
      break;

    case '[':
      printf("(left-bracket) text=\"%s\"\n", compretest_yyget_text(scanner->yyscanner));
      break;

    case ']':
      printf("(right-bracket) text=\"%s\"\n", compretest_yyget_text(scanner->yyscanner));
      break;

    case ' ':
      printf("(space) text=\"%s\"\n", compretest_yyget_text(scanner->yyscanner));
      break;

    case '=':
      printf("(equal) text=\"%s\"\n", compretest_yyget_text(scanner->yyscanner));
      break;

    case TOK_WEIGHT:
      weight = strtod(compretest_yyget_text(scanner->yyscanner),NULL);
      printf("(weight) =%g\n", weight);
      break;

    case TOK_UINT:
      uint = strtol(compretest_yyget_text(scanner->yyscanner),NULL,0);
      printf("(uint) =%d\n", uint);
      break;

    default:
      printf("(other=%d~'%c'): (%s)\n", tok, tok, compretest_yyget_text(scanner->yyscanner));
      break;
    }

    if (scanner->err) {
      fprintf(stderr, "Error: %s\n", scanner->err->message);
      g_clear_error(&(scanner->err));
      break;
    }
  }
}

int main(int argc,char **argv) {
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

  //-- lex
  testme((gfsmScanner*)reparser);

  //-- sanity check
  if (reparser->scanner.err) {
    fprintf(stderr, "Error: %s\n", reparser->scanner.err->message);
  }

  gfsm_scanner_free((gfsmScanner*)reparser);
 
  return 0;
}


GFSM_SCANNER_YYWRAP(compretest_yy)
