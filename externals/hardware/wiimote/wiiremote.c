// ===================================================================
// Wiiremote external for Puredata
// Written by Mike Wozniewki (Feb 2007), www.mikewoz.com
//
// Requires the CWiid library (version 0.6.00) by L. Donnie Smith
//
// ===================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
// ===================================================================

//  ChangeLog:
//  2008-04-14 Florian Krebs 
//  * adapt wiiremote external for the actual version of cwiid (0.6.00)
//  2009-09-14 IOhannes m zmölnig
//    * made it compile without private cwiid-headers


#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <bluetooth/bluetooth.h>
#include <m_pd.h>
#include <math.h>
#include <cwiid.h>

#define PI	3.14159265358979323

struct acc {
	unsigned char x;
	unsigned char y;
	unsigned char z;
};

// class and struct declarations for wiiremote pd external:
static t_class *wiiremote_class;
typedef struct _wiiremote
{
	t_object x_obj; // standard pd object (must be first in struct)
	
	cwiid_wiimote_t *wiiremote; // individual wiiremote handle per pd object, represented in libcwiid

	t_float connected;
	int wiiremoteID;


	int reportMode;

	struct acc_cal acc_cal; /* calibration for built-in accelerometer */
	struct acc_cal nc_acc_cal;  /* calibration for nunchuk accelerometer */

	// outlets:
	t_outlet *outlet_data;	
} t_wiiremote;


// For now, we make one global t_wiiremote pointer that we can refer to
// in the cwiid_callback. This means we can support maximum of ONE
// wiiremote. ARGH. We'll have to figure out how to have access to the
// pd object from the callback (without modifying the CWiid code):
#define MAX_WIIREMOTES 14

typedef struct _wiiremoteList {
  t_wiiremote*x;
  int id;
  struct _wiiremoteList*next;
} t_wiiremoteList;

t_wiiremoteList*g_wiiremoteList=NULL;

static int addWiiremoteObject(t_wiiremote*x, int id) {
  t_wiiremoteList*wl=g_wiiremoteList;
  t_wiiremoteList*newentry=NULL;
  if(NULL!=wl) {
    while(wl->next) {
      
      if(wl->x == x) {
        pd_error(x, "[wiiremote]: already bound to Wii%02d", wl->id);
        return 0;
      }
      if(wl->id == id) {
        pd_error(x, "[wiiremote]: another object is already bound to Wii%02d", wl->id);
        return 0;
      }
      wl=wl->next;
    }
  }

  newentry=(t_wiiremoteList*)getbytes(sizeof(t_wiiremoteList));
  newentry->next=NULL;
  newentry->x=x;
  newentry->id=id;

  if(wl)
    wl->next=newentry;
  else 
    g_wiiremoteList=newentry;

  return 1;
}

static t_wiiremote*getWiiremoteObject(const int id) {
  t_wiiremoteList*wl=g_wiiremoteList;
  if(NULL==wl)
    return NULL;

  while(wl) {
    if(id == wl->id) {
      return wl->x;
    }
    wl=wl->next;
  }
  return NULL;
}

static void removeWiiremoteObject(const t_wiiremote*x) {
  t_wiiremoteList*wl=g_wiiremoteList;
  t_wiiremoteList*last=NULL;
  if(NULL==wl)
    return;

  while(wl) {
    if(x == wl->x) {
      if(last) {
        last->next=wl->next;
      } else {
        g_wiiremoteList=wl->next;
      }
      wl->x=NULL;
      wl->id=0;
      wl->next=NULL;
      freebytes(wl, sizeof(t_wiiremoteList));

      return;
    }
    last=wl;
    wl=wl->next;
  }
}



