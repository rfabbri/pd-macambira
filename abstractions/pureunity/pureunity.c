/*
	$Id: pureunity.c,v 1.4 2006-01-06 20:19:28 matju Exp $
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
/*#include <m_pd.h>*/
#include "../../pd/src/m_pd.h"
#define ALIAS(y,x) class_addcreator((t_newmethod)getfn(m,gensym(x)),gensym(y),A_GIMME,0);

typedef struct {
	t_text o;
	struct timeval t0;
} t_rtimer;

t_class *rtimer_class;
void rtimer_reset(t_rtimer *self) {gettimeofday(&self->t0,0);}
void *rtimer_new(t_symbol *s) {
	t_rtimer *self = (t_rtimer *)pd_new(rtimer_class);
	inlet_new((t_text *)self, (t_pd *)self, gensym("bang"), gensym("1_bang"));
	outlet_new((t_text *)self, gensym("float"));
	rtimer_reset(self);
	return self;
}

void rtimer_1_bang(t_rtimer *self) {
	struct timeval t1;
	gettimeofday(&t1,0);
	outlet_float(self->o.ob_outlet,
		(t1.tv_sec -self->t0.tv_sec )*1000.0 +
		(t1.tv_usec-self->t0.tv_usec)/1000.0);
}

void pureunity_setup() {
	t_pd *m = &pd_objectmaker;
	ALIAS( "f.inlet","inlet"  );
	ALIAS( "#.inlet","inlet"  );
	ALIAS( "~.inlet","inlet~" );
	ALIAS("f.outlet","outlet" );
	ALIAS("#.outlet","outlet" );
	ALIAS("~.outlet","outlet~");
	ALIAS(  "f.swap","swap"   );
	rtimer_class = class_new(gensym("rtimer"),(t_newmethod)rtimer_new,0,sizeof(t_rtimer),0,0);
	class_addbang(rtimer_class,rtimer_reset);
	class_addmethod(rtimer_class, (t_method)rtimer_1_bang, gensym("1_bang"), 0);
}

