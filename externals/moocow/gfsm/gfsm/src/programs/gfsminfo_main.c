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
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <gfsm.h>

#include "gfsminfo_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsminfo";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";

//-- global structs
gfsmAutomaton *fsm;

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

  //-- initialize fsm
  fsm = gfsm_automaton_new();
}

/*--------------------------------------------------------------------------
 * Utilities
 *--------------------------------------------------------------------------*/
#define bool2char(b) (b ? 'y' : 'n')

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  gfsmError *err = NULL;
  GString   *modestr = NULL;

  GFSM_INIT

  get_my_options(argc,argv);
  guint n_eps_i, n_eps_o, n_eps_io;

  //-- load automaton
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, err->message);
    exit(2);
  }

  //-- print information
  printf("%-24s: %s\n", "Filename", infilename);
  printf("%-24s: %s\n", "Semiring", gfsm_sr_type_to_name(fsm->sr->type));
  printf("%-24s: %c\n", "Transducer?", bool2char(gfsm_automaton_is_transducer(fsm)));
  printf("%-24s: %c\n", "Weighted?", bool2char(gfsm_automaton_is_weighted(fsm)));
  printf("%-24s: %c\n", "Deterministic?", bool2char(fsm->flags.is_deterministic));
#if 0
  printf("%-24s: %s\n", "Sort Mode", gfsm_arc_sortmode_to_name(gfsm_automaton_sortmode(fsm)));
#else
  modestr = gfsm_acmask_to_gstring(fsm->flags.sort_mode, modestr);
  printf("%-24s: %s\n", "Sort Mode", modestr->str);
  g_string_free(modestr,TRUE);
#endif
  if (fsm->root_id != gfsmNoState) {
    printf("%-24s: %u\n", "Initial state", fsm->root_id);
  } else {
    printf("%-24s: %s\n", "Initial state", "none");
  }
  printf("%-24s: %u\n", "# of states", gfsm_automaton_n_states(fsm));
  printf("%-24s: %u\n", "# of final states", gfsm_automaton_n_final_states(fsm));
  printf("%-24s: %u\n", "# of arcs", gfsm_automaton_n_arcs_full(fsm, &n_eps_i, &n_eps_o, &n_eps_io));
  printf("%-24s: %u\n", "# of i/o epsilon arcs", n_eps_io);
  printf("%-24s: %u\n", "# of input epsilon arcs", n_eps_i);
  printf("%-24s: %u\n", "# of output epsilon arcs", n_eps_o);

  printf("%-24s: %c\n", "cyclic?", bool2char(gfsm_automaton_is_cyclic(fsm)));
  //...

  //-- cleanup
  gfsm_automaton_free(fsm);

  GFSM_FINISH

  return 0;
}
