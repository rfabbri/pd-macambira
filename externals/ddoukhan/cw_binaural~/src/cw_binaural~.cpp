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

#include <m_pd.h>
#include "binaural_processor.hpp"
#include "logstring.hpp"

static t_class *cw_binaural_tilde_class;

typedef struct _cw_binaural_tilde
{
	t_object		x_obj;
	BinauralProcessor	*bp;
	float			default_input;
}	t_cw_binaural_tilde;


t_int *cw_binaural_tilde_perform(t_int *w)
{
  t_cw_binaural_tilde *x = (t_cw_binaural_tilde *)(w[1]);

  // 3 signal input: signal, azimuth and elevation
  t_sample  *input = (t_sample *)(w[2]);
  t_sample  *azimuths = (t_sample *)(w[3]);
  t_sample  *elevations = (t_sample *)(w[4]);

  // 2 signal outputs: for the left and right ear
  t_sample *left_output = (t_sample *)(w[5]);
  t_sample *right_output = (t_sample *)(w[6]);

  int n = (int)(w[7]);
  
  x->bp->process(input, azimuths, elevations, left_output, right_output, n);

  return (w+8);
}





void cw_binaural_tilde_dsp(t_cw_binaural_tilde *x, t_signal **sp)
{
  dsp_add(cw_binaural_tilde_perform, 7, x, sp[0]->s_vec, sp[1]->s_vec,
	  sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
	  sp[0]->s_n);
}

// called when the message listen_db is sent to the object
void cw_binaural_set_listen_db(t_cw_binaural_tilde *x, t_symbol *hrtf_path)
{
  try
    {
      x->bp->set_listen_db(string(hrtf_path->s_name));
    }
  catch (int n) {}
  logstring2pdterm();
}

// called when the message cipic_db is sent to the object
void cw_binaural_set_cipic_db(t_cw_binaural_tilde *x, t_symbol *hrtf_path)
{
  try
    {
      x->bp->set_cipic_db(string(hrtf_path->s_name));
    }
  catch (int n) {}
  logstring2pdterm();
}

// convert a symbol to boolean
static bool	symb2bool(t_symbol* symb)
{
  string	s(symb->s_name);

  for (size_t i = 0; i < s.length(); ++i)
    s[i] = toupper(s[i]);
  return s == "TRUE";
}

// called when the message sethrtf is sent to the object
// message allowing to load any hrtf database
// hrtf_path correspond to the path to the directory storing the hrtf as wav files
// fname_regex is a regex representing the name of files containing impulse responses
// it should contain a group for each parameter to extract (azimuth and elevation)
// az_first tells wether the first group corresponds to azimuth or not
// vertical_polar_coords is set to true if the database is expressed in vertical polar
// coordinates, if set to false, it refers interaural polar coordinates
void cw_binaural_set_hrtf_db(t_cw_binaural_tilde *x,
			     t_symbol *symb_hrtf_path,
			     t_symbol *symb_fname_regex,
			     t_symbol *symb_az_first,
			     t_symbol *symb_vertical_polar_coords)
{
  string	hrtf_path(symb_hrtf_path->s_name);
  string	fname_regex(symb_fname_regex->s_name);
  bool		az_first = symb2bool(symb_az_first);
  bool		vertical_polar_coords = symb2bool(symb_vertical_polar_coords);

  // Pd does not allows to transmit backslashes in messages,
  // so we replace each occurence of double slashes with a back slash
  size_t	found;
  while ((found = fname_regex.find("//")) != string::npos)
    fname_regex.replace(found, 2, 1, '\\');

  try 
    {
      x->bp->set_hrtf(hrtf_path, fname_regex, az_first, vertical_polar_coords);
    }
  catch (int n) {}
  logstring2pdterm();
}



void cw_binaural_tilde_free(t_cw_binaural_tilde *x)
{
  if (x->bp)
    delete x->bp;
  logstring2pdterm();
}

void *cw_binaural_tilde_new(t_symbol *obj_name, int argc, t_atom *argv)

{
  // default creation arguments
  int impulse_response_size = 128;
  std::string filtering_method = std::string("RIFF");
  std::string delay_method = std::string("Hermite4");
  
  // parsing creation arguments
  switch (argc < 3 ? argc : 3)
    {
    case 3:
      delay_method = std::string(atom_getsymbol(argv + 2)->s_name);
    case 2:
      filtering_method = std::string(atom_getsymbol(argv + 1)->s_name);
    case 1:
      impulse_response_size = (int) atom_getfloat(argv);
    default:
      break;
    }

  // creating object
  t_cw_binaural_tilde *x = (t_cw_binaural_tilde *)pd_new(cw_binaural_tilde_class);
  x->default_input = 0;
  x->bp = NULL;

  try
    {
      // call to the constructor
      x->bp = new BinauralProcessor(impulse_response_size, filtering_method, delay_method);
    }
  catch (int n)
    {
      x->bp = 0;
      logstring2pdterm();
      return (void*) 0;
    }

  // azimuth inlet
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  // elevation inlet
  inlet_new(&x->x_obj , &x->x_obj.ob_pd, &s_signal, &s_signal);

  // left outlet
  outlet_new(&x->x_obj, &s_signal);
  // right outlet
  outlet_new(&x->x_obj, &s_signal);

  logstring2pdterm();

  return (void *)x;
}


extern "C"
#ifdef WIN32
__declspec(dllexport) void cw_binaural_tilde_setup(void)
#else
  void cw_binaural_tilde_setup(void)
#endif
{
  // creation of the cw_binaural~ instance
  cw_binaural_tilde_class =
    class_new(gensym("cw_binaural~"),
	      (t_newmethod)cw_binaural_tilde_new,
	      (t_method) cw_binaural_tilde_free, sizeof(t_cw_binaural_tilde),
	      CLASS_DEFAULT, 
	      A_GIMME, 0);
  
  // sound processing
  class_addmethod(cw_binaural_tilde_class,
		  (t_method)cw_binaural_tilde_dsp, gensym("dsp"), (t_atomtype) 0);

  // management of messages used to set a listen database
  class_addmethod(cw_binaural_tilde_class,
		  (t_method)cw_binaural_set_listen_db,
		  gensym("listen_db"),
		  A_DEFSYMBOL, (t_atomtype) 0);

  // management of messages used to set a CIPIC database
  class_addmethod(cw_binaural_tilde_class,
		  (t_method)cw_binaural_set_cipic_db,
		  gensym("cipic_db"),
		  A_DEFSYMBOL, (t_atomtype) 0);

  // management of messages used to set any database
  class_addmethod(cw_binaural_tilde_class,
		  (t_method)cw_binaural_set_hrtf_db,
		  gensym("set_hrtf_db"),
		  A_DEFSYMBOL, A_DEFSYMBOL, A_DEFSYMBOL, A_DEFSYMBOL, (t_atomtype) 0);
		
  CLASS_MAINSIGNALIN(cw_binaural_tilde_class, t_cw_binaural_tilde, default_input);
}

