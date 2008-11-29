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
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <gfsm.h>

#include "gfsmsigma_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmsigma";

//-- options
struct gengetopt_args_info args;

//-- files
const char *abetname = "-";
const char *outfilename = "-";

//-- global structs etc.
gfsmError *err = NULL;
gfsmAutomaton *fsmOut=NULL;
gfsmAlphabet *abet=NULL;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void get_my_options(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- output
  if (args.inputs_num) abetname = args.inputs[0];
  if (args.output_arg) outfilename = args.output_arg;

  //-- initialize automaton
  fsmOut = gfsm_automaton_new();
}


/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GFSM_INIT

  get_my_options(argc,argv);

  //-- load alphabet
  abet = gfsm_string_alphabet_new();
  if (!gfsm_alphabet_load_filename(abet, abetname, &err)) {
    g_printerr("%s: load failed for alphabet file '%s': %s\n",
	       progname, abetname, err->message);
    exit(2);
  }

  //-- compute operation
  gfsm_automaton_sigma(fsmOut,abet);

  //-- spew automaton
  if (!gfsm_automaton_save_bin_filename(fsmOut,outfilename,args.compress_arg,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  if (abet)   gfsm_alphabet_free(abet);
  if (fsmOut) gfsm_automaton_free(fsmOut);

  GFSM_FINISH

  return 0;
}
