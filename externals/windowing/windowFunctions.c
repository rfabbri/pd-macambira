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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#ifdef NT
#define M_PI 3.14159265358979323846
#endif

/* modified bessel function of zeroth order */
double i0(double x);

void fillHanning(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(0.5 * (1 + cos(M_PI * x)));
  }
}

void fillHamming(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(0.54 + 0.46 * cos(M_PI * x));
  }
}

void fillBlackman(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(0.42 + (0.5 * cos(M_PI * x)) + (0.08 * cos (2 * M_PI * x)));
  }
}

void fillConnes(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)((1  - (x * x)) * (1 - (x * x)));
  }
}

void fillCosine(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)cos(M_PI * x / 2);
  }
}

void fillWelch(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = 1 - (x * x);
  }
}

void fillBartlett(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(1 - fabs(x));
  }
}

void fillLanczos(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    if (x == 0) {
      vec[i] = 1;
    }
    else {
      vec[i] = (float)(sin(M_PI * x) / (M_PI * x));
    }
  }
}

void fillGaussian(float *vec, int n, float delta) {
  int i;
  float xShift = (float)n / 2;
  float x;
  if (delta == 0) {
    delta = 1;
  }
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(pow(2, (-1 * (x / delta) * (x / delta))));
  }
}

void fillKaiser(float *vec, int n, float alpha) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(i0(alpha * sqrt(1 - (x * x))) / i0(alpha));
  }
}
