/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* g_7_guis.c written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001 */
/* thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "m_imp.h"
#include "g_canvas.h"
#include "t_tk.h"
#include "g_all_guis.h"
#include <math.h>

#ifdef NT
#include <io.h>
#else
#include <unistd.h>
#endif

/*  #define GGEE_HSLIDER_COMPATIBLE  */

/*------------------ global varaibles -------------------------*/

int iemgui_color_hex[]=
{
    16579836, 10526880, 4210752, 16572640, 16572608,
    16579784, 14220504, 14220540, 14476540, 16308476,
    14737632, 8158332, 2105376, 16525352, 16559172,
    15263784, 1370132, 2684148, 3952892, 16003312,
    12369084, 6316128, 0, 9177096, 5779456,
    7874580, 2641940, 17488, 5256, 5767248
};

int iemgui_vu_db2i[]=
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9,10,10,10,10,10,
    11,11,11,11,11,12,12,12,12,12,
    13,13,13,13,14,14,14,14,15,15,
    15,15,16,16,16,16,17,17,17,18,
    18,18,19,19,19,20,20,20,21,21,
    22,22,23,23,24,24,25,26,27,28,
    29,30,31,32,33,33,34,34,35,35,
    36,36,37,37,37,38,38,38,39,39,
    39,39,39,39,40,40
};

int iemgui_vu_col[]=
{
    0,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    15,15,15,15,15,15,15,15,15,15,14,14,13,13,13,13,13,13,13,13,13,13,13,19,19,19
};

char *iemgui_vu_scale_str[]=
{
    "",
    "<-99",
    "",
    "",
    "",
    "-50",
    "",
    "",
    "",
    "-30",
    "",
    "",
    "",
    "-20",
    "",
    "",
    "",
    "-12",
    "",
    "",
    "",
    "-6",
    "",
    "",
    "",
    "-2",
    "",
    "",
    "",
    "-0dB",
    "",
    "",
    "",
    "+2",
    "",
    "",
    "",
    "+6",
    "",
    "",
    "",
    ">+12",
    "",
    "",
    "",
    "",
    "",
};


/*------------------ global functions -------------------------*/


int iemgui_clip_size(int size)
{
    if(size < IEM_GUI_MINSIZE)
	size = IEM_GUI_MINSIZE;
    return(size);
}

int iemgui_clip_font(int size)
{
    if(size < IEM_FONT_MINSIZE)
	size = IEM_FONT_MINSIZE;
    return(size);
}

int iemgui_modulo_color(int col)
{
    while(col >= IEM_GUI_MAX_COLOR)
	col -= IEM_GUI_MAX_COLOR;
    while(col < 0)
	col += IEM_GUI_MAX_COLOR;
    return(col);
}

t_symbol *iemgui_raute2dollar(t_symbol *s)
{
    if (s->s_name[0] == '#')
    {
    	char buf[MAXPDSTRING];
	strncpy(buf, s->s_name, MAXPDSTRING);
	buf[MAXPDSTRING-1] = 0;
	buf[0] = '$';
	return (gensym(buf));
    }
    else return (s);
}

t_symbol *iemgui_dollar2raute(t_symbol *s)
{
    if (s->s_name[0] == '$')
    {
    	char buf[MAXPDSTRING];
	strncpy(buf, s->s_name, MAXPDSTRING);
	buf[MAXPDSTRING-1] = 0;
	buf[0] = '#';
	return (gensym(buf));
    }
    else return (s);
}

t_symbol *iemgui_unique2dollarzero(t_symbol *s, int unique_num, int and_unique_flag)
{
    if(and_unique_flag)
    {
	int l=0;
	char *b, str[144];

	sprintf(str, "%d", unique_num);
	while(str[l])
	{
	    if(s->s_name[l] != str[l])
		return(s);
	    else
		l++;
	}
	str[0] = '$';
	str[1] = '0';
	str[2] = 0;
	b = s->s_name + l;
	if(strlen(b) >= IEM_MAX_SYM_LEN)
	    strncat(str, b, IEM_MAX_SYM_LEN-1);
	else
	    strcat(str, b);
	return(gensym(str));
    }
    else
	return(s);
}

