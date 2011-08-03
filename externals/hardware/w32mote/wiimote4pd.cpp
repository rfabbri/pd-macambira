#ifdef _WIN32
# define MSW
// linker->befehlsausgabe: zusätzliche flags: "/export:wiimote_setup"
#endif


// TODO: 
// use RefreshState(): copy internal state to actual instance

#ifdef  _MSC_VER
# pragma warning(disable: 4091)
# define WIIEXTERN __declspec(dllexport) extern
#else
# define WIIEXTERN extern
#endif

#include <m_pd.h>
#include "wiimote.h"

#include <map>

// class and struct declarations for wiimote pd external:
class wiimote4pd : public wiimote {
public:
private:
	t_object*m_obj;
	t_outlet *m_dataOut;
	t_outlet *m_statusOut;

	t_clock  *m_clock;

	bool  m_changed;
	typedef std::pair<std::string, std::vector<t_float> > datavec;
	std::vector < datavec> m_data;

	bool m_reportAccel, m_reportIR, m_reportNunchuk, m_reportClassic, m_reportBalance, m_reportMPlus, m_reportExt;

	bool m_connected;

	CRITICAL_SECTION DataLock;

public: 
	wiimote4pd(t_object*obj, t_outlet*dOut, t_outlet*sOut) : wiimote(), 
		m_obj(obj),
		m_dataOut(dOut), m_statusOut(sOut),
		m_clock(NULL),
		m_changed(NO_CHANGE),
		m_reportAccel(false), m_reportIR(false),
		m_reportNunchuk(false), m_reportClassic(false), m_reportBalance(false), 
		m_reportMPlus(false), 
		m_reportExt(false),
		m_connected(false)
	{
		InitializeCriticalSection(&DataLock);
		m_clock = clock_new(this, (t_method)wiimote_tickCallback);
	}
	virtual ~wiimote4pd(void) {
		clock_free(m_clock);
		DeleteCriticalSection(&DataLock);
	}

	static void wiimote_tickCallback(wiimote4pd*x) {
		x->tick();
	}

	void error(std::string s) {
		if(m_obj)
			pd_error(m_obj, "%s", s.c_str());
		else
			::error("[wiimote] %s", s.c_str());
	}

	void state(void) {
		t_atom atom;
		SETFLOAT(&atom, m_connected);
		outlet_anything(m_statusOut, gensym("open"), 1, &atom);
	};
	void state(bool connected) {
		m_connected=connected;
		state();
	};

	void data(const std::string&id, std::vector<t_float>&d) {
		const unsigned int len=d.size();
		t_atom*atoms=new t_atom[len];
		unsigned int i=0;
		for(i=0; i<len; i++) {
			SETFLOAT(atoms+i, d[i]);
		}
		outlet_anything(m_dataOut, gensym(id.c_str()), len, atoms);
		delete[]atoms;
	};

	void data(const std::string id, t_float f) {
		t_atom atom;
		SETFLOAT(&atom, f);
		outlet_anything(m_dataOut, gensym(id.c_str()), 1, &atom);
	}
	void data(const std::string id, t_float f0, t_float f1) {
		t_atom atoms[2];
		SETFLOAT(atoms+0, f0);
		SETFLOAT(atoms+1, f1);
		outlet_anything(m_dataOut, gensym(id.c_str()), 2, atoms);
	}
	void data(const std::string id, t_float f0, t_float f1, t_float f2) {
		t_atom atoms[3];
		SETFLOAT(atoms+0, f0);
		SETFLOAT(atoms+1, f1);
		SETFLOAT(atoms+2, f2);
		outlet_anything(m_dataOut, gensym(id.c_str()), 3, atoms);
	}


	void Disconnect(void) {
		wiimote::Disconnect();
		state(false);
	}

	void Connect(unsigned wiimote_index = FIRST_AVAILABLE) {
		m_connected=IsConnected();
		if(m_connected) {
			error("already connected to WiiMote");
			state();
			return;
		}
		post("discovering WiiMote");
		m_connected=wiimote::Connect( wiimote_index );
		if(!m_connected) {
			error("could not find any wiimotes. Please ensure that your WiiMote is associated with your system");
		}
		post("total connected WiiMotes: %d", TotalConnected());
		state();
	}