// ==============================================================
static void wiiremote_debug(t_wiiremote *x)
{
	post("\n======================");
	if (x->connected) post("Wiiremote (id: %d) is connected.", x->wiiremoteID);
	else post("Wiiremote (id: %d) is NOT connected.", x->wiiremoteID);
  post("acceleration: %s", (x->reportMode & CWIID_RPT_ACC)?"ON":"OFF");
  post("IR: %s", (x->reportMode & CWIID_RPT_IR)?"ON":"OFF");
  post("extensions: %s",  (x->reportMode & CWIID_RPT_EXT)?"ON":"OFF");
	post("");
	post("Accelerometer calibration: zero=(%d,%d,%d) one=(%d,%d,%d)",				 
			 x->acc_cal.zero[CWIID_X], x->acc_cal.zero[CWIID_Y], x->acc_cal.zero[CWIID_Z],
			 x->acc_cal.one [CWIID_X], x->acc_cal.one [CWIID_Y], x->acc_cal.one [CWIID_Z]);
	post("Nunchuk calibration:      zero=(%d,%d,%d) one=(%d,%d,%d)",
			 x->nc_acc_cal.zero[CWIID_X], x->nc_acc_cal.zero[CWIID_Y], x->nc_acc_cal.zero[CWIID_Z],
			 x->nc_acc_cal.one [CWIID_X], x->nc_acc_cal.one [CWIID_Y], x->nc_acc_cal.one [CWIID_Z]);
			 
	

}

// ==============================================================

static void wiiremote_cwiid_battery(t_wiiremote *x, int battery)
{
	t_atom ap[1];
	t_float bat=(1.f*battery) / CWIID_BATTERY_MAX;

	SETFLOAT(ap+0, bat);

	verbose(1, "Battery: %d%%", (int) (100*bat));

	outlet_anything(x->outlet_data, gensym("battery"), 1, ap);
}

// Button handler:
static void wiiremote_cwiid_btn(t_wiiremote *x, struct cwiid_btn_mesg *mesg)
{
	t_atom ap[2];
	SETFLOAT(ap+0, (mesg->buttons & 0xFF00)>>8);
	SETFLOAT(ap+1, mesg->buttons & 0x00FF);
	outlet_anything(x->outlet_data, gensym("button"), 2, ap);
}


static void wiiremote_cwiid_acc(t_wiiremote *x, struct cwiid_acc_mesg *mesg)
{
	double a_x, a_y, a_z;
	t_atom ap[3];
		
	a_x = ((double)mesg->acc[CWIID_X] - x->acc_cal.zero[CWIID_X]) / (x->acc_cal.one[CWIID_X] - x->acc_cal.zero[CWIID_X]);
	a_y = ((double)mesg->acc[CWIID_Y] - x->acc_cal.zero[CWIID_Y]) / (x->acc_cal.one[CWIID_Y] - x->acc_cal.zero[CWIID_Y]);
	a_z = ((double)mesg->acc[CWIID_Z] - x->acc_cal.zero[CWIID_Z]) / (x->acc_cal.one[CWIID_Z] - x->acc_cal.zero[CWIID_Z]);
		
	/*
		double a, roll, pitch;
		a = sqrt(pow(a_x,2)+pow(a_y,2)+pow(a_z,2));
		roll = atan(a_x/a_z);
		if (a_z <= 0.0) roll += PI * ((a_x > 0.0) ? 1 : -1);
		roll *= -1;
		pitch = atan(a_y/a_z*cos(roll));
	*/

		
	SETFLOAT(ap+0, a_x);
	SETFLOAT(ap+1, a_y);
	SETFLOAT(ap+2, a_z);
	outlet_anything(x->outlet_data, gensym("acceleration"), 3, ap);
	
}

static void wiiremote_cwiid_ir(t_wiiremote *x, struct cwiid_ir_mesg *mesg)
{
	unsigned int i;

	//post("IR (valid,x,y,size) #%d: %d %d %d %d", i, data->ir_data.ir_src[i].valid, data->ir_data.ir_src[i].x, data->ir_data.ir_src[i].y, data->ir_data.ir_src[i].size);
	for (i=0; i<CWIID_IR_SRC_COUNT; i++){		
		if (mesg->src[i].valid) {
			t_atom ap[4];
			SETFLOAT(ap+0, i);
			SETFLOAT(ap+1, mesg->src[i].pos[CWIID_X]);
			SETFLOAT(ap+2, mesg->src[i].pos[CWIID_Y]);
			SETFLOAT(ap+3, mesg->src[i].size);
			outlet_anything(x->outlet_data, gensym("ir"), 4, ap);
		}
	}
}

