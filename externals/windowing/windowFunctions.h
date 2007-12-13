/* Copyright (C) 2002 Joseph A. Sarlo
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** jsarlo@mambo.peabody.jhu.edu
*/

void fillHanning(float *vec, int n);
void fillHamming(float *vec, int n);
void fillBlackman(float *vec, int n);
void fillConnes(float *vec, int n);
void fillCosine(float *vec, int n);
void fillWelch(float *vec, int n);
void fillBartlett(float *vec, int n);
void fillLanczos(float *vec, int n);
void fillGaussian(float *vec, int n, float delta);
void fillKaiser(float *vec, int n, float delta);

