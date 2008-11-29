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
#include <string.h>

#include <gfsm.h>

#include "gfsmconvert_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmconvert";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton *fsm;
gfsmError     *err = NULL;
gfsmSRType     srtype = gfsmSRTUnknown;

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

  //-- initialize fsm
  fsm = gfsm_automaton_new();
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GFSM_INIT
  get_my_options(argc,argv);

  //-- load automaton
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, err->message);
    exit(3);
  }

  //-- set flags
  if (args.transducer_given) fsm->flags.is_transducer = args.transducer_arg;
  if (args.weighted_given)   fsm->flags.is_weighted   = args.weighted_arg;

  //-- set semiring
  if (args.semiring_given) {
    srtype = gfsm_sr_name_to_type(args.semiring_arg);
    if (srtype == gfsmSRTUnknown) {
      g_printerr("%s: Warning: unknown semiring name '%s' defaults to type 'tropical'.\n",
		 progname, args.semiring_arg);
      srtype = gfsmSRTTropical;
    }
    if (srtype != fsm->sr->type) {
      gfsm_automaton_set_semiring(fsm, gfsm_semiring_new(srtype));
    }
  }

  //-- store automaton
  if (!gfsm_automaton_save_bin_filename(fsm,outfilename,args.compress_arg,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  gfsm_automaton_free(fsm);

  GFSM_FINISH
  return 0;
}