t_symbol *iemgui_sym2dollararg(t_symbol *s, int nth_arg, int tail_len)
{
    if(nth_arg)
    {
	char *b, str[144];
	int i=(int)strlen(s->s_name) - tail_len;

	sprintf(str, "_%d", nth_arg);
	str[0] = '$';
	if(i < 0) i = 0;
	b = s->s_name + i;
	strcat(str, b);
	return(gensym(str));
    }
    else
	return(s);
}

t_symbol *iemgui_dollarzero2unique(t_symbol *s, int unique_num)
{
    int l;
    char *b, str[144];

    sprintf(str, "%d", unique_num);
    l = (int)strlen(s->s_name);
    b = s->s_name + 2;
    if(l < 2)
	strcat(str, "shorty");
    else if(l >= IEM_MAX_SYM_LEN)
	strncat(str, b, IEM_MAX_SYM_LEN-1);
    else
	strcat(str, b);
    return(gensym(str));
}

t_symbol *iemgui_dollararg2sym(t_symbol *s, int nth_arg, int tail_len, int pargc, t_atom *pargv)
{
    int l=(int)strlen(s->s_name);
    char *b, str[288]="0";
    t_symbol *s2;

    if(pargc <= 0){}
    else if(nth_arg < 1){}
    else if(nth_arg > pargc){}
    else if(IS_A_FLOAT(pargv, nth_arg-1))
	sprintf(str, "%d", atom_getintarg(nth_arg-1, pargc, pargv));
    else if(IS_A_SYMBOL(pargv, nth_arg-1))
    {
	s2 = atom_getsymbolarg(nth_arg-1, pargc, pargv);
	strcpy(str, s2->s_name);
    }
    b = s->s_name + (l - tail_len);
    if(l <= tail_len)
	strcat(str, "shorty");
    else if(l >= IEM_MAX_SYM_LEN)
	strncat(str, b, IEM_MAX_SYM_LEN-1);
    else
	strcat(str, b);
    return(gensym(str));
}

int iemgui_is_dollarzero(t_symbol *s)
{
    char *name=s->s_name;

    if((int)strlen(name) >= 2)
    {
	if((name[0] == '$') && (name[1] == '0') && ((name[2] < '0') || (name[2] > '9')))
	    return(1);
    }
    return(0);
}

int iemgui_is_dollararg(t_symbol *s, int *tail_len)
{
    char *name=s->s_name;

    *tail_len = (int)strlen(name);
    if(*tail_len >= 2)
    {
	if((name[0] == '$') && (name[1] >= '1') && (name[1] <= '9'))
	{
	    int i=2, arg=(int)(name[1]-'0');

	    (*tail_len) -= 2;
	    while(name[i] && (name[i] >= '0') && (name[i] <= '9'))
	    {
		arg *= 10;
		arg += (int)(name[i]-'0');
		i++;
		(*tail_len)--;
	    }
	    return(arg);
	}
    }
    return(0);
}

void iemgui_fetch_unique(t_iemgui *iemgui)
{
    if(!iemgui->x_unique_num)
    {
	pd_bind(&iemgui->x_glist->gl_gobj.g_pd, gensym("#X"));
	iemgui->x_unique_num = canvas_getdollarzero();
	pd_unbind(&iemgui->x_glist->gl_gobj.g_pd, gensym("#X"));
    }
}

void iemgui_fetch_parent_args(t_iemgui *iemgui, int *pargc, t_atom **pargv)
{
    t_canvas *canvas=glist_getcanvas(iemgui->x_glist);

    canvas_setcurrent(canvas);
    canvas_getargs(pargc, pargv);
    canvas_unsetcurrent(canvas);
}

void iemgui_verify_snd_ne_rcv(t_iemgui *iemgui)
{
    iemgui->x_fsf.x_put_in2out = 1;
    if(iemgui->x_fsf.x_snd_able && iemgui->x_fsf.x_rcv_able)
    {
	if(!strcmp(iemgui->x_snd->s_name, iemgui->x_rcv->s_name))
	    iemgui->x_fsf.x_put_in2out = 0;
    }
}

void iemgui_all_unique2dollarzero(t_iemgui *iemgui, t_symbol **srlsym)
{
    iemgui_fetch_unique(iemgui);
    srlsym[0] = iemgui_unique2dollarzero(srlsym[0], iemgui->x_unique_num,
					  iemgui->x_fsf.x_snd_is_unique);
    srlsym[1] = iemgui_unique2dollarzero(srlsym[1], iemgui->x_unique_num,
					  iemgui->x_fsf.x_rcv_is_unique);
    srlsym[2] = iemgui_unique2dollarzero(srlsym[2], iemgui->x_unique_num,
					  iemgui->x_fsf.x_lab_is_unique);
}