static void wiiremote_cwiid_nunchuk(t_wiiremote *x, struct cwiid_nunchuk_mesg *mesg)
{
	t_atom ap[4];
	double a_x, a_y, a_z;

	a_x = ((double)mesg->acc[CWIID_X] - x->nc_acc_cal.zero[CWIID_X]) / (x->nc_acc_cal.one[CWIID_X] - x->nc_acc_cal.zero[CWIID_X]);
	a_y = ((double)mesg->acc[CWIID_Y] - x->nc_acc_cal.zero[CWIID_Y]) / (x->nc_acc_cal.one[CWIID_Y] - x->nc_acc_cal.zero[CWIID_Y]);
	a_z = ((double)mesg->acc[CWIID_Z] - x->nc_acc_cal.zero[CWIID_Z]) / (x->nc_acc_cal.one[CWIID_Z] - x->nc_acc_cal.zero[CWIID_Z]);

	/*
	double a, roll, pitch;
	a = sqrt(pow(a_x,2)+pow(a_y,2)+pow(a_z,2));
	roll = atan(a_x/a_z);
	if (a_z <= 0.0) roll += PI * ((a_x > 0.0) ? 1 : -1);
	roll *= -1;
	pitch = atan(a_y/a_z*cos(roll));
	*/
	
	if (mesg->buttons & CWIID_NUNCHUK_BTN_C) {}
	if (mesg->buttons & CWIID_NUNCHUK_BTN_Z) {}
	/* nunchuk button */
	SETSYMBOL(ap+0, gensym("button"));
	SETFLOAT (ap+1, (t_float)mesg->buttons);
	outlet_anything(x->outlet_data, gensym("nunchuk"), 2, ap);
	

	/* nunchuk button */
	SETSYMBOL(ap+0, gensym("acceleration"));
	SETFLOAT (ap+1, a_x);
	SETFLOAT (ap+2, a_y);
	SETFLOAT (ap+3, a_z);
	outlet_anything(x->outlet_data, gensym("nunchuk"), 4, ap);
	
	/* nunchuk button */
	SETSYMBOL(ap+0, gensym("stick"));
	SETFLOAT (ap+1, mesg->stick[CWIID_X]);
	SETFLOAT (ap+2, mesg->stick[CWIID_Y]);
	outlet_anything(x->outlet_data, gensym("nunchuk"), 3, ap);
}

#ifdef CWIID_RPT_CLASSIC
static void wiiremote_cwiid_classic(t_wiiremote *x, struct cwiid_classic_mesg *mesg)
{
	t_atom ap[3];

	t_float scale = 1.f / ((uint16_t)0xFFFF);

	SETSYMBOL(ap+0, gensym("left_stick"));
	SETFLOAT (ap+1, mesg->l_stick[CWIID_X]);
	SETFLOAT (ap+2, mesg->l_stick[CWIID_Y]);
	outlet_anything(x->outlet_data, gensym("classic"), 3, ap);

	SETSYMBOL(ap+0, gensym("right_stick"));
	SETFLOAT (ap+1, mesg->r_stick[CWIID_X]);
	SETFLOAT (ap+2, mesg->r_stick[CWIID_Y]);
	outlet_anything(x->outlet_data, gensym("classic"), 3, ap);


	SETSYMBOL(ap+0, gensym("left"));
	SETFLOAT (ap+1, mesg->l);
	outlet_anything(x->outlet_data, gensym("classic"), 2, ap);

	SETSYMBOL(ap+0, gensym("right"));
	SETFLOAT (ap+1, mesg->r);
	outlet_anything(x->outlet_data, gensym("classic"), 2, ap);


	SETSYMBOL(ap+0, gensym("button"));
	SETFLOAT(ap+1, (mesg->buttons & 0xFF00)>>8);
	SETFLOAT(ap+2, mesg->buttons & 0x00FF);

	outlet_anything(x->outlet_data, gensym("classic"), 3, ap);


}
#endif

