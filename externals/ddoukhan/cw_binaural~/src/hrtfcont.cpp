/*
    cw_binaural~: a binaural synthesis external for pure data
    by David Doukhan - david.doukhan@gmail.com - http://www.limsi.fr/Individu/doukhan
    and Anne Sedes - sedes.anne@gmail.com
    Copyright (C) 2009-2011  David Doukhan and Anne Sedes

    For more details, see CW_binaural~, a binaural synthesis external for Pure Data
    David Doukhan and Anne Sedes, PDCON09


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <iostream>
#include <math.h>
#include "hrtfcont.hpp"
#include "logstring.hpp"

#define DEG2RAD (M_PI/180)
#define RAD2DEG (180/M_PI)
#define EPS 0.0000000000001

HrtfCont::HrtfCont(const ir_key& k):
  _vert_pol_coords(k.vertical_polar_coords) {}

// set elevation in [-90, +90], and azimuth in [-180, 180[
void HrtfCont::normalize_vertpolar_coords(float& azimuth, float& elevation) const
{
  // convert any elevation into a usefull value
  // the value used should lie between -90 and +90
  elevation = fmod(elevation, 360);
  if (elevation < 0)
    elevation += 360;

  if (elevation > 270)
    elevation -= 360;
  else if (elevation > 90)
    {
      elevation = 180 - elevation;
      azimuth += 180;
    }
  
  // azimuth values should be in [0,360[
  azimuth = fmod(azimuth, 360);
  if (azimuth < -180)
    azimuth += 360;
  else if (azimuth >= 180)
    azimuth -= 360;
}


// return the distance in degree between 2 normalized angles
// ie both angles are in [0, 360[
float HrtfCont::angular_dist(float a1, float a2) const
{
  float d = fabs(a1-a2);
  return d <= 180 ? d : 360 - d;
}


// iangle1, iangle2
void HrtfCont::add_a2_candidates(interp_cdts& ic, float a1_key, float a2, float weight)
{
  // get impulse responses corresponding to the current elevation
  const angle2_cont& a2_candidates = _m[a1_key];
  //slog << "interp for fixed angle1=" << a1_key << ", optimal angle2="<< a2<< ", weight=" << weight << endl;
  // there is only one angle2 measure at index angle1
  if (a2_candidates.size() == 1)
    return ic.add(a1_key, a2_candidates.begin()->first, weight);

  // iterator the the 1st element of key bigger or equal
  angle2_cit	supeq_a2it = a2_candidates.lower_bound(a2);  

  // the requested angle2 corresponds exactly to an available measure
  if (supeq_a2it != a2_candidates.end() && a2 == supeq_a2it->first)
    return ic.add(a1_key, supeq_a2it->first, weight); 
  
  // index candidates for angle2
  float	a2_cand1, a2_cand2;
  if (supeq_a2it == a2_candidates.end() || supeq_a2it == a2_candidates.begin())
    {
      //slog << "angle2 btwn 2 extremes" << (supeq_a2it == a2_candidates.end()) << (supeq_a2it == a2_candidates.begin()) << endl;
      // requested angle2 is bigger than all available measures
      // or smaller than all available measures
      // interpolation between extreme measures can be done
      // since angle2 is in [-180, 180[
      a2_cand1 = a2_candidates.begin()->first;
      supeq_a2it = a2_candidates.end();
      advance(supeq_a2it, -1);
      a2_cand2 = supeq_a2it->first;
    }
  else
    {
      // general case
      //slog << "angle2 interp general case" << endl;
      a2_cand1 = supeq_a2it->first;
      advance(supeq_a2it, -1);
      a2_cand2 = supeq_a2it->first;
    }
  // angular distance between the 2 candidates
  float a2_cands_dist = angular_dist(a2_cand1, a2_cand2);
  // weight of the second candidate
  float a2_cand2_weight = angular_dist(a2, a2_cand1) / a2_cands_dist;
  //slog << "dist between a2 candidates" << a2_cands_dist << endl;
  //slog << "angular_dist(a2, a2_cand1)" << angular_dist(a2, a2_cand1) << endl;

  // add 2 candidates
  ic.add(a1_key, a2_cand1, weight * (1-a2_cand2_weight));
  ic.add(a1_key, a2_cand2, weight * a2_cand2_weight);
}


void HrtfCont::set_candidates(interp_cdts& ic, float az, float el)
{
  // assumption: ic.size == 0, and map size != 0, not checked for RT issues :-(

  // index angles in the hrtf map
  float	iangle1, iangle2;

  //slog << "set candidates az:" << az << ", el:" << el << endl;
  // get the angular indexes expressed in the coordianates of the HRTF db to use
  if (_vert_pol_coords)
    {
      // Database using vertical polar coordinates
      normalize_vertpolar_coords(az, el);
      iangle1 = el;
      iangle2 = az;
    }
  else
    {
      // Database using interaural polar coordiantes
      vertpol2interaurpol(az,el);
      iangle1 = az;
      iangle2 = el;
    }
  //slog << "indexes " << iangle1 << ", " << iangle2 << endl;

  // there is only one available value for first index
  if (_m.size() == 1)
    return add_a2_candidates(ic, _m.begin()->first, iangle2, 1);

  // get the first index value >= iangle1
  angle1_cit	supeq_a1it = _m.lower_bound(iangle1);

  // the requested angle1 corresponds exactly to an indexed element
  if (supeq_a1it != _m.end() && iangle1 == supeq_a1it->first)
    return add_a2_candidates(ic, iangle1, iangle2, 1);

  if (supeq_a1it == _m.begin() || supeq_a1it == _m.end())
    {
      // requested angle1 is strictly smaller than
      // the smallest index of the database
      // or strictly bigger than the biggest index of the db
      float range_without_measures;
      float w2;
      if (supeq_a1it == _m.end())
	{
	  // requested index1 is bigger than available indexes
	  advance(supeq_a1it, -1);
	  range_without_measures = 2*(90 - supeq_a1it->first);
	  w2 = iangle1 - supeq_a1it->first;
	}
      else
	{
	  // requested index1 is smaller than available indexes
	  range_without_measures = -2*(-90 - supeq_a1it->first);
	  w2 = supeq_a1it->first - iangle1;
	}
      w2 /= range_without_measures;
      add_a2_candidates(ic, supeq_a1it->first, iangle2, 1-w2);
      iangle2 = iangle2 > 0 ? iangle2 - 180 : iangle2 + 180;
      add_a2_candidates(ic, supeq_a1it->first, iangle2, w2);
    }
  else
    {
      // general case: intepolation using 2 different indexed elements
      float index1_candidate1, index1_candidate2;
      index1_candidate2 = supeq_a1it->first;
      advance(supeq_a1it, -1);
      index1_candidate1 = supeq_a1it->first;
      // distance between index1 candidates. NB: candidate 2 is bigger than candidate 1!
      float dcands = index1_candidate2 - index1_candidate1;
      add_a2_candidates(ic, index1_candidate2, iangle2, (iangle1-index1_candidate1)/dcands);
      add_a2_candidates(ic, index1_candidate1, iangle2, (index1_candidate2-iangle1)/dcands);
    }
}


// TODO: could be optimized using templates
void HrtfCont::update_from_candidates(const interp_cdts& ic, float* left, float* right)
{
  //slog << "update from candidates " << left << right << endl;
  const float	*lc, *rc;
  float		wc;
  ir_buffer*	irb;

  irb = &(_m[ic.angle_index1[0]][ic.angle_index2[0]]);
  //slog << ic.angle_index1[0] << " " << ic.angle_index2[0] << " " << irb->fname << endl;
  //  return;

  wc = ic.weight[0];
  lc = irb->lbuf;
  rc = irb->rbuf;
  //slog << "update from " << _m[ic.angle_index1[0]][ic.angle_index2[0]].fname << " " << wc << endl;
  //slog << lc << rc << left << right << endl;

  for (size_t i = 0; i < _ir_length; ++i)
    {
      left[i] = wc * lc[i];
      right[i] = wc * rc[i];
    }

  for (size_t icand = 1; icand < ic.size; ++icand)
    {
      irb = &(_m[ic.angle_index1[icand]][ic.angle_index2[icand]]);
      wc = ic.weight[icand];
      //slog << "update from " << _m[ic.angle_index1[icand]][ic.angle_index2[icand]].fname << " " << wc << endl;
      lc = irb->lbuf;
      rc = irb->rbuf;
      for (size_t i = 0; i < _ir_length; ++i)
  	{
  	  left[i] += wc * lc[i];
  	  right[i] += wc * rc[i];
  	} 
    }
  //  slog << "endof candidate update" << endl;
}


// convert an azimuth/elevation couple expressed in
// vertical polar coordinates to interaural polar coordinates
void HrtfCont::vertpol2interaurpol(float& az, float& el) const
{
  const float raz = az * DEG2RAD; 
  const float rel = el * DEG2RAD; 
  const float cosaz = cos(raz);
  const float sinaz = sin(raz);
  const float cosel = cos(rel);
  const float sinel = sin(rel);
  const float cosazcosel = cosaz * cosel;

  el = atan2(sinel, cosazcosel) * RAD2DEG;
  az = atan2(cosel * sinaz, sqrt(cosazcosel * cosazcosel + sinel*sinel)) * RAD2DEG;
}
