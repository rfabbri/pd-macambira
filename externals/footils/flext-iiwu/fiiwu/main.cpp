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

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 300)
#error You need at least flext version 0.3.0
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
			
			AddInAnything();         // 2 audio ins
			AddOutSignal(2);         // 1 audio out [ == AddOutSignal(1) ]
			
			SetupInOut();           // set up inlets and outlets. 
			                        // Must be called once!
			// IIWU_DEFAULT_SETTINGS;	
			iiwu_synth_settings_t pd_iiwu_settings = 
				{ IIWU_SETTINGS_VERSION, 128, 0, NULL, NULL};
			
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
			
			// list input calls iiwu_note(...)
			FLEXT_ADDMETHOD_(0, "list",  iiwu_note);
			
			// alias shortcuts:
			FLEXT_ADDMETHOD_(0,"n",  iiwu_note);
			FLEXT_ADDMETHOD_(0,"p",  iiwu_program_change);
			FLEXT_ADDMETHOD_(0,"c",  iiwu_control_change);
			FLEXT_ADDMETHOD_(0,"cc", iiwu_control_change);
			
			
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

}; // end of class declaration for fiiwu


// Before we can run our fiiwu-class in PD, the object has to be registered as a
// PD object. Otherwise it would be a simple C++-class, and what good would
// that be for?  Registering is made easy with the FLEXT_NEW_* macros defined
// in flext.h. For tilde objects without arguments call:

FLEXT_NEW_TILDE_G("fiiwu~", fiiwu)


void fiiwu::iiwu_load(int argc, t_atom *argv)
{
	if (synth == NULL) return;
	
	if (argc == 1 && IsSymbol(argv[0]))	
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
	int   chan, key, vel;
	chan  = GetAInt(argv[0]);
	key   = GetAInt(argv[1]);
	vel   = GetAInt(argv[2]);
	iiwu_synth_noteon(synth,chan-1,key,vel);    
}

void fiiwu::iiwu_program_change(int argc, t_atom *argv)
{	
	if (synth == NULL) return;
	int   chan, prog;
	chan  = GetAInt(argv[0]);
	prog  = GetAInt(argv[1]);
	iiwu_synth_program_change(synth,chan-1,prog);
}

void fiiwu::iiwu_control_change(int argc, t_atom *argv)
{
	if (synth == NULL) return;
	int   chan, ctrl, val;
	chan  = GetAInt(argv[0]);
	ctrl  = GetAInt(argv[1]);
	val   = GetAInt(argv[1]);
	iiwu_synth_cc(synth,chan-1,ctrl,val);
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
