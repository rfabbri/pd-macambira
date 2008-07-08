/*
	$Id$

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "../gridflow.h.fcs"
extern "C" {
#include <pam.h>
};

\class FormatNetPBM : Format {
	struct pam inpam, outpam;
	\grin 0
	\constructor (t_symbol *mode, string filename) {
		Format::_0_open(0,0,mode,filename);
		memset(& inpam,sizeof(pam),0);
		memset(&outpam,sizeof(pam),0);
	}
	\decl 0 bang ();
};
\def 0 bang () {
	//inpam.allocation_depth = 3;
	pnm_readpaminit(f, &inpam, /*PAM_STRUCT_SIZE(tuple_type)*/ sizeof(struct pam));
	tuple *tuplerow = pnm_allocpamrow(&inpam);
	if (inpam.depth!=3) RAISE("image has %d channels instead of 3 channels",inpam.depth);
	GridOutlet out(this,0,new Dim(inpam.height,inpam.width,inpam.depth),cast);
	uint8 buf[inpam.width*3];
	for (int i=0; i<inpam.height; i++) {
		pnm_readpamrow(&inpam, tuplerow);
		for (int j=0; j<inpam.width; j++) {
			buf[j*3+0] = tuplerow[j][0];
			buf[j*3+1] = tuplerow[j][1];
			buf[j*3+2] = tuplerow[j][2];
		}
		out.send(inpam.width*inpam.depth,buf);
	}
	pnm_freepamrow(tuplerow);
}
GRID_INLET(FormatNetPBM,0) {
	if (in->dim->n!=3) RAISE("need 3 dimensions");
	if (in->dim->v[2]!=3) RAISE("need 3 channels");
	outpam.size = sizeof(struct pam);
	outpam.len  = sizeof(struct pam);
	outpam.file = f;
	outpam.format = PPM_FORMAT;
	outpam.height = in->dim->v[0];
	outpam.width = in->dim->v[1];
	outpam.depth = in->dim->v[2];
	outpam.plainformat = false;
	outpam.maxval = 255;
	//outpam.allocation_depth = 3;
	strcpy(outpam.tuple_type,PAM_PPM_TUPLETYPE);
	pnm_writepaminit(&outpam);
	in->set_chunk(1);
} GRID_FLOW {
	tuple *tuplerow = pnm_allocpamrow(&outpam);
	int m = in->dim->v[1];
	for (int i=0; i<n; i+=in->dim->prod(1)) {
		for (int j=0; j<m; j++, data+=3) {
			tuplerow[j][0] = int(data[0]);
			tuplerow[j][1] = int(data[1]);
			tuplerow[j][2] = int(data[2]);
		}
		pnm_writepamrow(&outpam, tuplerow);
	}
	pnm_freepamrow(tuplerow);
} GRID_FINISH {
	fflush(f);
} GRID_END
/* was supposed to be "#io.netpbm" but there's backwards compat. */
\classinfo {install_format("#io.ppm",6,"ppm pgm pnm pam");}
\end class FormatNetPBM

/*FormatPPM.subclass("#io:tk",1,1) {
	install_rgrid 0
	def initialize(mode)
		@id = sprintf("x%08x",object_id)
		@filename = "/tmp/tk-#{$$}-#{@id}.ppm"
		if mode!=:out then raise "only #out" end
		super(mode,:file,@filename)
		GridFlow.gui "toplevel .#{@id}\n"
		GridFlow.gui "wm title . GridFlow/Tk\n"
		GridFlow.gui "image create photo gf#{@id} -width 320 -height 240\n"
		GridFlow.gui "pack [label .#{@id}.im -image #{@id}]\n"
	end
	def _0_rgrid_end
		super
		@stream.seek 0,IO::SEEK_SET
		GridFlow.gui "image create photo #{@id} -file #{@filename}\n"
	end
	def delete
		GridFlow.gui "destroy .#{@id}\n"
		GridFlow.gui "image delete #{@id}\n"
	end
	alias close delete
}*/

void startup_netpbm () {
	pm_init(0,0);
	\startall
}
