//#=====================================================================================
//#
//#       Filename:  main.cpp
//#
//#    Description:  fiiwu~ - external for PD
//#
//#        Version:  1.0
//#        Created:  08/20/02
//#       Revision:  none
//#
//#         Author:  Frank Barknecht  (fbar)
//#          Email:  fbar@footils.org
//#      Copyright:  Frank Barknecht , 2002
//#
//#=====================================================================================


#include <flext.h>
#include <iiwusynth.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 303)
#error You need at least flext version 0.3.3
#endif


// A flext dsp external ("tilde object") inherits from the class flext_dsp 
class fiiwu: 
	public flext_dsp
{
	// Each external that is written in C++ needs to use #defines 
	// from flbase.h
	// 
	// The define
	// 
	// FLEXT_HEADER(NEW_CLASS, PARENT_CLASS)
	// 
	// should be somewhere in your dsp file.
	// A good place is here:
	
	FLEXT_HEADER(fiiwu, flext_dsp)

	public:
		fiiwu(int argc, t_atom *argv) 
		{
			// The constructor of your class is responsible for
			// setting up inlets and outlets and for registering
			// inlet-methods:
			
			AddInAnything();         // slurp anything
			AddOutSignal(2);         // 2 audio out [ == AddOutSignal(2) ]
			
			SetupInOut();           // set up inlets and outlets. 
			                        // Must be called once!
			
			float sr=Samplerate();
			
			if (sr != 44100.f) 
			{
				post("WARNING: Current samplerate %.0f != 44100", sr);
				post("WARNING: fiiwu~ might be out of tune!");
			}
			
			iiwu_synth_settings_t pd_iiwu_settings = IIWU_DEFAULT_SETTINGS;
			
			// plugin synth: AUDIO off
			pd_iiwu_settings.flags        &= ~IIWU_AUDIO;
			pd_iiwu_settings.sample_format = IIWU_FLOAT_FORMAT;
			if (sr != 0)
			{
				pd_iiwu_settings.sample_rate   = static_cast<int>(sr);
			}
			
	
			// Create iiwusynth instance:
			synth = new_iiwu_synth(&pd_iiwu_settings);
			
			if ( synth == NULL )
			{
			        post("fiiwu~: couldn't create synth\n");
			}
			
			// try to load argument as soundfont
			iiwu_load(argc, argv);


			FLEXT_ADDMETHOD_(0,"load", iiwu_load);
			FLEXT_ADDMETHOD_(0,"note", iiwu_note);
			FLEXT_ADDMETHOD_(0,"prog", iiwu_program_change);
			FLEXT_ADDMETHOD_(0,"control", iiwu_control_change);
			FLEXT_ADDMETHOD_(0,"bend", iiwu_pitch_bend);
			FLEXT_ADDMETHOD_(0,"bank",  iiwu_bank);
			
			// list input calls iiwu_note(...)
			FLEXT_ADDMETHOD_(0, "list",  iiwu_note);
			
			// some alias shortcuts:
			FLEXT_ADDMETHOD_(0,"n",  iiwu_note);
			FLEXT_ADDMETHOD_(0,"p",  iiwu_program_change);
			FLEXT_ADDMETHOD_(0,"c",  iiwu_control_change);
			FLEXT_ADDMETHOD_(0,"cc", iiwu_control_change);
			FLEXT_ADDMETHOD_(0,"b",  iiwu_pitch_bend);
			
			
			// We're done constructing:
			post("-- fiiwu~ with flext ---");
	
		} // end of constructor
		~fiiwu()
		{
			if ( synth != NULL )
			{
			        delete_iiwu_synth(synth);
			}
		}
	
	
		virtual void m_help()
		{
			const char * helptext = 
			"_ __fiiwu~_ _  a soundfont external for Pd and Max/MSP \n"
			"_ argument: \"/path/to/soundfont.sf\" to load on object creation\n"
			"_ messages: \n"
			"load /path/to/soundfont.sf2  --- Loads a Soundfont \n"
			"note 0 0 0                   --- Play note. Arguments: \n"
			"                                 channel-# note-#  veloc-#\n"
			"n 0 0 0                      --- Play note, same as above\n"
			"0 0 0                        --- Play note, same as above\n"
			;
			post("%s", helptext);

		}
	protected:
		// here we declare the virtual DSP function
		virtual void m_signal(int n, float *const *in, float *const *out);
		
	private:	
		iiwu_synth_t *synth;
		
		FLEXT_CALLBACK_G(iiwu_load)
		void iiwu_load(int argc, t_atom *argv);
		
		FLEXT_CALLBACK_G(iiwu_note)
		void iiwu_note(int argc, t_atom *argv);
		
		FLEXT_CALLBACK_G(iiwu_program_change)
		void iiwu_program_change(int argc, t_atom *argv);
		
		FLEXT_CALLBACK_G(iiwu_control_change)
		void iiwu_control_change(int argc, t_atom *argv);
		
		FLEXT_CALLBACK_G(iiwu_pitch_bend)
		void iiwu_pitch_bend(int argc, t_atom *argv);
		
		FLEXT_CALLBACK_G(iiwu_bank)
		void iiwu_bank(int argc, t_atom *argv);

}; // end of class declaration for fiiwu


