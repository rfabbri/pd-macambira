/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

#include <gfsm.h>

#include "gfsmstrings_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmstrings";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";
FILE *outfile = NULL;

//-- global structs
gfsmAutomaton *fsm;
gfsmAlphabet  *ilabels=NULL, *olabels=NULL;
gfsmError *err = NULL;

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
  if (args.output_given)  outfilename = args.output_arg;

  //-- labels: input
  if (args.ilabels_given) {
    ilabels = gfsm_string_alphabet_new();
    if (!gfsm_alphabet_load_filename(ilabels,args.ilabels_arg,&err)) {
      g_printerr("%s: load failed for input-labels file '%s': %s\n",
		 progname, args.ilabels_arg, (err ? err->message : "?"));
      exit(2);
    }
  }
  //-- labels: output
  if (args.olabels_given) {
    olabels = gfsm_string_alphabet_new();
    if (!gfsm_alphabet_load_filename(olabels,args.olabels_arg,&err)) {
      g_printerr("%s: load failed for output-labels file '%s': %s\n",
		 progname, args.olabels_arg, (err ? err->message : "?"));
      exit(2);
    }
  }

  //-- initialize fsm
  fsm = gfsm_automaton_new();
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  gfsmSet *paths = NULL;
  GSList  *strings = NULL;
  get_my_options(argc,argv);

  //-- load automaton
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, (err ? err->message : "?"));
    exit(3);
  }

  //-- sanity check
  if (gfsm_automaton_is_cyclic(fsm)) {
    g_printerr("%s: input automaton must be acyclic!\n", progname);
    exit(255);
  }

  //-- open output file
  outfile = gfsm_open_filename(outfilename, "w", &err);
  if (!outfile) {
    g_printerr("%s: %s\n", progname, (err ? err->message : "?"));
    exit(4);
  }
  

  //-- get & stringify full paths
  if (args.viterbi_flag) {
    //-- serialize Viterbi trellis automaton
    paths   = gfsm_viterbi_trellis_paths_full(fsm, NULL, gfsmLSBoth);
  }
  else {
    //-- serialize "normal" automaton
    paths   = gfsm_automaton_paths_full(fsm, NULL, gfsmLSBoth);
  }
  strings = gfsm_paths_to_strings(paths,
				  ilabels,
				  olabels,
				  fsm->sr,
				  TRUE,
				  args.att_given,
				  NULL);
  while (strings) {
    //-- pop first datum
    char *s = (char *)strings->data;
    strings = g_slist_delete_link(strings,strings);

    //-- print string
    fputs(s, outfile);
    fputc('\n', outfile);

    g_free(s);
  }

  //-- cleanup
  if (paths)   gfsm_set_free(paths);
  if (ilabels) gfsm_alphabet_free(ilabels);
  if (olabels) gfsm_alphabet_free(olabels);
  if (fsm)     gfsm_automaton_free(fsm);

  if (outfile != stdout) fclose(outfile);

  return 0;
}