void iemgui_all_sym2dollararg(t_iemgui *iemgui, t_symbol **srlsym)
{
    srlsym[0] = iemgui_sym2dollararg(srlsym[0], iemgui->x_isa.x_snd_is_arg_num,
				      iemgui->x_isa.x_snd_arg_tail_len);
    srlsym[1] = iemgui_sym2dollararg(srlsym[1], iemgui->x_isa.x_rcv_is_arg_num,
				      iemgui->x_isa.x_rcv_arg_tail_len);
    srlsym[2] = iemgui_sym2dollararg(srlsym[2], iemgui->x_fsf.x_lab_is_arg_num,
				      iemgui->x_fsf.x_lab_arg_tail_len);
}

void iemgui_all_dollarzero2unique(t_iemgui *iemgui, t_symbol **srlsym)
{
    iemgui_fetch_unique(iemgui);
    if(iemgui_is_dollarzero(srlsym[0]))
    {
	iemgui->x_fsf.x_snd_is_unique = 1;
	iemgui->x_isa.x_snd_is_arg_num = 0;
	iemgui->x_isa.x_snd_arg_tail_len = 0;
	srlsym[0] = iemgui_dollarzero2unique(srlsym[0], iemgui->x_unique_num);
    }
    else
	iemgui->x_fsf.x_snd_is_unique = 0;
    if(iemgui_is_dollarzero(srlsym[1]))
    {
	iemgui->x_fsf.x_rcv_is_unique = 1;
	iemgui->x_isa.x_rcv_is_arg_num = 0;
	iemgui->x_isa.x_rcv_arg_tail_len = 0;
	srlsym[1] = iemgui_dollarzero2unique(srlsym[1], iemgui->x_unique_num);
    }
    else
	iemgui->x_fsf.x_rcv_is_unique = 0;
    if(iemgui_is_dollarzero(srlsym[2]))
    {
	iemgui->x_fsf.x_lab_is_unique = 1;
	iemgui->x_fsf.x_lab_is_arg_num = 0;
	iemgui->x_fsf.x_lab_arg_tail_len = 0;
	srlsym[2] = iemgui_dollarzero2unique(srlsym[2], iemgui->x_unique_num);
    }
    else
	iemgui->x_fsf.x_lab_is_unique = 0;
}

void iemgui_all_dollararg2sym(t_iemgui *iemgui, t_symbol **srlsym)
{
    int pargc, tail_len, nth_arg;
    t_atom *pargv;

    iemgui_fetch_parent_args(iemgui, &pargc, &pargv);
    if(nth_arg = iemgui_is_dollararg(srlsym[0], &tail_len))
    {
	iemgui->x_isa.x_snd_is_arg_num = nth_arg;
	iemgui->x_isa.x_snd_arg_tail_len = tail_len;
	iemgui->x_fsf.x_snd_is_unique = 0;
	srlsym[0] = iemgui_dollararg2sym(srlsym[0], nth_arg, tail_len, pargc, pargv);
    }
    else
    {
	iemgui->x_isa.x_snd_is_arg_num = 0;
	iemgui->x_isa.x_snd_arg_tail_len = 0;
    }
    if(nth_arg = iemgui_is_dollararg(srlsym[1], &tail_len))
    {
	iemgui->x_isa.x_rcv_is_arg_num = nth_arg;
	iemgui->x_isa.x_rcv_arg_tail_len = tail_len;
	iemgui->x_fsf.x_rcv_is_unique = 0;
	srlsym[1] = iemgui_dollararg2sym(srlsym[1], nth_arg, tail_len, pargc, pargv);
    }
    else
    {
	iemgui->x_isa.x_rcv_is_arg_num = 0;
	iemgui->x_isa.x_rcv_arg_tail_len = 0;
    }
    if(nth_arg = iemgui_is_dollararg(srlsym[2], &tail_len))
    {
	iemgui->x_fsf.x_lab_is_arg_num = nth_arg;
	iemgui->x_fsf.x_lab_arg_tail_len = tail_len;
	iemgui->x_fsf.x_lab_is_unique = 0;
	srlsym[2] = iemgui_dollararg2sym(srlsym[2], nth_arg, tail_len, pargc, pargv);
    }
    else
    {
	iemgui->x_fsf.x_lab_is_arg_num = 0;
	iemgui->x_fsf.x_lab_arg_tail_len = 0;
    }
}

