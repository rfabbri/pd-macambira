/*
	$Id: flow_objects_for_matrix.c,v 1.1 2005-10-04 02:02:13 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

#include <math.h>
#include "grid.h.fcs"

// produce an upper triangular matrix with ones on the diagonal
// will also affect any additional columns using the same row-operations

void expect_complete_matrix (P<Dim> d) {
	if (d->n!=2) RAISE("bletch");
	if (d->get(0)>d->get(1)) RAISE("argh");
}

\class GridMatrixSolve < GridObject
struct GridMatrixSolve : GridObject {
	Numop *op_sub;
	Numop *op_mul;
	Numop *op_div;
	PtrGrid matrix;
	GridMatrixSolve() {
		matrix.constrain(expect_complete_matrix);
	}
	\decl void initialize ();
	\grin 0 float
};

GRID_INPUT(GridMatrixSolve,0,matrix) {
	int n = matrix->dim->get(0); // # rows
	int m = matrix->dim->get(1); // # columns
	Pt<T> mat = (Pt<T>)*matrix;
	for (int j=0; j<n; j++) {
		op_div->map(m,mat+j*m,mat[j*m+j]);
		for (int i=j+1; i<n; i++) {
			STACK_ARRAY(T,row,m);
			COPY(row,mat+j,m);
			op_mul->map(m,row,mat[i*m+j]);
			op_sub->zip(m,mat+i*m,row);
		}
	}
	GridOutlet out(this,0,matrix->dim);
	out.send(n*m,mat);
} GRID_END

\def void initialize () {
	rb_call_super(argc,argv);
	this->op_sub = op_sub;
	this->op_mul = op_mul;
	this->op_div = op_div;
}

\classinfo { IEVAL(rself,"install '#matrix_solve',1,1"); }
\end class

void startup_flow_objects_for_matrix () {
	\startall
}
