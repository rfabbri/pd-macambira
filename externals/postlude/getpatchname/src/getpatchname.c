
/* getpatchname - Returns the filename of the current patch 
 * 
 * Copyright (C) 2006 Jamie Bullock 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "m_pd.h"
#include "g_canvas.h"

static t_class *getpatchname_class;

typedef struct _getpatchname {
  t_object  x_obj;
  t_symbol *patch_name;
  t_outlet *outlet;
} t_getpatchname;

void getpatchname_bang(t_getpatchname *x)
{
/* At some point we need to be to get the new patch name if it changes, couldn't make this work though */
    outlet_symbol(x->outlet, x->patch_name);
}

void *getpatchname_new(void)
{
  t_getpatchname *x = (t_getpatchname *)pd_new(getpatchname_class);
  x->patch_name = canvas_getcurrent()->gl_name;
  x->outlet = outlet_new(&x->x_obj, &s_symbol);
  return (void *)x;
}

void getpatchname_setup(void) {
  getpatchname_class = class_new(gensym("getpatchname"),
        (t_newmethod)getpatchname_new,
        0, sizeof(t_getpatchname),
        CLASS_DEFAULT, 0);
  class_addbang(getpatchname_class, getpatchname_bang);
}