void iemgui_first_dollararg2sym(t_iemgui *iemgui, t_symbol **srlsym)
{
    int pargc=0, tail_len, nth_arg;
    t_atom pargv;
    char *name;

    SETFLOAT(&pargv, 0.0);
    name = srlsym[0]->s_name;
    if(iemgui->x_isa.x_snd_is_arg_num && (name[0] == '$')
       && (name[1] >= '1') && (name[1] <= '9'))
    {
	srlsym[0] = iemgui_dollararg2sym(srlsym[0], iemgui->x_isa.x_snd_is_arg_num,
					  iemgui->x_isa.x_snd_arg_tail_len, pargc, &pargv);
    }
    name = srlsym[1]->s_name;
    if(iemgui->x_isa.x_rcv_is_arg_num && (name[0] == '$')
       && (name[1] >= '1') && (name[1] <= '9'))
    {
	srlsym[1] = iemgui_dollararg2sym(srlsym[1], iemgui->x_isa.x_rcv_is_arg_num,
					  iemgui->x_isa.x_rcv_arg_tail_len, pargc, &pargv);
    }
    name = srlsym[2]->s_name;
    if(iemgui->x_fsf.x_lab_is_arg_num && (name[0] == '$')
       && (name[1] >= '1') && (name[1] <= '9'))
    {
	srlsym[2] = iemgui_dollararg2sym(srlsym[2], iemgui->x_fsf.x_lab_is_arg_num,
					  iemgui->x_fsf.x_lab_arg_tail_len, pargc, &pargv);
    }
}

void iemgui_all_col2save(t_iemgui *iemgui, int *bflcol)
{
    bflcol[0] = -1 - (((0xfc0000 & iemgui->x_bcol) >> 6)|
		      ((0xfc00 & iemgui->x_bcol) >> 4)|((0xfc & iemgui->x_bcol) >> 2));
    bflcol[1] = -1 - (((0xfc0000 & iemgui->x_fcol) >> 6)|
		      ((0xfc00 & iemgui->x_fcol) >> 4)|((0xfc & iemgui->x_fcol) >> 2));
    bflcol[2] = -1 - (((0xfc0000 & iemgui->x_lcol) >> 6)|
		      ((0xfc00 & iemgui->x_lcol) >> 4)|((0xfc & iemgui->x_lcol) >> 2));
}

void iemgui_all_colfromload(t_iemgui *iemgui, int *bflcol)
{
    if(bflcol[0] < 0)
    {
	bflcol[0] = -1 - bflcol[0];
	iemgui->x_bcol = ((bflcol[0] & 0x3f000) << 6)|((bflcol[0] & 0xfc0) << 4)|
	    ((bflcol[0] & 0x3f) << 2);
    }
    else
    {
	bflcol[0] = iemgui_modulo_color(bflcol[0]);
	iemgui->x_bcol = iemgui_color_hex[bflcol[0]];
    }
    if(bflcol[1] < 0)
    {
	bflcol[1] = -1 - bflcol[1];
	iemgui->x_fcol = ((bflcol[1] & 0x3f000) << 6)|((bflcol[1] & 0xfc0) << 4)|
	    ((bflcol[1] & 0x3f) << 2);
    }
    else
    {
	bflcol[1] = iemgui_modulo_color(bflcol[1]);
	iemgui->x_fcol = iemgui_color_hex[bflcol[1]];
    }
    if(bflcol[2] < 0)
    {
	bflcol[2] = -1 - bflcol[2];
	iemgui->x_lcol = ((bflcol[2] & 0x3f000) << 6)|((bflcol[2] & 0xfc0) << 4)|
	    ((bflcol[2] & 0x3f) << 2);
    }
    else
    {
	bflcol[2] = iemgui_modulo_color(bflcol[2]);
	iemgui->x_lcol = iemgui_color_hex[bflcol[2]];
    }
}

