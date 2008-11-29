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

#include "gfsmheader_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmheader";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";

//-- global structs
gfsmAutomatonHeader hdr;

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

  //-- initialize header
  memset(&hdr, 0, sizeof(gfsmAutomatonHeader));
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
  gfsmIOHandle *ioh = NULL;
  GString   *modestr = NULL;

  GFSM_INIT

  get_my_options(argc,argv);

  //-- open file
  if (!(ioh = gfsmio_new_filename(infilename,"rb",-1,&err)) || err) {
    g_printerr("%s: open failed for '%s': %s\n", progname, infilename, err->message);
    exit(2);
  }

  //-- read header
  if (!gfsmio_read(ioh, &hdr, sizeof(gfsmAutomatonHeader))) {
    g_printerr("%s: failed to read header!\n", progname);
    exit(3);
  }
  gfsmio_close(ioh);

  //-- print header information
  printf("%-24s: %s\n", "Filename", infilename);
  printf("%-24s: %s\n", "magic", hdr.magic);
  printf("%-24s: %d.%d.%d\n", "version",
	 hdr.version.major, hdr.version.minor, hdr.version.micro);
  printf("%-24s: %d.%d.%d\n", "version_min",
	 hdr.version_min.major, hdr.version_min.minor, hdr.version_min.micro);

  printf("%-24s: %d\n", "flags.is_transducer", hdr.flags.is_transducer);
  printf("%-24s: %d\n", "flags.is_weighted", hdr.flags.is_transducer);
#if 0
  printf("%-24s: %d (%s)\n", "flags.sort_mode",
	 hdr.flags.sort_mode, gfsm_arc_sortmode_to_name(hdr.flags.sort_mode));
#else
  modestr = gfsm_acmask_to_gstring(hdr.flags.sort_mode, modestr);
  printf("%-24s: %d (%s)\n", "flags.sort_mode", hdr.flags.sort_mode, modestr->str);
  g_string_free(modestr,TRUE);
#endif
  printf("%-24s: %d\n", "flags.is_deterministic", hdr.flags.is_deterministic);
  printf("%-24s: %d\n", "flags.unused", hdr.flags.unused);

  printf("%-24s: %u\n", "root_id", hdr.root_id);
  printf("%-24s: %u\n", "n_states", hdr.n_states);
  printf("%-24s: %u\n", "n_arcs", hdr.n_arcs_007);
  printf("%-24s: %u (%s)\n", "srtype", hdr.srtype, gfsm_sr_type_to_name(hdr.srtype));

  printf("%-24s: %u\n", "unused1", hdr.unused1);
  printf("%-24s: %u\n", "unused2", hdr.unused2);
  printf("%-24s: %u\n", "unused3", hdr.unused3);

  GFSM_FINISH

  return 0;
}
