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

/* Wiiremote Callback */
cwiid_mesg_callback_t cwiid_callback;

// class and struct declarations for wiiremote pd external:
static t_class *cwiid_class;
typedef struct _wiiremote
{
	t_object x_obj; // standard pd object (must be first in struct)
	
	cwiid_wiimote_t *wiiremote; // individual wiiremote handle per pd object, represented in libcwiid

	t_float connected;
	int wiiremoteID;
	
	t_float toggle_acc, toggle_ir, toggle_nc;

	struct acc acc_zero, acc_one; // acceleration
	struct acc nc_acc_zero, nc_acc_one; // nunchuck acceleration

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

int addWiiremoteObject(t_wiiremote*x, int id) {
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

t_wiiremote*getWiiremoteObject(const int id) {
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

void removeWiiremoteObject(const t_wiiremote*x) {
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
void cwiid_debug(t_wiiremote *x)
{
	post("\n======================");
	if (x->connected) post("Wiiremote (id: %d) is connected.", x->wiiremoteID);
	else post("Wiiremote (id: %d) is NOT connected.", x->wiiremoteID);
  post("acceleration: %s", (x->toggle_acc)?"ON":"OFF");
  post("IR: %s", (x->toggle_ir)?"ON":"OFF");
  post("nunchuck: %s", (x->toggle_nc)?"ON":"OFF");
	post("");
	post("Accelerometer calibration: zero=(%d,%d,%d) one=(%d,%d,%d)",x->acc_zero.x,x->acc_zero.y,x->acc_zero.z,x->acc_one.x,x->acc_one.y,x->acc_one.z);
	post("Nunchuck calibration:      zero=(%d,%d,%d) one=(%d,%d,%d)",x->nc_acc_zero.x,x->nc_acc_zero.y,x->nc_acc_zero.z,x->nc_acc_one.x,x->nc_acc_one.y,x->nc_acc_one.z);
	

}

// ==============================================================

// Button handler:
void cwiid_btn(t_wiiremote *x, struct cwiid_btn_mesg *mesg)
{
	t_atom ap[2];
	SETFLOAT(ap+0, (mesg->buttons & 0xFF00)>>8);
	SETFLOAT(ap+1, mesg->buttons & 0x00FF);
	outlet_anything(x->outlet_data, gensym("button"), 2, ap);
}


void cwiid_acc(t_wiiremote *x, struct cwiid_acc_mesg *mesg)
{
	double a_x, a_y, a_z;
	t_atom ap[3];

	if(!x->toggle_acc)
		return;

		
	a_x = ((double)mesg->acc[CWIID_X] - x->acc_zero.x) / (x->acc_one.x - x->acc_zero.x);
	a_y = ((double)mesg->acc[CWIID_Y] - x->acc_zero.y) / (x->acc_one.y - x->acc_zero.y);
	a_z = ((double)mesg->acc[CWIID_Z] - x->acc_zero.z) / (x->acc_one.z - x->acc_zero.z);
		
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

void cwiid_ir(t_wiiremote *x, struct cwiid_ir_mesg *mesg)
{
	unsigned int i;
	if(!x->toggle_ir)
		return;

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

void cwiid_nunchuk(t_wiiremote *x, struct cwiid_nunchuk_mesg *mesg)
{
	t_atom ap[4];
	double a_x, a_y, a_z;

	a_x = ((double)mesg->acc[CWIID_X] - x->nc_acc_zero.x) / (x->nc_acc_one.x - x->nc_acc_zero.x);
	a_y = ((double)mesg->acc[CWIID_Y] - x->nc_acc_zero.y) / (x->nc_acc_one.y - x->nc_acc_zero.y);
	a_z = ((double)mesg->acc[CWIID_Z] - x->nc_acc_zero.z) / (x->nc_acc_one.z - x->nc_acc_zero.z);

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
	/* nunchuck button */
	SETSYMBOL(ap+0, gensym("button"));
	SETFLOAT (ap+1, (t_float)mesg->buttons);
	outlet_anything(x->outlet_data, gensym("nunchuck"), 2, ap);
	

	/* nunchuck button */
	SETSYMBOL(ap+0, gensym("acceleration"));
	SETFLOAT (ap+1, a_x);
	SETFLOAT (ap+2, a_y);
	SETFLOAT (ap+3, a_z);
	outlet_anything(x->outlet_data, gensym("nunchuck"), 4, ap);
	
	/* nunchuck button */
	SETSYMBOL(ap+0, gensym("stick"));
	SETFLOAT (ap+1, mesg->stick[CWIID_X]);
	SETFLOAT (ap+2, mesg->stick[CWIID_Y]);
	outlet_anything(x->outlet_data, gensym("nunchuck"), 3, ap);
}

// The CWiid library invokes a callback function whenever events are
// generated by the wiiremote. This function is specified when connecting
// to the wiiremote (in the cwiid_open function).

// Unfortunately, the mesg struct passed as an argument to the
// callback does not have a pointer to the wiiremote instance, and it
// is thus impossible to know which wiiremote has invoked the callback.
// For this case we provide a hard-coded set of wrapper callbacks to
// indicate which Pd wiiremote instance to control.

// So far I have only checked with one wiiremote

/*void cwiid_callback(cwiid_wiiremote_t *wiimt, int mesg_count, union cwiid_mesg *mesg[], struct timespec *timestamp)
*/
void cwiid_callback(cwiid_wiimote_t *wiiremote, int mesg_count,
                    union cwiid_mesg mesg_array[], struct timespec *timestamp)
{
	unsigned char buf[7];
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
			
  for (i=0; i < mesg_count; i++)
		{	
			switch (mesg_array[i].type) {
      case CWIID_MESG_STATUS:
        post("Battery: %d%", (int) (100.0 * mesg_array[i].status_mesg.battery / CWIID_BATTERY_MAX));
        switch (mesg_array[i].status_mesg.ext_type) {
        case CWIID_EXT_NONE:
          post("No nunchuck attached");
          break;
        case CWIID_EXT_NUNCHUK:
          post("Nunchuck extension attached");
						
          if (cwiid_read(x->wiiremote, CWIID_RW_REG | CWIID_RW_DECODE, 0xA40020, 								7, buf)) {
            post("Unable to retrieve Nunchuk calibration");
          }
          else {
            x->nc_acc_zero.x = buf[0];
            x->nc_acc_zero.y = buf[1];
            x->nc_acc_zero.z = buf[2];
            x->nc_acc_one.x  = buf[4];
            x->nc_acc_one.y  = buf[5];
            x->nc_acc_one.z  = buf[6];
          }	
          break;
        case CWIID_EXT_CLASSIC:
          post("Classic controller attached. There is no support for this yet.");
          break;
        case CWIID_EXT_UNKNOWN:
          post("Unknown extension attached");
          break;
        }
        break;
      case CWIID_MESG_BTN:
        cwiid_btn(x, &mesg_array[i].btn_mesg);
        break;
      case CWIID_MESG_ACC:
        cwiid_acc(x, &mesg_array[i].acc_mesg);
        break;
      case CWIID_MESG_IR:
        cwiid_ir(x, &mesg_array[i].ir_mesg);
        break;
      case CWIID_MESG_NUNCHUK:
        cwiid_nunchuk(x, &mesg_array[i].nunchuk_mesg);
        break;
      case CWIID_MESG_CLASSIC:
        // todo
        break;
      default:
        break;
      }
    }
}

// ==============================================================



void cwiid_setReportMode(t_wiiremote *x, t_floatarg r)
{
	unsigned char rpt_mode;

	if (r >= 0) rpt_mode = (unsigned char) r;
	else {
		rpt_mode = CWIID_RPT_STATUS | CWIID_RPT_BTN;
		if (x->toggle_ir) rpt_mode |= CWIID_RPT_IR;
		if (x->toggle_acc) rpt_mode |= CWIID_RPT_ACC;
		if (x->toggle_nc) rpt_mode |= CWIID_RPT_EXT;
	}
	if (x->connected)
	{
    verbose(1, "changing report mode for Wii%02d to %d", x->wiiremoteID, rpt_mode);
		if (cwiid_command(x->wiiremote, CWIID_CMD_RPT_MODE, rpt_mode)) {
			post("wiiremote error: problem setting report mode.");
		}
	}
}

void cwiid_reportAcceleration(t_wiiremote *x, t_floatarg f)
{
	x->toggle_acc = f;
	cwiid_setReportMode(x, -1);
}

void cwiid_reportIR(t_wiiremote *x, t_floatarg f)
{
	x->toggle_ir = f;
	cwiid_setReportMode(x, -1);
}

void cwiid_reportNunchuck(t_wiiremote *x, t_floatarg f)
{
	x->toggle_nc = f;
	cwiid_setReportMode(x, -1);
}
void cwiid_setRumble(t_wiiremote *x, t_floatarg f)
{
	if (x->connected)
	{
		if (cwiid_command(x->wiiremote, CWIID_CMD_RUMBLE, f)) post("wiiremote error: problem setting rumble.");
	}
}

void cwiid_setLED(t_wiiremote *x, t_floatarg f)
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

void cwiid_doConnect(t_wiiremote *x, t_symbol *addr, t_symbol *dongaddr)
{
	unsigned char buf[7];
	int i;
	bdaddr_t bdaddr;

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
  x->wiiremote = cwiid_open(&bdaddr, dong_bdaddr_ptr, CWIID_FLAG_MESG_IFC);
#else
#warning multi-dongle support...
  x->wiiremote = cwiid_open(&bdaddr, CWIID_FLAG_MESG_IFC);
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
  if (cwiid_read(x->wiiremote, CWIID_RW_EEPROM, 0x16, 7, buf)) {
    post("Unable to retrieve accelerometer calibration");
  } else {
    x->acc_zero.x = buf[0];
    x->acc_zero.y = buf[1];
    x->acc_zero.z = buf[2];
    x->acc_one.x  = buf[4];
    x->acc_one.y  = buf[5];
    x->acc_one.z  = buf[6];
    //post("Retrieved wiiremote calibration: zero=(%.1f,%.1f,%.1f) one=(%.1f,%.1f,%.1f)",buf[0],buf[2],buf[3],buf[4],buf[5],buf[6]);
  }

  x->connected = 1;
  cwiid_setReportMode(x,-1);

  if (cwiid_set_mesg_callback(x->wiiremote, &cwiid_callback)) {
    pd_error(x, "Unable to set message callback");
  }
}

// The following function attempts to discover a wiiremote. It requires
// that the user puts the wiiremote into 'discoverable' mode before being
// called. This is done by pressing the red button under the battery
// cover, or by pressing buttons 1 and 2 simultaneously.
// TODO: Without pressing the buttons, I get a segmentation error. So far, I don't know why.

void cwiid_discover(t_wiiremote *x)
{
	post("Put the wiiremote into discover mode by pressing buttons 1 and 2 simultaneously.");
		
	cwiid_doConnect(x, NULL, gensym("NULL"));
	if (!(x->connected))
	{
		post("Error: could not find any wiiremotes. Please ensure that bluetooth is enabled, and that the 		'hcitool scan' command lists your Nintendo device.");
	}
}

void cwiid_doDisconnect(t_wiiremote *x)
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

static void *cwiid_new(t_symbol* s, int argc, t_atom *argv)
{
	bdaddr_t bdaddr; // wiiremote bdaddr
	t_wiiremote *x = (t_wiiremote *)pd_new(cwiid_class);
	
	// create outlets:
	x->outlet_data = outlet_new(&x->x_obj, NULL);

	// initialize toggles:
	x->toggle_acc = 0;
	x->toggle_ir = 0;
	x->toggle_nc = 0;

	x->connected = 0;
	x->wiiremoteID = -1;
	
		// connect if user provided an address as an argument:
		
	if (argc==2)
	{
		post("conecting to provided address...");
		if (argv->a_type == A_SYMBOL)
		{
			cwiid_doConnect(x, NULL, atom_getsymbol(argv));
		} else {
			error("[wiiremote] expects either no argument, or a bluetooth address as an argument. eg, 00:19:1D:70:CE:72");
			return NULL;
		}
	}
	return (x);
}


static void cwiid_free(t_wiiremote* x)
{
	cwiid_doDisconnect(x);
}

void wiiremote_setup(void)
{
	int i;
	
	cwiid_class = class_new(gensym("wiiremote"), (t_newmethod)cwiid_new, (t_method)cwiid_free, sizeof(t_wiiremote), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_debug, gensym("debug"), 0);
	class_addmethod(cwiid_class, (t_method) cwiid_doConnect, gensym("connect"), A_SYMBOL, A_SYMBOL, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_doDisconnect, gensym("disconnect"), 0);
	class_addmethod(cwiid_class, (t_method) cwiid_discover, gensym("discover"), 0);
	class_addmethod(cwiid_class, (t_method) cwiid_setReportMode, gensym("setReportMode"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_reportAcceleration, gensym("reportAcceleration"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_reportNunchuck, gensym("reportNunchuck"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_reportIR, gensym("reportIR"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_setRumble, gensym("setRumble"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_setLED, gensym("setLED"), A_DEFFLOAT, 0);
}