int iemgui_compatible_col(int i)
{
    int j;

    if(i >= 0)
    {
	j = iemgui_modulo_color(i);
	return(iemgui_color_hex[(j)]);
    }
    else
	return((-1 -i)&0xffffff);
}

void iemgui_all_dollar2raute(t_symbol **srlsym)
{
    srlsym[0] = iemgui_dollar2raute(srlsym[0]);
    srlsym[1] = iemgui_dollar2raute(srlsym[1]);
    srlsym[2] = iemgui_dollar2raute(srlsym[2]);
}

void iemgui_all_raute2dollar(t_symbol **srlsym)
{
    srlsym[0] = iemgui_raute2dollar(srlsym[0]);
    srlsym[1] = iemgui_raute2dollar(srlsym[1]);
    srlsym[2] = iemgui_raute2dollar(srlsym[2]);
}

void iemgui_send(void *x, t_iemgui *iemgui, t_symbol *s)
{
    t_symbol *snd;
    int pargc, tail_len, nth_arg, sndable=1, oldsndrcvable=0;
    t_atom *pargv;

    if(iemgui->x_fsf.x_rcv_able)
	oldsndrcvable += IEM_GUI_OLD_RCV_FLAG;
    if(iemgui->x_fsf.x_snd_able)
        oldsndrcvable += IEM_GUI_OLD_SND_FLAG;

    if(!strcmp(s->s_name, "empty")) sndable = 0;
    iemgui_fetch_unique(iemgui);
    snd = iemgui_raute2dollar(s);
    if(iemgui_is_dollarzero(snd))
    {
	iemgui->x_fsf.x_snd_is_unique = 1;
	iemgui->x_isa.x_snd_is_arg_num = 0;
	iemgui->x_isa.x_snd_arg_tail_len = 0;
	snd = iemgui_dollarzero2unique(snd, iemgui->x_unique_num);
    }
    else
	iemgui->x_fsf.x_snd_is_unique = 0;
    iemgui_fetch_parent_args(iemgui, &pargc, &pargv);
    if(nth_arg = iemgui_is_dollararg(snd, &tail_len))
    {
	iemgui->x_isa.x_snd_is_arg_num = nth_arg;
	iemgui->x_isa.x_snd_arg_tail_len = tail_len;
	iemgui->x_fsf.x_snd_is_unique = 0;
	snd = iemgui_dollararg2sym(snd, nth_arg, tail_len, pargc, pargv);
    }
    else
    {
	iemgui->x_isa.x_snd_is_arg_num = 0;
	iemgui->x_isa.x_snd_arg_tail_len = 0;
    }
    iemgui->x_snd = snd;
    iemgui->x_fsf.x_snd_able = sndable;
    iemgui_verify_snd_ne_rcv(iemgui);
    (*iemgui->x_draw)(x, iemgui->x_glist, IEM_GUI_DRAW_MODE_IO + oldsndrcvable);
}

void iemgui_receive(void *x, t_iemgui *iemgui, t_symbol *s)
{
    t_symbol *rcv;
    int pargc, tail_len, nth_arg, rcvable=1, oldsndrcvable=0;
    t_atom *pargv;

    if(iemgui->x_fsf.x_rcv_able)
	oldsndrcvable += IEM_GUI_OLD_RCV_FLAG;
    if(iemgui->x_fsf.x_snd_able)
        oldsndrcvable += IEM_GUI_OLD_SND_FLAG;

    if(!strcmp(s->s_name, "empty")) rcvable = 0;
    rcv = iemgui_raute2dollar(s);
    iemgui_fetch_unique(iemgui);
    if(iemgui_is_dollarzero(rcv))
    {
	iemgui->x_fsf.x_rcv_is_unique = 1;
	iemgui->x_isa.x_rcv_is_arg_num = 0;
	iemgui->x_isa.x_rcv_arg_tail_len = 0;
	rcv = iemgui_dollarzero2unique(rcv, iemgui->x_unique_num);
    }
    else
	iemgui->x_fsf.x_rcv_is_unique = 0;
    iemgui_fetch_parent_args(iemgui, &pargc, &pargv);
    if(nth_arg = iemgui_is_dollararg(rcv, &tail_len))
    {
	iemgui->x_isa.x_rcv_is_arg_num = nth_arg;
	iemgui->x_isa.x_rcv_arg_tail_len = tail_len;
	iemgui->x_fsf.x_rcv_is_unique = 0;
	rcv = iemgui_dollararg2sym(rcv, nth_arg, tail_len, pargc, pargv);
    }
    else
    {
	iemgui->x_isa.x_rcv_is_arg_num = 0;
	iemgui->x_isa.x_rcv_arg_tail_len = 0;
    }
    if(rcvable)
    {
	if(strcmp(rcv->s_name, iemgui->x_rcv->s_name))
	{
	    if(iemgui->x_fsf.x_rcv_able)
		pd_unbind(&iemgui->x_obj.ob_pd, iemgui->x_rcv);
	    iemgui->x_rcv = rcv;
	    pd_bind(&iemgui->x_obj.ob_pd, iemgui->x_rcv);
	}
    }
    else if(!rcvable && iemgui->x_fsf.x_rcv_able)
    {
	pd_unbind(&iemgui->x_obj.ob_pd, iemgui->x_rcv);
	iemgui->x_rcv = rcv;
    }
    iemgui->x_fsf.x_rcv_able = rcvable;
    iemgui_verify_snd_ne_rcv(iemgui);
    (*iemgui->x_draw)(x, iemgui->x_glist, IEM_GUI_DRAW_MODE_IO + oldsndrcvable);
}

