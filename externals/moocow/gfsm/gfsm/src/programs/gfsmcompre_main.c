/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

#include "gfsmcompre_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmcompre";

//-- options
struct gengetopt_args_info args;

//-- files
const char *fstfilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmRegexCompiler *rec  = NULL;
gfsmAlphabet      *abet = NULL;
gfsmError         *err  = NULL;
gboolean  emit_warnings = TRUE;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void get_my_options(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  //cmdline_parser_envdefaults(&args);

  //-- sanity checks
  if (!args.regex_given) {
    g_printerr("%s: no regex specified!\n", progname);
    cmdline_parser_print_help();
    exit(-1);
  }

  //-- filenames
  outfilename = args.output_arg;

  //-- alphabet: basic labels
  abet = gfsm_string_alphabet_new();
  if (args.labels_given) {
    if (!gfsm_alphabet_load_filename(abet, args.labels_arg, &err)) {
      g_printerr("%s: load failed for labels file '%s': %s\n",
		 progname, args.labels_arg, err->message);
      exit(2);
    }
  }

  //-- options for regex compiler
  rec = gfsm_regex_compiler_new_full("gfsmRegexCompiler",
				     abet,
				     gfsm_sr_name_to_type(args.semiring_arg),
				     emit_warnings);
}


/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  gfsmAutomaton *fsm;
  char          *regex=NULL;

  GFSM_INIT
  get_my_options(argc,argv);

  //-- string-compile hack: escape everything
  if (args.string_flag) {
    GString *tmp = g_string_new("");
    int i;
    for (i=0; i < strlen(args.regex_arg); i++) {
      g_string_append_c(tmp,'\\');
      g_string_append_c(tmp,args.regex_arg[i]);
    }
    regex = tmp->str;
    g_string_free(tmp,FALSE);
  } else {
    regex = args.regex_arg;
  }

  //-- parse regex string
  gfsm_scanner_scan_string(&(rec->scanner), regex);
  fsm = gfsm_regex_compiler_parse(rec);

  //-- check for errors
  if (rec->scanner.err) {
    g_printerr("%s: %s\n", progname, err->message);
    exit(3);
  }
  if (!fsm) {
    g_printerr("%s: no automaton!\n", progname);
    exit(4);
  }

  
  //-- save output fsm
  if (!gfsm_automaton_save_bin_filename(fsm,outfilename,args.compress_arg,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(5);
  }

  //-- cleanup
  gfsm_regex_compiler_free(rec,TRUE,TRUE);


  GFSM_FINISH
  return 0;
}
