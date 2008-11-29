#include <gfsm.h>
#include <glib.h>
#include <stdlib.h>

#undef VERBOSE

int main (int argc, char **argv) {
  GScanner *scanner = g_scanner_new(&gfsm_automaton_scanner_config);
  GTokenType typ;

  scanner->input_name = *argv;
  g_scanner_input_file(scanner, fileno(stdin));
  while ((typ = g_scanner_get_next_token(scanner)) != G_TOKEN_EOF) {
#ifdef VERBOSE
    switch (typ) {
    case G_TOKEN_INT:
      printf("INT %ld\n", scanner->value.v_int);
      break;
    case G_TOKEN_FLOAT:
      printf("FLOAT %g\n", scanner->value.v_float);
      break;
    case G_TOKEN_CHAR:
      if (scanner->value.v_char == '\n') printf ("CHAR '\\n'\n");
      else printf("CHAR '%c'\n", scanner->value.v_char);
      break;
    default:
      printf("? (typ=%d)\n", typ);
      break;
    }
#else // !VERBOSE
    switch (typ) {
    case G_TOKEN_INT:
      printf("%ld\t", scanner->value.v_int);
      break;
    case G_TOKEN_FLOAT:
      printf("%g\t", scanner->value.v_float);
      break;
    case G_TOKEN_CHAR:
      if (scanner->value.v_char == '\n') fputc('\n',stdout);
      else exit(1);
      break;
    default:
      exit(1);
      break;
    }
#endif // VERBOSE
  }
  g_scanner_destroy(scanner);

  return 0;
}