	void updateReport(void) {
#if 0
		m_reportAccel(false), m_reportIR(false),
		m_reportNunchuk(false), m_reportClassic(false), m_reportBalance(false), m_reportMPlus(false), m_reportExt(false)

			IN_BUTTONS				 = 0x30,
			IN_BUTTONS_ACCEL		 = 0x31, 001 
			IN_BUTTONS_ACCEL_IR		 = 0x33, 011 // reports IR EXTENDED data (dot sizes)
			IN_BUTTONS_ACCEL_EXT	 = 0x35, 101
			IN_BUTTONS_ACCEL_IR_EXT	 = 0x37, 111 // reports IR BASIC data (no dot sizes)
			IN_BUTTONS_BALANCE_BOARD = 0x32, 010 // must use this for the balance board
#endif

			input_report reportmode=wiimote::IN_BUTTONS;

			if(m_reportNunchuk || m_reportClassic || m_reportMPlus || m_reportExt || m_reportBalance) {
				error("Extension reporting currently not supported");
			}
			if(m_reportIR && !m_reportAccel) {
				post("cannot report IR without acceleration");
			}
			if(m_reportAccel && m_reportIR) 
				reportmode=wiimote::IN_BUTTONS_ACCEL_IR_EXT;
			else if(m_reportAccel)
				reportmode=wiimote::IN_BUTTONS_ACCEL;
				

		SetReportType(reportmode);
	}

	void report(const std::string id, bool state) {
		if(!id.compare("acceleration"))m_reportAccel=state;
		else if(!id.compare("acceleration"))m_reportAccel=state;
		else if(!id.compare("ir"))m_reportIR=state;
		else if(!id.compare("nunchuk"))m_reportNunchuk=state;
		else if(!id.compare("classic"))m_reportClassic=state;
		else if(!id.compare("balance"))m_reportBalance=state;
		else if(!id.compare("motionplus"))m_reportMPlus=state;
		else if(!id.compare("ext"))m_reportExt=state;
		//else if(!id.compare("state"))m_reportState=state;
		//else if(!id.compare("button"))m_reportButton=state;
		else {
			std::string err="unknown report mode '";
			err+=id;
			err+="'";
			error(err);
		}
		updateReport();
	}


	inline void lock(void) {
		EnterCriticalSection(&DataLock);
	}
	inline void unlock(void) {
		LeaveCriticalSection(&DataLock);
	}

	virtual void			ChangedNotifier (state_change_flags  changed,
		const wiimote_state &new_state) {
#define ADDvMSG(VEC, id) do {datavec dv; dv.first=id; dv.second=v; VEC.push_back(dv); v.clear();} while(0)

			// merge the new changed set with the cached one
			lock();
			bool pristine=(m_changed==false);
			m_changed = true;
			std::vector<t_float>v;
			// Wiimote specific) {
			if(changed & CONNECTION_LOST) {
				v.push_back(0);
				ADDvMSG(m_data, "state");
			} if(changed & CONNECTED) {
				v.push_back(1);
				ADDvMSG(m_data, "state");
			} if(changed & BATTERY_CHANGED || changed & BATTERY_DRAINED) {
				v.push_back(new_state.BatteryPercent * 0.01f);
				ADDvMSG(m_data, "battery");
			} if(changed & BUTTONS_CHANGED) {
				unsigned char but1 = (new_state.Button.Bits & 0xFF00)>>8;
				unsigned char but2 = (new_state.Button.Bits & 0x00FF);
				v.push_back(but2);
				v.push_back(but1);
				ADDvMSG(m_data, "button");
			} if(changed & ACCEL_CHANGED) {
				v.push_back(new_state.Acceleration.X);
				v.push_back(new_state.Acceleration.Y);
				v.push_back(new_state.Acceleration.Z);
				ADDvMSG(m_data, "acceleration");
#if 0
			} if(changed & ORIENTATION_CHANGED) {
				// this is a cooked version of ACCEL
			} if(changed & LEDS_CHANGED) {
				// they won't change on their own, would they?
#endif
			} if(changed & IR_CHANGED) {
				unsigned int i;
				for(i=0; i<4; i++) {
					const wiimote_state::ir::dot &dot = new_state.IR.Dot[i];
					if(dot.bVisible) {
						v.push_back(i);
						v.push_back(dot.RawX);
						v.push_back(dot.RawY);
						v.push_back(dot.Size);
						ADDvMSG(m_data, "ir");
					}
				}
				// - Extensions -
				//  Nunchuk
#if 0
			} if(changed & NUNCHUK_CONNECTED) {

			} if(changed & NUNCHUK_BUTTONS_CHANGED) {
			} if(changed & NUNCHUK_ACCEL_CHANGED) {
			} if(changed & NUNCHUK_ORIENTATION_CHANGED) {
			} if(changed & NUNCHUK_JOYSTICK_CHANGED) {

				//  Classic Controller (inc. Guitars etc)
			} if(changed & CLASSIC_CONNECTED) {
			} if(changed & CLASSIC_BUTTONS_CHANGED) {
			} if(changed & CLASSIC_JOYSTICK_L_CHANGED) {
			} if(changed & CLASSIC_JOYSTICK_R_CHANGED) {
			} if(changed & CLASSIC_TRIGGERS_CHANGED) {

				//  Balance Board
			} if(changed & BALANCE_CONNECTED) {
			} if(changed & BALANCE_WEIGHT_CHANGED) {

				//  Motion Plus
			} if(changed & MOTIONPLUS_DETECTED) {
			} if(changed & MOTIONPLUS_ENABLED) {
			} if(changed & MOTIONPLUS_SPEED_CHANGED) {
			} if(changed & MOTIONPLUS_EXTENSION_CONNECTED) {
			} if(changed & MOTIONPLUS_EXTENSION_DISCONNECTED) {
#endif
			}
			unlock();
			if(pristine) {
				sys_lock();
				clock_delay(m_clock, 0);
				sys_unlock();
			}
	}

