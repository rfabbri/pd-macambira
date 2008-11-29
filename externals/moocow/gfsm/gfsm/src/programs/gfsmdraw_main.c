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

#include "gfsmdraw_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmdraw";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton *fsm;
gfsmAlphabet  *ilabels=NULL, *olabels=NULL, *slabels=NULL;
gfsmError *err = NULL;

typedef enum _gfsmDrawMode {
  gfsmDMNone,
  gfsmDMDot,
  gfsmDMVCG
} gfsmDrawMode;
gfsmDrawMode mode = gfsmDMDot; //-- default mode

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
  //-- labels: state
  if (args.slabels_given) {
    slabels = gfsm_string_alphabet_new();
    if (!gfsm_alphabet_load_filename(slabels,args.slabels_arg,&err)) {
      g_printerr("%s: load failed for state-labels file '%s': %s\n",
		 progname, args.slabels_arg, (err ? err->message : "?"));
      exit(2);
    }
  }

  //-- draw mode
  if (args.dot_given) mode = gfsmDMDot;
  else if (args.vcg_given) mode = gfsmDMVCG;


  //-- initialize fsm
  fsm = gfsm_automaton_new();
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  gboolean rc = FALSE;
  get_my_options(argc,argv);

  //-- load automaton
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, (err ? err->message : "?"));
    exit(3);
  }

  //-- draw automaton
  if (mode==gfsmDMDot)
    rc = gfsm_automaton_draw_dot_filename_full(fsm,
					       outfilename,
					       ilabels,
					       olabels,
					       slabels,
					       (args.title_given ? args.title_arg : infilename),
					       args.width_arg,
					       args.height_arg,
					       args.fontsize_arg,
					       args.font_arg,
					       args.portrait_given,
					       args.vertical_given,
					       args.nodesep_arg,
					       args.ranksep_arg,
					       &err);
  else if (mode==gfsmDMVCG) 
    rc = gfsm_automaton_draw_vcg_filename_full(fsm,
					       outfilename,
					       ilabels,
					       olabels,
					       slabels,
					       (args.title_given ? args.title_arg : infilename),
					       args.xspace_arg,
					       args.yspace_arg,
					       (args.vertical_given ? "top_to_bottom" : "left_to_right"),
					       args.state_shape_arg,
					       args.state_color_arg,
					       args.final_color_arg,
					       &err);

  if (!rc) {
    g_printerr("%s: store failed to '%s': %s\n",
	       progname, outfilename, (err ? err->message : "?"));
    exit(4);
  }

  //-- cleanup
  if (ilabels) gfsm_alphabet_free(ilabels);
  if (olabels) gfsm_alphabet_free(olabels);
  if (slabels) gfsm_alphabet_free(slabels);
  gfsm_automaton_free(fsm);

  return 0;
}