#ifdef CWIID_RPT_BALANCE
#warning Balance ignores calibration data
static void wiiremote_cwiid_balance_output(t_wiiremote *x, t_symbol*s, uint16_t value[3], t_float scale)
{
	t_atom ap[4];
	t_float a = scale*value[CWIID_X];
	t_float b = scale*value[CWIID_Y];
	t_float c = scale*value[CWIID_Z];

	SETSYMBOL(ap+0, s);
	SETFLOAT (ap+1, a);
	SETFLOAT (ap+2, b);
	SETFLOAT (ap+3, c);
	outlet_anything(x->outlet_data, gensym("balance"), 4, ap);
}

static void wiiremote_cwiid_balance(t_wiiremote *x, struct cwiid_balance_mesg *mesg)
{
	t_float scale = 1.f / ((uint16_t)0xFFFF);
	wiiremote_cwiid_balance_output(x, gensym("right_top"), &mesg->right_top, scale);
	wiiremote_cwiid_balance_output(x, gensym("right_bottom"), &mesg->right_bottom, scale);
	wiiremote_cwiid_balance_output(x, gensym("left_top"), &mesg->left_top, scale);
	wiiremote_cwiid_balance_output(x, gensym("left_bottom"), &mesg->left_bottom, scale);
}
#endif

#ifdef CWIID_RPT_MOTIONPLUS
static void wiiremote_cwiid_motionplus(t_wiiremote *x, struct cwiid_motionplus_mesg *mesg)
{
	t_atom ap[3];
	t_float scale = 1.f / ((uint16_t)0xFFFF);

	t_float phi  = scale*mesg->angle_rate[CWIID_PHI];
	t_float theta= scale*mesg->angle_rate[CWIID_THETA];
	t_float psi  = scale*mesg->angle_rate[CWIID_PSI];

	SETFLOAT(ap+0, phi);
	SETFLOAT(ap+1, theta);
	SETFLOAT(ap+2, psi);

	outlet_anything(x->outlet_data, gensym("motionplus"), 3, ap);
}
#endif



static void wiiremote_cwiid_message(t_wiiremote *x, union cwiid_mesg mesg) {
	unsigned char buf[7];
	switch (mesg.type) {
	case CWIID_MESG_STATUS:
		wiiremote_cwiid_battery(x, mesg.status_mesg.battery);
		switch (mesg.status_mesg.ext_type) {
		case CWIID_EXT_NONE:
			post("No extension attached");
			break;
		case CWIID_EXT_NUNCHUK:
			post("Nunchuk extension attached");
			if(cwiid_get_acc_cal(x->wiiremote, CWIID_EXT_NUNCHUK, &x->nc_acc_cal)) {
				post("Unable to retrieve nunchuk calibration");
			} else {
				post("Retrieved nunchuk calibration: zero=(%02d,%02d,%02d) one=(%02d,%02d,%02d)",
						 x->nc_acc_cal.zero[CWIID_X],
						 x->nc_acc_cal.zero[CWIID_Y],
						 x->nc_acc_cal.zero[CWIID_Z],
						 x->nc_acc_cal.one [CWIID_X],
						 x->nc_acc_cal.one [CWIID_Y],
						 x->nc_acc_cal.one [CWIID_Z]);
			}
			break;
		case CWIID_EXT_CLASSIC:
			post("Classic controller attached. There is no support for this yet.");
			break;
		case CWIID_EXT_BALANCE:
			post("Balance controller attached. There is no support for this yet.");
			break;
		case CWIID_EXT_MOTIONPLUS:
			post("MotionPlus controller attached.");
			/* no calibration needed for MotionPlus */
			break;
		case CWIID_EXT_UNKNOWN:
			post("Unknown extension attached");
			break;
		default:
			post("ext mesg %d unknown", mesg.status_mesg.ext_type);
			break;
		}
		break;
	case CWIID_MESG_BTN:
		wiiremote_cwiid_btn(x, &mesg.btn_mesg);
		break;
	case CWIID_MESG_ACC:
		wiiremote_cwiid_acc(x, &mesg.acc_mesg);
		break;
	case CWIID_MESG_IR:
		wiiremote_cwiid_ir(x, &mesg.ir_mesg);
		break;
#ifdef CWIID_RPT_NUNCHUK
	case CWIID_MESG_NUNCHUK:
		wiiremote_cwiid_nunchuk(x, &mesg.nunchuk_mesg);
		break;
#endif
#ifdef CWIID_RPT_CLASSIC
	case CWIID_MESG_CLASSIC:
		// todo
		break;
#endif
#ifdef CWIID_RPT_MOTIONPLUS
	case CWIID_MESG_MOTIONPLUS:
		wiiremote_cwiid_motionplus(x, &mesg.motionplus_mesg);
		break;
#endif
#ifdef CWIID_RPT_BALANCE
	case CWIID_MESG_BALANCE:
		wiiremote_cwiid_balance(x, &mesg.balance_mesg);
		break;
#endif
	default:
		post("mesg %d unknown", (mesg.type));
		break;
	}
}