	void tick(void) {
		lock();
		m_changed=false;
		unsigned int i;
		for(i=0; i<m_data.size(); i++) {
			datavec dv=m_data[i];
			data(dv.first, dv.second);
		}
		m_data.clear();
		unlock();

	}

	bool array2sample(const std::string&arrayname, wiimote_sample&result, const speaker_freq freq=FREQ_3130HZ) {
		t_garray *a = (t_garray *)pd_findbyclass(gensym(arrayname.c_str()), garray_class);
		if(!a) {
			std::string err="no such array";
			err+=": '";
			err+=arrayname;
			err+="'";
			error(err);
			return false;
		}
		int length=0;
		t_word *vec=NULL;
		if(!garray_getfloatwords(a, &length, &vec)) {
			std::string err="bad template for array";
			err+=": '";
			err+=arrayname;
			err+="'";
			error(err);
			return false;
		}
		if(length<1 || NULL==vec) return false;
		signed short *samples=new signed short[length];
		int i;
		for(i=0; i<length; i++) {
			signed short s=static_cast<signed short>(32768.*vec[i].w_float);
			samples[i]=s;
		}
		bool res=Convert16bitMonoSamples  (samples, true, length, freq, result);
		delete[]samples;
		return res;
	}

	wiimote_sample m_sample;
	void play(const std::string&arrayname) {
		bool res=array2sample(arrayname, m_sample);
		res=PlaySample(m_sample);
	}


};
static t_class *wiimote_class;
typedef struct _wiimote
{
	t_object x_obj; // standard pd object (must be first in struct)
	wiimote4pd*x_wiimote;
	t_outlet*x_dataOut,*x_statusOut;
} t_wiimote;

static void wiimote_ctor(t_wiimote*x){
	if(!x->x_wiimote) {
		try {
			x->x_wiimote=new wiimote4pd(&x->x_obj, x->x_dataOut, x->x_statusOut);
		} catch (int fourtytwo) {
			error("ouch! wiimote allocation failed fatally");
		}
	}
}

static void wiimote_dtor(t_wiimote*x){
	if(x->x_wiimote) {
		delete x->x_wiimote;
	}
}


static void wiimote_report(t_wiimote*x, t_symbol*s, t_float f)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->report(s->s_name, f>=0.5);
#if 0
	int flag=-1;
	if(gensym("status")==s) flag=CWIID_RPT_STATUS;
	else if(gensym("button")==s) flag=CWIID_RPT_BTN;
	else if(gensym("acceleration")==s) flag=CWIID_RPT_ACC;
	else if(gensym("ir")==s) flag=CWIID_RPT_IR;
	else if(gensym("nunchuk")==s) flag=CWIID_RPT_NUNCHUK;
	else if(gensym("classic")==s) flag=CWIID_RPT_CLASSIC;
	else if(gensym("balance")==s) flag=CWIID_RPT_BALANCE;
	else if(gensym("motionplus")==s) flag=CWIID_RPT_MOTIONPLUS;
	else if(gensym("ext")==s) flag=CWIID_RPT_EXT;
	else {
		pd_error(x, "unknown report mode '%s'", s->s_name);
	}

	if(flag!=-1) {
		if(onoff) {
			x->reportMode |= flag;
		} else {
			x->reportMode &= ~flag;
		}
	}
	wiimote_resetReportMode(x);
#endif
}
static void wiimote_reportAcceleration(t_wiimote *x, t_floatarg f)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->report("acceleration", f>=0.5);
}
static void wiimote_reportIR(t_wiimote *x, t_floatarg f)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->report("ir", f>=0.5);
}
static void wiimote_reportNunchuk(t_wiimote *x, t_floatarg f)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->report("nunchuk", f>=0.5);
}
static void wiimote_reportMotionplus(t_wiimote *x, t_floatarg f)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->report("motionplus", f>=0.5);
}
static void wiimote_setReportMode(t_wiimote*x, t_floatarg r) {
	if(!x->x_wiimote)return;
	x->x_wiimote->error("setReportMode not implemented");
}
static void wiimote_setLED(t_wiimote *x, t_floatarg f)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->SetLEDs(static_cast<unsigned char>(f));
}
static void wiimote_setRumble(t_wiimote *x, t_floatarg f)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->SetRumble(f > 0.5f);
}
// ==============================================================

