/*
    cw_binaural~: a binaural synthesis external for pure data
    by David Doukhan - david.doukhan@gmail.com - http://perso.limsi.fr/doukhan
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


#include <regex.h>
#include <stdlib.h>
#include <dirent.h>
#include <sndfile.h>
#include <assert.h>
#include "raw_wav_hrtfcont.hpp"
#include "logstring.hpp"



void RawWavHrtfCont::set_ir_buffer_from_path(ir_buffer& irb, string path)
{
  SF_INFO	sfinfo;
  SNDFILE*	sndfd;

  // slog << "Trying " << path << std::endl;
  //irb.fname = path.c_str();

  // opening the sound file
  sfinfo.format = 0;
  sndfd = sf_open(path.c_str(), SFM_READ, &sfinfo);
  if (!sndfd)
    {
      slog << "failed to open " << path << endl;
      throw 0;
    }

  //slog << sfinfo.channels << endl;
  if (sfinfo.channels != 2)
    {
      slog << path << " has " << sfinfo.channels << " channels instead of 2!" << endl;
      throw 0;
    }
  assert(sfinfo.samplerate == 44100);

  //slog << "frames" << sfinfo.frames << endl;
  const size_t wav_length = sfinfo.frames;// / 2;
  //slog << "ir length before" << _ir_length << endl;
  if (!_ir_length)
    // we're extrating the first impulse response
    // all other impulse response extrated
    // must have the same length
    _ir_length = wav_length;
  //slog << "ir length" << _ir_length << endl;

  // if the considered file is empty
  // or if its size is different from previously
  // extracted impulse response files
  assert(wav_length && (wav_length == _ir_length));

  float* wavdata = new float[sfinfo.frames*2];
  sf_read_float(sndfd, wavdata, sfinfo.frames*2);

  irb.lbuf = new float[wav_length];
  irb.rbuf = new float[wav_length];


  for (size_t i = 0; i < wav_length; ++i)
    {
      irb.lbuf[i] = wavdata[i*2];
      irb.rbuf[i] = wavdata[i*2+1];
    }

  delete [] wavdata;
  sf_close(sndfd);
}

// constructor should parse the directory
// deduce the how many different azimuths
// and elevations are present, according
// to a given regexp
// => intermediate list structure
// then use a faster structure to store
// the extracted wavs!!!
// FIXME: check the map size is != 0
RawWavHrtfCont::RawWavHrtfCont(const ir_key& k):
  HrtfCont(k)
{
  // regex vars
  int		re_status;
  regex_t	re;
  regmatch_t	pmatch[3];
  // directory parsing vars
  DIR		*dp;
  struct dirent *dirp;
  // other
  float		az, elev;
  
  _ir_length = 0;

  // slog << "Raw Wav HRTF CONT " << k.iir_regex << endl;
  // compile the regex
  if (regcomp(&re, k.iir_regex.c_str(), REG_EXTENDED|REG_ICASE))
    {
      slog << "regex: " << k.iir_regex << " is not supported!" << endl;
      throw 0;
    }

  // test existence of directory supposed to store hrtf impulse responses
  if((dp  = opendir(k.path.c_str())) == NULL)
    {
      slog << "directory " << k.path << " does not exist!" << endl;
      throw 0;
    }

  // extract wav files matching the regex
  // contained in path
  while ((dirp = readdir(dp)))
    //{ slog << dirp->d_name << endl;
    if (!regexec(&re, dirp->d_name, 3, pmatch, REG_NOTBOL|REG_NOTEOL))
      {
	//slog << "match!" << endl;
	string dirname(dirp->d_name);
	string group0 = dirname.substr(pmatch[1].rm_so, pmatch[1].rm_eo);
	string group1 = dirname.substr(pmatch[2].rm_so, pmatch[2].rm_eo);
	
	if (k.is_azimuth_first)
	  {
	    az = atof(group0.c_str());
	    elev = atof(group1.c_str());
	  }
	else
	  {
	    az = atof(group1.c_str());
	    elev = atof(group0.c_str());
	  }
	// slog << "(az,elev)=(" << az << ", " << elev << ") for file " << dirname << endl;
	
	if (k.vertical_polar_coords)
	  {
	    normalize_vertpolar_coords(az, elev);
	    set_ir_buffer_from_path(_m[elev][az], k.path + "/" + dirname);
	  }
	else
	  {
	    // TO BE IMPROVED
	    if (elev > 180)
	      elev -= 360;
	    set_ir_buffer_from_path(_m[az][elev], k.path + "/" + dirname);
	  }

      }
  
  // close the directory structure
  closedir(dp);

  // free the regex
  regfree(&re);

  //slog << "END OF RAW WAV HRTF CONT" << endl;
}