// The CWiid library invokes a callback function whenever events are
// generated by the wiiremote. This function is specified when connecting
// to the wiiremote (in the cwiid_open function).

// Unfortunately, the mesg struct passed as an argument to the
// callback does not have a pointer to the wiiremote instance, and it
// is thus impossible to know which wiiremote-object has invoked the callback.
// For this case we provide a hard-coded set of wrapper callbacks to
// indicate which Pd wiiremote instance to control.

// So far I have only checked with one wiiremote

/*void cwiid_callback(cwiid_wiiremote_t *wiimt, int mesg_count, union cwiid_mesg *mesg[], struct timespec *timestamp)
*/
static void cwiid_callback(cwiid_wiimote_t *wiiremote, int mesg_count,
                    union cwiid_mesg mesg_array[], struct timespec *timestamp)
{
	int i;
  t_wiiremote *x=NULL;

  if(g_wiiremoteList==NULL||wiiremote==NULL) {
    post("no wii's known");
    return;
  }
  x=getWiiremoteObject(cwiid_get_id(wiiremote));
  if(NULL==x) {
		post("no wiiremote loaded: %d%",cwiid_get_id(wiiremote));
    return;
	}
			
  for (i=0; i < mesg_count; i++) {
		wiiremote_cwiid_message(x, mesg_array[i]);
	}
}

// ==============================================================

static void wiiremote_status(t_wiiremote *x)
{
	if(x->connected) {
		if (cwiid_request_status(x->wiiremote)) {
			pd_error(x, "error requesting status message");
		}
	}

}


static void wiiremote_resetReportMode(t_wiiremote *x)
{
	if (x->connected)	{
		verbose(1, "changing report mode for Wii%02d to %d", x->wiiremoteID, x->reportMode);
		if (cwiid_command(x->wiiremote, CWIID_CMD_RPT_MODE, x->reportMode)) {
			post("wiiremote error: problem setting report mode.");
		}
	}
}


static void wiiremote_setReportMode(t_wiiremote *x, t_floatarg r)
{
	if (r >= 0) {
		x->reportMode = (int) r;
		wiiremote_resetReportMode(x);
	}	else {
		return;
	}
}


static void wiiremote_report(t_wiiremote*x, int flag, int onoff)
{
	if(onoff) {
		x->reportMode |= flag;
	} else {
		x->reportMode &= ~flag;
	}
	wiiremote_resetReportMode(x);
}

static void wiiremote_reportAcceleration(t_wiiremote *x, t_floatarg f)
{
	wiiremote_report(x, CWIID_RPT_ACC, f);
}

