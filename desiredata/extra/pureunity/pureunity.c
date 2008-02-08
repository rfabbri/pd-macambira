/*
	$Id: pureunity.c,v 1.1.2.3 2007-06-28 03:21:16 matju Exp $
	PureUnity
	Copyright 2006 by Mathieu Bouchard <matju à artengine point ca>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ./COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <sys/time.h>
#include "../../src/m_pd.h"
#define ALIAS(y,x) class_addcreator((t_newmethod)getfn(m,gensym(x)),gensym(y),A_GIMME,0);

void pureunity_setup() {
	t_pd *m = &pd_objectmaker;
	ALIAS( "inlet.f","inlet"  );
	ALIAS( "inlet.#","inlet"  );
	ALIAS( "inlet.~","inlet~" );
	ALIAS("outlet.f","outlet" );
	ALIAS("outlet.#","outlet" );
	ALIAS("outlet.~","outlet~");
	ALIAS(  "f.swap","swap"   );
}

