/* plugin~, a Pd tilde object for hosting LADSPA/VST plug-ins
   Copyright (C) 2000 Jarno Seppänen
   $Id: winplugintilde.h,v 1.1 2002-11-19 09:52:46 ggeiger Exp $

   This file is part of plugin~.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#ifndef __WINPLUGINTILDE_H__
#define __WINPLUGINTILDE_H__

#include "vitunmsvc.h" /* rint, M_PI */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


__declspec(dllexport) extern void winplugintilde_setup (void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WINPLUGINTILDE_H__ */
/* EOF */