static void wiiremote_reportIR(t_wiiremote *x, t_floatarg f)
{
	wiiremote_report(x, CWIID_RPT_IR, f);
}

static void wiiremote_reportNunchuk(t_wiiremote *x, t_floatarg f)
{
	wiiremote_report(x, CWIID_RPT_EXT, f);
}

static void wiiremote_reportMotionplus(t_wiiremote *x, t_floatarg f)
{
#ifdef CWIID_RPT_MOTIONPLUS
	int flag=f;
	if (x->connected)	{
		verbose(1, "changing motionplus report mode for Wii%02d to %d", x->wiiremoteID, flag);
		int err=0;
		if(flag) {
			err=cwiid_enable(x->wiiremote, CWIID_FLAG_MOTIONPLUS);
		} else {
			err=cwiid_disable(x->wiiremote, CWIID_FLAG_MOTIONPLUS);
		}
		if(err) {
			pd_error(x, "turning %s motionplus returned %d", (flag?"on":"off"), err);
		} else {
			wiiremote_report(x, CWIID_RPT_MOTIONPLUS, f);
		}
	}
#endif
}

static void wiiremote_setRumble(t_wiiremote *x, t_floatarg f)
{
	if (x->connected)
	{
		if (cwiid_command(x->wiiremote, CWIID_CMD_RUMBLE, f)) post("wiiremote error: problem setting rumble.");
	}
}

static void wiiremote_setLED(t_wiiremote *x, t_floatarg f)
{
	// some possible values:
	// CWIID_LED0_ON		0x01
	// CWIID_LED1_ON		0x02
	// CWIID_LED2_ON		0x04
	// CWIID_LED3_ON		0x08
	if (x->connected)
	{
		if (cwiid_command(x->wiiremote, CWIID_CMD_LED, f)) post("wiiremote error: problem setting LED.");
	}
}



// ==============================================================


// The following function attempts to connect to a wiiremote at a
// specific address, provided as an argument. eg, 00:19:1D:70:CE:72
// This address can be discovered by running the following command
// in a console:
//   hcitool scan | grep Nintendo

static void wiiremote_doConnect(t_wiiremote *x, t_symbol *addr, t_symbol *dongaddr)
{
	unsigned char buf[7];
	int i;
	bdaddr_t bdaddr;
	unsigned int flags =  CWIID_FLAG_MESG_IFC;

	bdaddr_t  dong_bdaddr;
	bdaddr_t* dong_bdaddr_ptr=&dong_bdaddr;

	// determine address:
	if (NULL==addr || addr==gensym("")) {
		post("Searching automatically...");		
		bdaddr = *BDADDR_ANY;
	}
	else {
		str2ba(addr->s_name, &bdaddr);
		post("Connecting to given address...");
		post("Press buttons 1 and 2 simultaneously.");
		} 

	// determine dongleaddress:
	if (NULL==dongaddr || dongaddr==gensym("")) {
		post("Binding automatically...");		
		dong_bdaddr_ptr = NULL;		
	}
	else {
		str2ba(dongaddr->s_name, &dong_bdaddr);
	} 	
	// connect:


#if 0
  x->wiiremote = cwiid_open(&bdaddr, dong_bdaddr_ptr, flags);
#else
#warning multi-dongle support...
  x->wiiremote = cwiid_open(&bdaddr, flags);
#endif

  if(NULL==x->wiiremote) {
		post("wiiremote error: unable to connect");
    return;
  }

  if(!addWiiremoteObject(x, cwiid_get_id(x->wiiremote))) {
    cwiid_close(x->wiiremote);
    x->wiiremote==NULL;
    return;
  }

  x->wiiremoteID= cwiid_get_id(x->wiiremote);
  
  post("wiiremote %i is successfully connected", x->wiiremoteID);

	if(cwiid_get_acc_cal(x->wiiremote, CWIID_EXT_NONE, &x->acc_cal)) {
    post("Unable to retrieve accelerometer calibration");
  } else {
    post("Retrieved wiiremote calibration: zero=(%02d,%02d,%02d) one=(%02d,%02d,%02d)",
				 x->acc_cal.zero[CWIID_X],
				 x->acc_cal.zero[CWIID_Y],
				 x->acc_cal.zero[CWIID_Z],
				 x->acc_cal.one [CWIID_X],
				 x->acc_cal.one [CWIID_Y],
				 x->acc_cal.one [CWIID_Z]);
  }

  x->connected = 1;

	x->reportMode |= CWIID_RPT_STATUS;
	x->reportMode |= CWIID_RPT_BTN;
  wiiremote_resetReportMode(x);

  if (cwiid_set_mesg_callback(x->wiiremote, &cwiid_callback)) {
    pd_error(x, "Unable to set message callback");
  }
}