// Before we can run our fiiwu-class in PD, the object has to be registered as a
// PD object. Otherwise it would be a simple C++-class, and what good would
// that be for?  Registering is made easy with the FLEXT_NEW_* macros defined
// in flext.h. 

FLEXT_NEW_TILDE_G("fiiwu~", fiiwu)


void fiiwu::iiwu_load(int argc, t_atom *argv)
{
	if (synth == NULL) return;
	
	if (argc >= 1 && IsSymbol(argv[0]))	
	{
		const char* filename = GetString(argv[0]);
		if ( iiwu_synth_sfload(synth, filename) == 0)
		{
			post("Loaded Soundfont: %s", filename);
		}
	}
}

void fiiwu::iiwu_note(int argc, t_atom *argv)
{
	if (synth == NULL) return;
	if (argc == 3)
	{
		int   chan, key, vel;
		chan  = GetAInt(argv[0]);
		key   = GetAInt(argv[1]);
		vel   = GetAInt(argv[2]);
		iiwu_synth_noteon(synth,chan-1,key,vel);
	}
}

void fiiwu::iiwu_program_change(int argc, t_atom *argv)
{	
	if (synth == NULL) return;
	if (argc == 2)
	{
		int   chan, prog;
		chan  = GetAInt(argv[0]);
		prog  = GetAInt(argv[1]);
		iiwu_synth_program_change(synth,chan-1,prog);
	}
}

void fiiwu::iiwu_control_change(int argc, t_atom *argv)
{
	if (synth == NULL) return;
	if (argc == 3)
	{
		int   chan, ctrl, val;
		chan  = GetAInt(argv[0]);
		ctrl  = GetAInt(argv[1]);
		val   = GetAInt(argv[2]);
		iiwu_synth_cc(synth,chan-1,ctrl,val);
	}
}

void fiiwu::iiwu_pitch_bend(int argc, t_atom *argv)
{
	if (synth == NULL) return;
	if (argc == 2)
	{
		int   chan, val;
		chan  = GetAInt(argv[0]);
		val   = GetAInt(argv[1]);
		iiwu_synth_pitch_bend(synth, chan-1, val);
	}
}

void fiiwu::iiwu_bank(int argc, t_atom *argv)
{
	if (synth == NULL) return;
	if (argc == 2)
	{	
		int   chan, bank;
		chan  = GetAInt(argv[0]);
		bank  = GetAInt(argv[1]);
		iiwu_synth_bank_select(synth, chan-1, bank);
	}
}



// Now we define our DSP function. It gets this arguments:
// 
// int n: length of signal vector. Loop over this for your signal processing.
// float *const *in, float *const *out: 
//          These are arrays of the signals in the objects signal inlets rsp.
//          oulets. We come to that later inside the function.

void fiiwu::m_signal(int n, float *const *in, float *const *out)
{
	
	float *left  = out[0];
	float *right = out[1];
	
	iiwu_synth_write_float(synth, n, left, 0, 1, right, 0, 1); 
	
}  // end m_signal
