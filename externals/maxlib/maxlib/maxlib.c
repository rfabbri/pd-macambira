/* --------------------------  maxlib  ---------------------------------------- */
/*                                                                              */
/* maxlib :: music analysis extensions library.                                 */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */
#ifndef VERSION
#define VERSION "1.2"
#endif

#include "m_pd.h"


#ifndef __DATE__ 
#define __DATE__ "without using a gnu compiler"
#endif

typedef struct _maxlib
{
     t_object x_obj;
} t_maxlib;

static t_class* maxlib_class;

	/* objects */
void arbran_setup();
void average_setup();
void beat_setup();
void beta_setup();
void bilex_setup();
void borax_setup();
void cauchy_setup();
void chord_setup();
void delta_setup();
void dist_setup();
void divide_setup();
void divmod_setup();
void edge_setup();
void expo_setup();
void fifo_setup();
void gauss_setup();
void gestalt_setup();
void history_setup();
void ignore_setup();
void iso_setup();
void lifo_setup();
void limit_setup();
void linear_setup();
void listfunnel_setup();
void match_setup();
void minus_setup();
void mlife_setup();
void multi_setup();
void netclient_setup();
void netdist_setup();
void netrec_setup();
void netserver_setup();
void nroute_setup();
void pitch_setup();
void plus_setup();
void poisson_setup();
void pong_setup();
void pulse_setup();
void remote_setup();
void rewrap_setup();
void rhythm_setup();
void scale_setup();
void score_setup();
void speedlim_setup();
void split_setup();
void step_setup();
void subst_setup();
void temperature_setup();
void tilt_setup();
void timebang_setup();
void triang_setup();
void unroute_setup();
void urn_setup();
void velocity_setup();
void weibull_setup();
void wrap_setup();

static void* maxlib_new(t_symbol* s)
{
    t_maxlib *x = (t_maxlib *)pd_new(maxlib_class);
    return (x);
}

void maxlib_setup(void) 
{
	maxlib_class = class_new(gensym("maxlib"), (t_newmethod)maxlib_new, 0,
    	sizeof(t_maxlib), 0,0);

	arbran_setup();
	average_setup();
	beat_setup();
	beta_setup();
	bilex_setup();
	borax_setup();
	cauchy_setup();
	chord_setup();
	delta_setup();
	dist_setup();
	divide_setup();
	divmod_setup();
	edge_setup();
	expo_setup();
	fifo_setup();
	gauss_setup();
	gestalt_setup();
	history_setup();
	ignore_setup();
	iso_setup();
	lifo_setup();
	limit_setup();
	linear_setup();
	listfunnel_setup();
	match_setup();
	minus_setup();
    mlife_setup();
	multi_setup();
	netclient_setup();
	netdist_setup();
	netrec_setup();
	netserver_setup();
	nroute_setup();
	pitch_setup();
	plus_setup();
	poisson_setup();
	pong_setup();
	pulse_setup();
	remote_setup();
	rewrap_setup();
	rhythm_setup();
	scale_setup();
	score_setup();
	speedlim_setup();
	split_setup();
	step_setup();
    subst_setup();
	temperature_setup();
	tilt_setup();
	timebang_setup();
	triang_setup();
	unroute_setup();
	urn_setup();
	velocity_setup();
	weibull_setup();
	wrap_setup();

	post("\n       maxlib :: Music Analysis eXtensions LIBrary");
	post("       written by Olaf Matthes <olaf.matthes@gmx.de>");
	post("       version "VERSION);
	post("       compiled "__DATE__);
	post("       latest version at http://www.akustische-kunst.org/puredata/maxlib/");
	post("       objects: arbran average beat beta bilex borax cauchy chord delta dist ");
	post("                divide divmod edge expo fifo gauss gestalt history ignore iso ");
	post("                lifo linear listfunnel match minus mlife multi netclient ");
	post("                netdist netrec netserver nroute pitch plus poisson pong pulse ");
	post("                remote rewrap rhythm scale score speedlim split step subst ");
	post("                temperature tilt timebang triang unroute urn velocity weibull ");
	post("                wrap\n");
}