void iemgui_label(void *x, t_iemgui *iemgui, t_symbol *s)
{
    t_symbol *lab;
    int pargc, tail_len, nth_arg;
    t_atom *pargv;

    lab = iemgui_raute2dollar(s);
    iemgui_fetch_unique(iemgui);

    if(iemgui_is_dollarzero(lab))
    {
	iemgui->x_fsf.x_lab_is_unique = 1;
	iemgui->x_fsf.x_lab_is_arg_num = 0;
	iemgui->x_fsf.x_lab_arg_tail_len = 0;
	lab = iemgui_dollarzero2unique(lab, iemgui->x_unique_num);
    }
    else
	iemgui->x_fsf.x_lab_is_unique = 0;

    iemgui_fetch_parent_args(iemgui, &pargc, &pargv);
    if(nth_arg = iemgui_is_dollararg(lab, &tail_len))
    {
	iemgui->x_fsf.x_lab_is_arg_num = nth_arg;
	iemgui->x_fsf.x_lab_arg_tail_len = tail_len;
	iemgui->x_fsf.x_lab_is_unique = 0;
	lab = iemgui_dollararg2sym(lab, nth_arg, tail_len, pargc, pargv);
    }
    else
    {
	iemgui->x_fsf.x_lab_is_arg_num = 0;
	iemgui->x_fsf.x_lab_arg_tail_len = 0;
    }
    iemgui->x_lab = lab;
    if(glist_isvisible(iemgui->x_glist))
	sys_vgui(".x%x.c itemconfigure %xLABEL -text {%s} \n",
		 glist_getcanvas(iemgui->x_glist), x,
		 strcmp(s->s_name, "empty")?iemgui->x_lab->s_name:"");
}

void iemgui_label_pos(void *x, t_iemgui *iemgui, t_symbol *s, int ac, t_atom *av)
{
    iemgui->x_ldx = (int)atom_getintarg(0, ac, av);
    iemgui->x_ldy = (int)atom_getintarg(1, ac, av);
    if(glist_isvisible(iemgui->x_glist))
	sys_vgui(".x%x.c coords %xLABEL %d %d\n",
		 glist_getcanvas(iemgui->x_glist), x,
		 iemgui->x_obj.te_xpix+iemgui->x_ldx,
		 iemgui->x_obj.te_ypix+iemgui->x_ldy);
}

void iemgui_label_font(void *x, t_iemgui *iemgui, t_symbol *s, int ac, t_atom *av)
{
    int f = (int)atom_getintarg(0, ac, av);

    if(f == 1) strcpy(iemgui->x_font, "helvetica");
    else if(f == 2) strcpy(iemgui->x_font, "times");
    else
    {
	f = 0;
	strcpy(iemgui->x_font, "courier");
    }
    iemgui->x_fsf.x_font_style = f;
    f = (int)atom_getintarg(1, ac, av);
    if(f < 4)
	f = 4;
    iemgui->x_fontsize = f;
    if(glist_isvisible(iemgui->x_glist))
	sys_vgui(".x%x.c itemconfigure %xLABEL -font {%s %d bold}\n",
		 glist_getcanvas(iemgui->x_glist), x, iemgui->x_font, iemgui->x_fontsize);
}