// The following function attempts to discover a wiimote. It requires
// that the user puts the wiimote into 'discoverable' mode before being
// called. This is done by pressing the red button under the battery
// cover, or by pressing buttons 1 and 2 simultaneously.
// TODO: Without pressing the buttons, I get a segmentation error. So far, I don't know why.

static void wiimote_discover(t_wiimote *x)
{
    wiimote_ctor(x);
	x->x_wiimote->Connect();
}

static void wiimote_disconnect(t_wiimote *x)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->Disconnect();
	delete x->x_wiimote;x->x_wiimote=NULL;
}


static void wiimote_play(t_wiimote* x, t_symbol*s) {
	if(!x->x_wiimote)return;
	x->x_wiimote->play(s->s_name);
}

static void wiimote_bang(t_wiimote *x)
{
	if(!x->x_wiimote)return;
	x->x_wiimote->tick();
}

// ==============================================================
// ==============================================================
static void *wiimote_new(t_symbol*s, int argc, t_atom *argv)
{
	t_wiimote *x = NULL;
	x=(t_wiimote *)pd_new(wiimote_class);

	x->x_wiimote = NULL;
	x->x_dataOut  =outlet_new(&x->x_obj, 0);
	x->x_statusOut=outlet_new(&x->x_obj, 0);

	wiimote_ctor(x);

	return (x);
}

static void wiimote_free(t_wiimote* x)
{
	delete x->x_wiimote;
	x->x_wiimote=NULL;
	outlet_free(x->x_dataOut  ); x->x_dataOut  = NULL;
	outlet_free(x->x_statusOut); x->x_statusOut= NULL;

}

extern "C" {
	WIIEXTERN void wiimote_setup(void) {
		wiimote_class = class_new(gensym("wiimote"), (t_newmethod)wiimote_new, (t_method)wiimote_free, sizeof(t_wiimote), CLASS_DEFAULT, A_GIMME, 0);
#if 0
		class_addmethod(wiimote_class, (t_method) wiimote_debug, gensym("debug"), 0);
		class_addmethod(wiimote_class, (t_method) wiimote_status, gensym("status"), 0);


		/* connection settings */
		//   class_addmethod(wiimote_class, (t_method) wiimote_doConnect, gensym("connect"), A_DEFSYMBOL, A_DEFSYMBOL, 0);

		/* query data */
		//... 
#endif
		class_addbang(wiimote_class, (t_method) wiimote_bang);

		/* connection handling */
		class_addmethod(wiimote_class, (t_method) wiimote_disconnect, gensym("disconnect"), A_NULL);
		class_addmethod(wiimote_class, (t_method) wiimote_discover, gensym("discover"), A_NULL);

		/* activate WiiMote stuff */
		class_addmethod(wiimote_class, (t_method) wiimote_setLED, gensym("setLED"), A_FLOAT, 0);
		class_addmethod(wiimote_class, (t_method) wiimote_setRumble, gensym("setRumble"), A_FLOAT, 0);

		/* report modes */
		class_addmethod(wiimote_class, (t_method) wiimote_report, gensym("report"), A_SYMBOL, A_FLOAT, 0);

		/* legacy report modes */
		class_addmethod(wiimote_class, (t_method) wiimote_setReportMode, gensym("setReportMode"), A_FLOAT, 0);
		class_addmethod(wiimote_class, (t_method) wiimote_reportAcceleration, gensym("reportAcceleration"), A_FLOAT, 0);
		class_addmethod(wiimote_class, (t_method) wiimote_reportIR, gensym("reportIR"), A_FLOAT, 0);

		class_addmethod(wiimote_class, (t_method) wiimote_reportNunchuk, gensym("reportNunchuck"), A_FLOAT, 0);
		class_addmethod(wiimote_class, (t_method) wiimote_reportNunchuk, gensym("reportNunchuk"), A_FLOAT, 0);
		class_addmethod(wiimote_class, (t_method) wiimote_reportMotionplus, gensym("reportMotionplus"), A_FLOAT, 0);

		/* play a sample */
		class_addmethod(wiimote_class, (t_method) wiimote_play, gensym("play"), A_SYMBOL, 0);

		post("[wiimote]: reading data from the Wii remote controller");
		post("	(c) 2011 IOhannes m zmölnig");
#ifdef VERSION
		post("	version " VERSION " published under the GNU General Public License");
#else
		post("	published under the GNU General Public License");
#endif
		post("contains WiiYourself! wiimote code by gl.tter http://gl.tter.org");
	}
}