// The following function attempts to discover a wiiremote. It requires
// that the user puts the wiiremote into 'discoverable' mode before being
// called. This is done by pressing the red button under the battery
// cover, or by pressing buttons 1 and 2 simultaneously.
// TODO: Without pressing the buttons, I get a segmentation error. So far, I don't know why.

static void wiiremote_discover(t_wiiremote *x)
{
	post("Put the wiiremote into discover mode by pressing buttons 1 and 2 simultaneously.");
		
	wiiremote_doConnect(x, NULL, gensym("NULL"));
	if (!(x->connected))
	{
		post("Error: could not find any wiiremotes. Please ensure that bluetooth is enabled, and that the 		'hcitool scan' command lists your Nintendo device.");
	}
}

static void wiiremote_doDisconnect(t_wiiremote *x)
{
		
	if (x->connected)
	{
		if (cwiid_close(x->wiiremote)) {
			post("wiiremote error: problems when disconnecting.");
		} 
		else {
			post("disconnect successfull, resetting values");
            removeWiiremoteObject(x);
			x->connected = 0;
		}
	}
	else post("device is not connected");
	
}


// ==============================================================
// ==============================================================

static void *wiiremote_new(t_symbol* s, int argc, t_atom *argv)
{
	bdaddr_t bdaddr; // wiiremote bdaddr
	t_wiiremote *x = (t_wiiremote *)pd_new(wiiremote_class);
	
	// create outlets:
	x->outlet_data = outlet_new(&x->x_obj, NULL);

	// initialize toggles:
	x->connected = 0;
	x->wiiremoteID = -1;
	
		// connect if user provided an address as an argument:
		
	if (argc==2)
	{
		post("conecting to provided address...");
		if (argv->a_type == A_SYMBOL)
		{
			wiiremote_doConnect(x, NULL, atom_getsymbol(argv));
		} else {
			error("[wiiremote] expects either no argument, or a bluetooth address as an argument. eg, 00:19:1D:70:CE:72");
			return NULL;
		}
	}
	return (x);
}


static void wiiremote_free(t_wiiremote* x)
{
	wiiremote_doDisconnect(x);
}

void wiiremote_setup(void)
{
	int i;
	
	wiiremote_class = class_new(gensym("wiiremote"), (t_newmethod)wiiremote_new, (t_method)wiiremote_free, sizeof(t_wiiremote), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_debug, gensym("debug"), 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_doConnect, gensym("connect"), A_SYMBOL, A_SYMBOL, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_doDisconnect, gensym("disconnect"), 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_discover, gensym("discover"), 0);


	class_addmethod(wiiremote_class, (t_method) wiiremote_status, gensym("status"), 0);

	class_addmethod(wiiremote_class, (t_method) wiiremote_setReportMode, gensym("setReportMode"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_reportAcceleration, gensym("reportAcceleration"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_reportNunchuk, gensym("reportNunchuck"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_reportNunchuk, gensym("reportNunchuk"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_reportMotionplus, gensym("reportMotionplus"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_reportIR, gensym("reportIR"), A_DEFFLOAT, 0);


	class_addmethod(wiiremote_class, (t_method) wiiremote_setRumble, gensym("setRumble"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class, (t_method) wiiremote_setLED, gensym("setLED"), A_DEFFLOAT, 0);
}


