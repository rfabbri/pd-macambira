#!/usr/bin/env python

#/* --------------------------- gendasc  ----------------------------------- */
#/*   ;; Kjetil S. Matheussen, 2004.                                             */
#/*                                                                              */
#/* This program is free software; you can redistribute it and/or                */
#/* modify it under the terms of the GNU General Public License                  */
#/* as published by the Free Software Foundation; either version 2               */
#/* of the License, or (at your option) any later version.                       */
#/*                                                                              */
#/* This program is distributed in the hope that it will be useful,              */
#/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
#/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
#/* GNU General Public License for more details.                                 */
#/*                                                                              */
#/* You should have received a copy of the GNU General Public License            */
#/* along with this program; if not, write to the Free Software                  */
#/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
#/*                                                                              */
#/* ---------------------------------------------------------------------------- */


import sys,string,xreadlines


path=sys.argv[1]
if path[-1]=="/":
    filename=path+"d_dac.c"
else:
    filename=path+"/"+"d_dac.c"


success=0
for line in xreadlines.xreadlines(open(filename,"r")):
    line=string.replace(line,'adc','from_sc')
    line=string.replace(line,'dac','to_sc')
    line=string.replace(line,"(t_newmethod)from_sc_new","(t_newmethod)from_sc_newnew")
    line=string.replace(line,"(t_newmethod)to_sc_new","(t_newmethod)to_sc_newnew")
    sys.stdout.write(line)
    if line=='#include "m_pd.h"\n':
        print 'static void *from_sc_newnew(t_symbol *s, int argc, t_atom *argv);'
        print 'static void *to_sc_newnew(t_symbol *s, int argc, t_atom *argv);'
        success=1

if success==0:
    print "Fix gendasc.py script."