void iemgui_size(void *x, t_iemgui *iemgui)
{
    if(glist_isvisible(iemgui->x_glist))
    {
	(*iemgui->x_draw)(x, iemgui->x_glist, IEM_GUI_DRAW_MODE_MOVE);
	canvas_fixlinesfor(glist_getcanvas(iemgui->x_glist), (t_text*)x);
    }
}

void iemgui_delta(void *x, t_iemgui *iemgui, t_symbol *s, int ac, t_atom *av)
{
    iemgui->x_obj.te_xpix += (int)atom_getintarg(0, ac, av);
    iemgui->x_obj.te_ypix += (int)atom_getintarg(1, ac, av);
    if(glist_isvisible(iemgui->x_glist))
    {
	(*iemgui->x_draw)(x, iemgui->x_glist, IEM_GUI_DRAW_MODE_MOVE);
	canvas_fixlinesfor(glist_getcanvas(iemgui->x_glist), (t_text*)x);
    }
}

void iemgui_pos(void *x, t_iemgui *iemgui, t_symbol *s, int ac, t_atom *av)
{
    iemgui->x_obj.te_xpix = (int)atom_getintarg(0, ac, av);
    iemgui->x_obj.te_ypix = (int)atom_getintarg(1, ac, av);
    if(glist_isvisible(iemgui->x_glist))
    {
	(*iemgui->x_draw)(x, iemgui->x_glist, IEM_GUI_DRAW_MODE_MOVE);
	canvas_fixlinesfor(glist_getcanvas(iemgui->x_glist), (t_text*)x);
    }
}

void iemgui_color(void *x, t_iemgui *iemgui, t_symbol *s, int ac, t_atom *av)
{
    iemgui->x_bcol = iemgui_compatible_col(atom_getintarg(0, ac, av));
    if(ac > 2)
    {
	iemgui->x_fcol = iemgui_compatible_col(atom_getintarg(1, ac, av));
	iemgui->x_lcol = iemgui_compatible_col(atom_getintarg(2, ac, av));
    }
    else
	iemgui->x_lcol = iemgui_compatible_col(atom_getintarg(1, ac, av));
    if(glist_isvisible(iemgui->x_glist))
	(*iemgui->x_draw)(x, iemgui->x_glist, IEM_GUI_DRAW_MODE_CONFIG);
}

void iemgui_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_iemguidummy *x = (t_iemguidummy *)z;

    x->x_gui.x_obj.te_xpix += dx;
    x->x_gui.x_obj.te_ypix += dy;
    (*x->x_gui.x_draw)((void *)z, glist, IEM_GUI_DRAW_MODE_MOVE);
    canvas_fixlinesfor(glist_getcanvas(glist), (t_text *)z);
}

void iemgui_select(t_gobj *z, t_glist *glist, int selected)
{
    t_iemguidummy *x = (t_iemguidummy *)z;

    x->x_gui.x_fsf.x_selected = selected;
    (*x->x_gui.x_draw)((void *)z, glist, IEM_GUI_DRAW_MODE_SELECT);
}

void iemgui_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

void iemgui_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_iemguidummy *x = (t_iemguidummy *)z;
    t_rtext *y;

    if(vis)
    {
	y = rtext_new_without_senditup(glist, (t_text *)z, glist->gl_editor->e_rtext);
	(*x->x_gui.x_draw)((void *)z, glist, IEM_GUI_DRAW_MODE_NEW);
    }
    else
    {
	y = glist_findrtext(glist, (t_text *)z);
	(*x->x_gui.x_draw)((void *)z, glist, IEM_GUI_DRAW_MODE_ERASE);
	rtext_free(y);
    }
}

void iemgui_save(t_iemgui *iemgui, t_symbol **srl, int *bflcol)
{
    srl[0] = iemgui->x_snd;
    srl[1] = iemgui->x_rcv;
    srl[2] = iemgui->x_lab;
    iemgui_all_unique2dollarzero(iemgui, srl);
    iemgui_all_sym2dollararg(iemgui, srl);
    iemgui_all_col2save(iemgui, bflcol);
}

