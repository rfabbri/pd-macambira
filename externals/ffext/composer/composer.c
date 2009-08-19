/* ------------------------------------------------------------------------ */
/* Copyright (c) 2009 Federico Ferri.                                       */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.           */
/*                                                                          */
/* composer: a music composition framework for pure-data                    */
/*                                                                          */
/* This program is free software; you can redistribute it and/or            */
/* modify it under the terms of the GNU General Public License              */
/* as published by the Free Software Foundation; either version 2           */
/* of the License, or (at your option) any later version.                   */
/*                                                                          */
/* See file LICENSE for further informations on licensing terms.            */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program; if not, write to the Free Software Foundation,  */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.          */
/*                                                                          */
/* Based on PureData by Miller Puckette and others.                         */
/* ------------------------------------------------------------------------ */

#include "common.h"

#include "song.c"
#include "track.c"
#include "pattern.c"
#include "song_proxy.c"
#include "track_proxy.c"

ArrayListDeclare(songs, t_song*, int);

void list_snconvf(char *buf, size_t bufsz, t_symbol* s, size_t argc, t_atom* argv) {
    static const unsigned int tmpsz = 80;
    char* tmp;
    size_t i,j,len;
    if(bufsz < 1) goto list_snconvf_szbug;
    buf[0] = '\0';
    len = 0;
    if(s != gensym("")) {
        len += strlen(s->s_name) + 1;
        if(bufsz <= len) goto list_snconvf_szbug;
        strncat(buf, s->s_name, bufsz);
        strncat(buf, " ", bufsz);
    }
    for(i = 0; i < argc; i++) {
        if(i > 0) {
            len += 1;
            if(bufsz <= len) goto list_snconvf_szbug;
            strncat(buf, " ", bufsz);
        }
        if(argv[i].a_type == A_FLOAT) {
            tmp = (char*)getbytes(tmpsz*sizeof(char));
            if(argv[i].a_w.w_float == (t_float)(t_int)argv[i].a_w.w_float)
                sprintf(tmp, "%ld", (t_int)argv[i].a_w.w_float);
            else
                sprintf(tmp, "%f", argv[i].a_w.w_float);
            len += strlen(tmp);
            if(bufsz <= len) goto list_snconvf_szbug;
            strncat(buf, tmp, bufsz);
            freebytes(tmp, tmpsz*sizeof(char));
        } else if(argv[i].a_type == A_SYMBOL) {
            len += strlen(argv[i].a_w.w_symbol->s_name);
            if(bufsz <= len) goto list_snconvf_szbug;
            strncat(buf, argv[i].a_w.w_symbol->s_name, bufsz);
        } else {
            len += 4;
            if(bufsz <= len) goto list_snconvf_szbug;
            strncat(buf, "null", bufsz);
        }
    }
    if(strlen(buf) >= bufsz) {
        goto list_snconvf_szbug;
    }
    return;

list_snconvf_szbug:
    debugprint("track: BUG: list_snconvf_szbug (message too long)");
    bug("track: BUG: list_snconvf_szbug (message too long)");
}

void composer_setup(void) {
    debugprint("loading composer library for pd");
    sys_vgui("source {window.tk}\n");
    song_proxy_setup();
    track_proxy_setup();
}
