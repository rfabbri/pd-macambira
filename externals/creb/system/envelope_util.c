/*
 *   Utility functions for exponential decay 
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "extlib_util.h"

float milliseconds_2_one_minus_realpole(float time)
{
  float r;

  if (time < 0.0f) time = 0.0f;
  r = -expm1(1000.0f * log(ENVELOPE_RANGE) / (sys_getsr() * time));
  if (!(r < 1.0f)) r = 1.0f;

  //post("%f",r);
  return r;
}