void iemgui_properties(t_iemgui *iemgui, t_symbol **srl)
{
    srl[0] = iemgui->x_snd;
    srl[1] = iemgui->x_rcv;
    srl[2] = iemgui->x_lab;
    iemgui_all_unique2dollarzero(iemgui, srl);
    iemgui_all_sym2dollararg(iemgui, srl);
    iemgui_all_dollar2raute(srl);
}

int iemgui_dialog(t_iemgui *iemgui, t_symbol **srl, int argc, t_atom *argv)
{
    char str[144];
    int init = (int)atom_getintarg(5, argc, argv);
    int ldx = (int)atom_getintarg(10, argc, argv);
    int ldy = (int)atom_getintarg(11, argc, argv);
    int f = (int)atom_getintarg(12, argc, argv);
    int fs = (int)atom_getintarg(13, argc, argv);
    int bcol = (int)atom_getintarg(14, argc, argv);
    int fcol = (int)atom_getintarg(15, argc, argv);
    int lcol = (int)atom_getintarg(16, argc, argv);
    int sndable=1, rcvable=1, oldsndrcvable=0;

    if(iemgui->x_fsf.x_rcv_able)
	oldsndrcvable += IEM_GUI_OLD_RCV_FLAG;
    if(iemgui->x_fsf.x_snd_able)
        oldsndrcvable += IEM_GUI_OLD_SND_FLAG;
    if(IS_A_SYMBOL(argv,7))
	srl[0] = atom_getsymbolarg(7, argc, argv);
    else if(IS_A_FLOAT(argv,7))
    {
	sprintf(str, "%d", (int)atom_getintarg(7, argc, argv));
	srl[0] = gensym(str);
    }
    if(IS_A_SYMBOL(argv,8))
	srl[1] = atom_getsymbolarg(8, argc, argv);
    else if(IS_A_FLOAT(argv,8))
    {
	sprintf(str, "%d", (int)atom_getintarg(8, argc, argv));
	srl[1] = gensym(str);
    }
    if(IS_A_SYMBOL(argv,9))
	srl[2] = atom_getsymbolarg(9, argc, argv);
    else if(IS_A_FLOAT(argv,9))
    {
	sprintf(str, "%d", (int)atom_getintarg(9, argc, argv));
	srl[2] = gensym(str);
    }
    if(init != 0) init = 1;
    iemgui->x_isa.x_loadinit = init;
    if(!strcmp(srl[0]->s_name, "empty")) sndable = 0;
    if(!strcmp(srl[1]->s_name, "empty")) rcvable = 0;
    iemgui_all_raute2dollar(srl);
    iemgui_all_dollarzero2unique(iemgui, srl);
    iemgui_all_dollararg2sym(iemgui, srl);
    if(rcvable)
    {
	if(strcmp(srl[1]->s_name, iemgui->x_rcv->s_name))
	{
	    if(iemgui->x_fsf.x_rcv_able)
		pd_unbind(&iemgui->x_obj.ob_pd, iemgui->x_rcv);
	    iemgui->x_rcv = srl[1];
	    pd_bind(&iemgui->x_obj.ob_pd, iemgui->x_rcv);
	}
    }
    else if(!rcvable && iemgui->x_fsf.x_rcv_able)
    {
	pd_unbind(&iemgui->x_obj.ob_pd, iemgui->x_rcv);
	iemgui->x_rcv = srl[1];
    }
    iemgui->x_snd = srl[0];
    iemgui->x_fsf.x_snd_able = sndable;
    iemgui->x_fsf.x_rcv_able = rcvable;
    iemgui->x_lcol = lcol & 0xffffff;
    iemgui->x_fcol = fcol & 0xffffff;
    iemgui->x_bcol = bcol & 0xffffff;
    iemgui->x_lab = srl[2];
    iemgui->x_ldx = ldx;
    iemgui->x_ldy = ldy;
    if(f == 1) strcpy(iemgui->x_font, "helvetica");
    else if(f == 2) strcpy(iemgui->x_font, "times");
    else
    {
	f = 0;
	strcpy(iemgui->x_font, "courier");
    }
    iemgui->x_fsf.x_font_style = f;
    if(fs < 4)
	fs = 4;
    iemgui->x_fontsize = fs;
    iemgui_verify_snd_ne_rcv(iemgui);
    return(oldsndrcvable);
}
