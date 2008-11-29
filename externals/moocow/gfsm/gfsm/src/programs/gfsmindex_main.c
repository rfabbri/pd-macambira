/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2007 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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

#include <gfsm.h>

#include "gfsmindex_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmindex";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton         *fsm=NULL;
gfsmIndexedAutomaton *xfsm=NULL;
gfsmError             *err=NULL;

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
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GFSM_INIT
  get_my_options(argc,argv);

  //-- dispatch
  if (args.unindex_given) {
    //-- convert indexed --> vanilla

    //-- load index
    xfsm = gfsm_indexed_automaton_new();
    if (!gfsm_indexed_automaton_load_bin_filename(xfsm,infilename,&err)) {
      g_printerr("%s: load failed for indexed automaton from '%s': %s\n", progname, infilename,
		 (err ? err->message : "?"));
      exit(3);
    }

    //-- unindex
    fsm = gfsm_indexed_to_automaton(xfsm,NULL);

    //-- store vanilla
    if (!gfsm_automaton_save_bin_filename(fsm,outfilename,args.compress_arg,&err)) {
      g_printerr("%s: store failed for vanilla automaton to '%s': %s\n", progname, outfilename,
		 (err ? err->message : "?"));
      exit(4);
    }
  }
  else {
    //-- convert vanilla --> indexed

    //-- load vanilla
    fsm = gfsm_automaton_new();
    if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
      g_printerr("%s: load failed for vanilla automaton from '%s': %s\n", progname, infilename,
		 (err ? err->message : "?"));
      exit(3);
    }

    //-- index & sort
    xfsm = gfsm_automaton_to_indexed(fsm,NULL);
    //gfsm_indexed_automaton_sort(xfsm, gfsm_acmask_from_args(gfsmACLower,gfsmACWeight)); //-- TODO: make these options!

    //-- store indexed
    if (!gfsm_indexed_automaton_save_bin_filename(xfsm,outfilename,args.compress_arg,&err)) {
      g_printerr("%s: store failed for indexed automaton to '%s': %s\n", progname, outfilename,
		 (err ? err->message : "?"));
      exit(4);
    }
  }

  //-- cleanup
  if (fsm)  gfsm_automaton_free(fsm);
  if (xfsm) gfsm_indexed_automaton_free(xfsm);

  GFSM_FINISH
  return 0;
}
