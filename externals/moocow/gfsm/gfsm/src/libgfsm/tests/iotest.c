#include <glib.h>
#include <gfsmIO.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------
 * generic test: output
 */
void test_output_generic(gfsmIOHandle *io, const char *label)
{
  gboolean rc;
  fprintf(stderr, "\n-------- I/O: %s: output\n", label);

  fprintf(stderr, "+ write(\"foo\\n\",4):\n");
  rc = gfsmio_write(io, "foo\n", 4);
  gfsmio_flush(io);
  fprintf(stderr, "  --> %d\n", rc);

  fprintf(stderr, "+ puts(\"bar\\n\"):\n");
  rc = gfsmio_puts(io, "bar\n");
  gfsmio_flush(io);
  fprintf(stderr, "  --> %d\n", rc);

  fprintf(stderr, "+ putc('x'); putc('\\n'):\n");
  rc = gfsmio_putc(io, 'x') && gfsmio_putc(io, '\n');
  gfsmio_flush(io);
  fprintf(stderr, "  --> %d\n", rc);

  fprintf(stderr, "+ printf(\"%%s%%s\\n\",\"foo\",\"bar\"):\n");
  rc = gfsmio_printf(io, "%s%s\n", "foo", "bar");
  gfsmio_flush(io);
  fprintf(stderr, "  --> %d\n", rc);
}

/*---------------------------------------------------
 * generic test: input
 */
void test_input_generic(gfsmIOHandle *io, const char *label)
{
  char buf[2];
  char *linebuf=NULL;
  size_t n=0;
  ssize_t nread=0;
  gboolean rc;

  fprintf(stderr, "\n-------- I/O: %s: input\n", label);

  fprintf(stderr, "+ read(2)\n");
  rc = gfsmio_read(io, buf, 2);
  fprintf(stderr, "  --> %d ; buf=\"%c%c\"\n", rc, buf[0], buf[1]);

  fprintf(stderr, "+ getline()\n");
  while ( (nread=gfsmio_getline(io, &linebuf, &n)) > 0) {
    fprintf(stderr, "  --> %d ; linebuf=\"%s\"\n", nread, linebuf);
    fprintf(stderr, "+ getline()\n");
  }
  fprintf(stderr, "  --> %d ; linebuf=\"%s\"\n", nread, linebuf);

  if (linebuf) free(linebuf);
}


/*---------------------------------------------------
 * test: FILE*
 */
void test_io_cfile(void) {
  gfsmIOHandle *ioh=NULL;

  //-- I/O to file: output
  ioh = gfsmio_new_file(stdout);
  test_output_generic(ioh, "FILE* (stdout)");
  gfsmio_handle_free(ioh);

  //-- I/O from file: input
  ioh = gfsmio_new_file(stdin);
  test_input_generic(ioh, "FILE* (stdin)");
  gfsmio_handle_free(ioh);
}

/*---------------------------------------------------
 * test: GString*
 */
void test_io_gstring(void) {
  GString         *gs = g_string_new("");
  gfsmPosGString  pgs = { gs, 0 };
  gfsmIOHandle   *ioh = NULL;

  //-- I/O to GString*: output
  ioh = gfsmio_new_gstring(&pgs);
  test_output_generic(ioh, "GString*");
  fprintf(stderr, "+ OUTPUT=\"%s\"\n", gs->str);

  //-- I/O from GString*: input
  pgs.pos = 0;
  /*
  g_string_assign(gs, "ab\ncde");
  test_input_generic(ioh, "GString* \"ab\\nc\")");
  */
  /*
  g_string_assign(gs, "a b c\nd e f");
  test_input_generic(ioh, "GString* \"a b c\\nd e f\")");
  */
  /*
  g_string_assign(gs, "abcde\nfghij\nklmnopqrstuvwxyz");
  test_input_generic(ioh, "GString* \"...\")");
  */
  g_string_assign(gs, "abc\n\ndef\n");
  test_input_generic(ioh, "GString* \"...\")");


  gfsmio_handle_free(ioh);
  g_string_free(gs,TRUE);
}

/*---------------------------------------------------
 * test: gzFile
 */
void test_io_zfile(void) {
  gfsmIOHandle *ioh=NULL;
  gfsmError *err=NULL;

  //-- I/O to gzGile: output
  ioh = gfsmio_new_filename("iotest-out.gz", "wb", 0, &err);
  test_output_generic(ioh, "gzFile (iotest-out.gz)");
  gfsmio_close(ioh);
  gfsmio_handle_free(ioh);

  //-- I/O from gzFile: input
  ioh = gfsmio_new_filename("iotest-in.gz", "rb", -1, &err);
  test_input_generic(ioh, "gzFile (iotest-in.gz)");
  gfsmio_close(ioh);
  gfsmio_handle_free(ioh);
}

/*---------------------------------------------------
 * test: gzFile from FILE*
 */
void test_io_zcfile(void) {
  gfsmIOHandle *ioh=NULL;
  int zlevel = -1;

  //-- I/O to gzGile: output
  ioh = gfsmio_new_zfile(stdout, "wb", zlevel);
  test_output_generic(ioh, "gzFile(fileno(stdout))");
  gfsmio_close(ioh);
  gfsmio_handle_free(ioh);

  //-- I/O from gzFile: input
  ioh = gfsmio_new_zfile(stdin, "rb", zlevel);
  test_input_generic(ioh, "gzFile(fileno(stdin))");
  gfsmio_close(ioh);
  gfsmio_handle_free(ioh);
}


/*---------------------------------------------------
 * MAIN
 */
int main (void) {
  /*
  fprintf(stderr, "\n=================================\n");
  test_io_cfile();
  */

  /*
  fprintf(stderr, "\n=================================\n");
  test_io_gstring();
  */

  /*
  fprintf(stderr, "\n=================================\n");
  test_io_zfile();
  */

  fprintf(stderr, "\n=================================\n");
  test_io_zcfile();

  return 0;
}
