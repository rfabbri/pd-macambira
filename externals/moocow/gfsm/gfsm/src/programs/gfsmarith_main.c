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

#include "gfsmarith_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmarith";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton *fsm;
gfsmError     *err = NULL;

//-- arithmetic operation
gfsmArithOp    op=gfsmAONone;
gfsmWeight     arg=0;

//-- weight selection
gboolean       do_arcs;
gboolean       do_finals;
gboolean       do_zero;

//-- state & label selection
gfsmStateId    qid;
gfsmLabelVal    lo;
gfsmLabelVal    hi;

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

  //-- operator selection
  if      (args.exp_given)      { op = gfsmAOExp; }
  else if (args.log_given)      { op = gfsmAOLog; }
  else if (args.multiply_given) { op = gfsmAOMult; arg=args.multiply_arg; }
  else if (args.add_given)      { op = gfsmAOAdd;  arg=args.add_arg; }
  else if (args.positive_given) { op = gfsmAONoNeg; }
  else if (args.times_given)    { op = gfsmAOSRTimes; arg=args.times_arg; }
  else if (args.plus_given)     { op = gfsmAOSRPlus;  arg=args.plus_arg; }
  else if (args.sr_positive_given) { op = gfsmAOSRNoNeg; }

  //-- weight selection
  do_arcs   = !args.no_arcs_given;
  do_finals = !args.no_finals_given;
  do_zero   =  args.zero_given;

  //-- state & label selection
  qid = args.state_given ? args.state_arg : gfsmNoState;
  lo  = args.lower_given ? args.lower_arg : gfsmNoLabel;
  hi  = args.upper_given ? args.upper_arg : gfsmNoLabel;

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

  //-- hack: initial-state selection
  if (args.initial_flag) qid=fsm->root_id;

  //-- perform weight aritmetic
  gfsm_automaton_arith_state(fsm, qid, op, arg, lo, hi, do_arcs, do_finals, do_zero);

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
