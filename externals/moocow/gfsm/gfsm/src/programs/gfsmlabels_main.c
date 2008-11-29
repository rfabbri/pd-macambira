/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2005-2008 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <gfsm.h>

/*-- use gnulib --*/
#include "gnulib/getdelim.h"

#include "gfsmlabels_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmlabels";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

FILE  *outfile = NULL;

//-- global structs
gfsmAlphabet  *labels=NULL;
gfsmError     *err = NULL;
gboolean       att_mode = FALSE;
gboolean       map_mode = FALSE;
gboolean       warn_on_undef = TRUE;

/* HACK */
//extern ssize_t getline(char **LINEPTR, size_t *N, FILE *STREAM);

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void get_my_options(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  //cmdline_parser_envdefaults(&args);

  //-- filenames
  if (args.inputs_num > 0) infilename = args.inputs[0];
  outfilename = args.output_arg;

  //-- open output file
  if (args.output_given) {
    outfilename = args.output_arg;
    outfile = gfsm_open_filename(outfilename,"w",&err);
    if (!outfile) {
      g_printerr("%s: open failed for output file '%s': %s\n",
		 progname, outfilename, strerror(errno));
      exit(2);
    }
  }
  else {
    outfile = stdout;
  } 

  //-- labels
  if (args.labels_given) {
    labels = gfsm_string_alphabet_new();
    if (!gfsm_alphabet_load_filename(labels, args.labels_arg, &err)) {
      g_printerr("%s: load failed for labels file '%s': %s\n",
		 progname, args.labels_arg, err->message);
      exit(3);
    }
  } else {
    g_printerr("%s: no labels file specified!\n", progname);
    exit(3);
  }

  //-- mode flags
  att_mode = args.att_mode_flag;
  map_mode = args.map_mode_flag;
  warn_on_undef = !args.quiet_flag;
}

/*--------------------------------------------------------------------------
 * apply_labels_file()
 */
void apply_labels_file(gfsmAlphabet *labels, FILE *infile, FILE *outfile)
{
  char            *str = NULL;
  size_t           buflen = 0;
  ssize_t          linelen = 0;
  ssize_t          i;
  gfsmLabelVal     lab;
  gfsmLabelVector  *vec = g_ptr_array_new();

  while (!feof(infile)) {
    linelen = getdelim(&str,&buflen,'\n',infile);
    if (linelen<0) { break; } //-- EOF

    //-- truncate terminating newline character
    if (str[linelen-1] == '\n') { str[linelen-1] = 0; }

    //-- map mode?
    if (map_mode) { fprintf(outfile, "%s\t", str); }

    //-- convert
    vec = gfsm_alphabet_generic_string_to_labels(labels,str,vec,warn_on_undef,att_mode);

    //-- dump labels
    for (i=0; i<vec->len; i++) {
      lab = GPOINTER_TO_UINT(vec->pdata[i]);
      if (i>0) { fputc(' ',outfile); }
      fprintf(outfile, "%d", lab);
    }
    fputc('\n', outfile);
  }

  if (str) free(str);
  if (vec) g_ptr_array_free(vec,TRUE);
}

void apply_labels_file_0(gfsmAlphabet *labels, FILE *infile, FILE *outfile)
{
  char            *str = NULL;
  size_t           buflen = 0;
  ssize_t          linelen = 0;
  ssize_t          i;
  gfsmLabelVal     lab;
  char             cs[2] = {'\0', '\0'};

  while (!feof(infile)) {
    /*linelen = getline(&str,&buflen,infile);*/
    linelen = getdelim(&str,&buflen,'\n',infile);
    for (i=0; i < linelen; i++) {
      if (isspace(str[i])) continue;
      cs[0] = str[i];
      lab = gfsm_alphabet_find_label(labels,cs);

      if (lab==gfsmNoLabel) {
	g_printerr("%s: Warning: no label for character '%c' -- skipping.\n",
		   progname, cs[0]);
	continue;
      }

      fprintf(outfile, "%d ", lab);
    }
    fputs("\n", outfile);
  }

  if (str) free(str);
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  int i;

  GFSM_INIT
  get_my_options(argc,argv);

  //-- process input(s)
  if (args.inputs_num==0) {
    apply_labels_file(labels,stdin,outfile);
  }
  for (i=0; i < args.inputs_num; i++) {
    FILE *infile = (strcmp(args.inputs[i],"-")==0 ? stdin : fopen(args.inputs[i], "r"));
    if (!infile) {
      g_printerr("%s: load failed for input file '%s': %s\n", progname, args.inputs[i], strerror(errno));
      exit(255);
    }
    apply_labels_file(labels,infile,outfile);
    if (infile != stdin) fclose(infile);
  }


  //-- cleanup
  if (labels) gfsm_alphabet_free(labels);

  GFSM_FINISH
  return 0;
}
