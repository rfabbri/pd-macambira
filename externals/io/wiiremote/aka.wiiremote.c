// aka.wiiremote.c
// Copyright by Masayuki Akamatsu
// 1.0B1 : 2006.12.12
// 1.0B2 : 2006.12.15
// 1.0B3 : 2006.12.20

#ifdef PD
#include "m_pd.h"
#define SETLONG SETFLOAT
static t_class *wiiremote_class;
#else /* Max */
#include "ext.h"
#endif /* PD */

#include "wiiremote.h"
#include <stdio.h>

#define kInterval	100
#define	kMaxTrial	100

typedef struct _akawiiremote
{
#ifdef PD
	t_object        x_obj;
#else /* Max */
	struct object	obj;
#endif
	
	WiiRemoteRef	wiiremote;
	
	void			*clock;
	long			interval;
	long			trial;
	
	void			*statusOut;
	void			*buttonsOut;
	void			*irOut;
	void			*accOut;
} t_akawiiremote;

void *akawiiremote_class;	// the number of instance of this object

void akawiiremote_bang(t_akawiiremote *x);
void akawiiremote_connect(t_akawiiremote *x);
void akawiiremote_disconnect(t_akawiiremote *x);
void akawiiremote_motionsensor(t_akawiiremote *x, long enable);
void akawiiremote_irsensor(t_akawiiremote *x, long enable);
void akawiiremote_vibration(t_akawiiremote *x, long enable);
void akawiiremote_led(t_akawiiremote *x, long enable1, long enable2, long enable3, long enable4);

void akawiiremote_getbatterylevel(t_akawiiremote *x);
void akawiiremote_getexpansionstatus(t_akawiiremote *x);
void akawiiremote_getledstatus(t_akawiiremote *x);

void akawiiremote_assist(t_akawiiremote *x, void *b, long m, long a, char *s);
void akawiiremote_clock(t_akawiiremote *x);
void *akawiiremote_new(t_symbol *s, short ac, t_atom *av);
void akawiiremote_free(t_akawiiremote *x);

#ifdef PD
void wiiremote_setup()
#else /* Max */
void main()
#endif /* PD */
{
	NumVersion				outSoftwareVersion;
	BluetoothHCIVersionInfo	outHardwareVersion;
	
	if (IOBluetoothGetVersion(&outSoftwareVersion, &outHardwareVersion)==kIOReturnSuccess)
	{
		if (outSoftwareVersion.majorRev < 1 || outSoftwareVersion.minorAndBugRev < 0x63)
		{
			error("requires Blutooth version 1.6.3 or later.");
			return;
		}
	}
	else
	{
		error("can't get Bluetooth version.");
		return;
	}

	post("aka.wiiremote 1.0B4-UB by Masayuki Akamatsu");

#ifdef PD
	post("\tPd port by Hans-Christoph Steiner");

	wiiremote_class = class_new(gensym("wiiremote"), 
								 (t_newmethod)akawiiremote_new, 
								 (t_method)akawiiremote_free,
								 sizeof(t_akawiiremote),
								 CLASS_DEFAULT,
								 A_GIMME,0);

	class_addbang(wiiremote_class,(t_method)akawiiremote_bang);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_connect,gensym("connect"),0);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_disconnect,gensym("disconnect"),0);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_motionsensor,gensym("motionsensor"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_irsensor,gensym("irsensor"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_vibration,gensym("vibration"), A_DEFFLOAT, 0);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_led,gensym("led"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

	class_addmethod(wiiremote_class,(t_method)akawiiremote_getbatterylevel,gensym("getbatterylevel"),0);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_getexpansionstatus,gensym("getexpansionstatus"),0);
	class_addmethod(wiiremote_class,(t_method)akawiiremote_getledstatus,gensym("getledstatus"),0);
	
	class_addmethod(wiiremote_class,(t_method)akawiiremote_assist,gensym("assist"),A_CANT,0);
#else /* Max */
	setup((t_messlist **)&akawiiremote_class, (method)akawiiremote_new, (method)akawiiremote_free, (short)sizeof(t_akawiiremote), 0L, A_GIMME, 0);

	addbang((method)akawiiremote_bang);
	addmess((method)akawiiremote_connect,"connect",0);
	addmess((method)akawiiremote_disconnect,"disconnect",0);
	addmess((method)akawiiremote_motionsensor,"motionsensor", A_DEFLONG, 0);
	addmess((method)akawiiremote_irsensor,"irsensor", A_DEFLONG, 0);
	addmess((method)akawiiremote_vibration,"vibration", A_DEFLONG, 0);
	addmess((method)akawiiremote_led,"led", A_DEFLONG, A_DEFLONG, A_DEFLONG, A_DEFLONG, 0);

	addmess((method)akawiiremote_getbatterylevel,"getbatterylevel",0);
	addmess((method)akawiiremote_getexpansionstatus,"getexpansionstatus",0);
	addmess((method)akawiiremote_getledstatus,"getledstatus",0);
	
	addmess((method)akawiiremote_assist,"assist",A_CANT,0);
#endif /* PD */
}

//--------------------------------------------------------------------------------------------

void akawiiremote_bang(t_akawiiremote *x)
{
	t_atom list[4]; 
	
	if (x->wiiremote->device == nil)
		return;	// do nothing
	
#ifdef PD
	outlet_float(x->buttonsOut, (t_float) x->wiiremote->buttonData);
#else /* Max */	
	outlet_int(x->buttonsOut, x->wiiremote->buttonData);
#endif /* PD */

	if (x->wiiremote->isIRSensorEnabled)
	{
		SETFLOAT(list,     x->wiiremote->posX);
		SETFLOAT(list + 1, x->wiiremote->posY);
		SETFLOAT(list + 2, x->wiiremote->angle);
		SETLONG (list + 3, x->wiiremote->tracking);
		outlet_list(x->irOut, 0L, 4, list); 
	}

	if (x->wiiremote->isMotionSensorEnabled)
	{
		SETLONG(list,     x->wiiremote->accX);
		SETLONG(list + 1, x->wiiremote->accY);
		SETLONG(list + 2, x->wiiremote->accZ);
		SETLONG(list + 3, x->wiiremote->orientation);
		outlet_list(x->accOut, 0L, 4, list); 
	}
	
	//wiiremote_getstatus();	// stopped in B3
}

void akawiiremote_connect(t_akawiiremote *x)
{
	t_atom	status;
	Boolean	result;

	if (wiiremote_isconnected(x->wiiremote))
	{
		SETLONG(&status, -1);
		outlet_anything(x->statusOut, gensym("connect"), 1, &status);		
	}
	else
	{
		result = wiiremote_search(x->wiiremote);	// start searching the device
		x->trial = 0;
		clock_unset(x->clock);			// stop clock
		clock_delay(x->clock, 0);		// start clock to check the device found
	}
}

void akawiiremote_foundFunc(t_akawiiremote *x)
{
}

void akawiiremote_disconnect(t_akawiiremote *x)
{
	Boolean	result;
	t_atom	status;
	
	result = wiiremote_disconnect(x->wiiremote);
	SETLONG(&status, result);
	outlet_anything(x->statusOut, gensym("disconnect"), 1, &status);		
}

void akawiiremote_motionsensor(t_akawiiremote *x, long enable)
{
	wiiremote_motionsensor(x->wiiremote, enable);
}

void akawiiremote_irsensor(t_akawiiremote *x, long enable)
{
	wiiremote_irsensor(x->wiiremote, enable);
}

void akawiiremote_vibration(t_akawiiremote *x, long enable)
{
	wiiremote_vibration(x->wiiremote, enable);
}

void akawiiremote_led(t_akawiiremote *x, long enable1, long enable2, long enable3, long enable4)
{
	wiiremote_led(x->wiiremote, enable1, enable2, enable3, enable4);
}

//--------------------------------------------------------------------------------------------

void akawiiremote_getbatterylevel(t_akawiiremote *x)
{
	t_atom	status;

	SETFLOAT(&status, x->wiiremote->batteryLevel);
	outlet_anything(x->statusOut, gensym("batterylevel"), 1, &status);		
}

void akawiiremote_getexpansionstatus(t_akawiiremote *x)
{
	t_atom	status;
	
	SETLONG(&status, x->wiiremote->isExpansionPortUsed);
	outlet_anything(x->statusOut, gensym("expansionstatus"), 1, &status);		
}

void akawiiremote_getledstatus(t_akawiiremote *x)
{
	t_atom list[4]; 
	
	SETLONG(list,     x->wiiremote->isLED1Illuminated);
	SETLONG(list + 1, x->wiiremote->isLED2Illuminated);
	SETLONG(list + 2, x->wiiremote->isLED3Illuminated);
	SETLONG(list + 3, x->wiiremote->isLED4Illuminated);
	outlet_anything(x->statusOut, gensym("ledstatus"), 4, list);		
}

//--------------------------------------------------------------------------------------------

void akawiiremote_clock(t_akawiiremote *x)
{
	//Boolean	result;
	t_atom	status;
	
	if (wiiremote_isconnected(x->wiiremote))	// if the device is connected...
	{
		clock_unset(x->clock);			// stop clock

		wiiremote_stopsearch(x->wiiremote);
		//result = wiiremote_connect();	// remove in B3
		wiiremote_getstatus(x->wiiremote);		// add in B3
		SETLONG(&status, 1);
		outlet_anything(x->statusOut, gensym("connect"), 1, &status);
	}
	else	// if the device is not connected...
	{
		x->trial++;
		//SETLONG(&status, x->trial);
		//outlet_anything(x->statusOut, gensym("searching"), 1, &status);

		if (x->trial >= kMaxTrial)		// if trial is over
		{
			clock_unset(x->clock);		// stop clock

			wiiremote_stopsearch(x->wiiremote);
			SETLONG(&status, 0);
			outlet_anything(x->statusOut, gensym("connect"), 1, &status);
		}
		else
		{
			//post("trial %d",x->trial);
			clock_delay(x->clock, x->interval);	// restart clock
		}
	}
}

//--------------------------------------------------------------------------------------------

void akawiiremote_assist(t_akawiiremote *x, void *b, long m, long a, char *s)
{
#ifndef PD /* Max */
	if (m==ASSIST_INLET)
	{
		sprintf(s,"connect, bang, disconnect....");
	}
	else  
#endif /* NOT PD */
	{
		switch(a)
		{
			case 0: sprintf(s,"list(acc-x acc-y acc-z orientation)"); break;
			case 1: sprintf(s,"list(pos-x pos-y angle tracking)"); break;
			case 2: sprintf(s,"int(buttons)"); break;
			case 3: sprintf(s,"message(status)"); break;
		}
	}
}

//--------------------------------------------------------------------------------------------

void *akawiiremote_new(t_symbol *s, short ac, t_atom *av)
{
#ifdef PD
	t_akawiiremote *x = (t_akawiiremote *)pd_new(wiiremote_class);

	if (x->wiiremote != nil)
		wiiremote_init(x->wiiremote);
	
	x->clock = clock_new(x, (t_method)akawiiremote_clock);

	/* create anything outlet used for HID data */ 
	x->statusOut = outlet_new(&x->x_obj, 0);
	x->buttonsOut = outlet_new(&x->x_obj, &s_float);
	x->irOut = outlet_new(&x->x_obj, &s_list);
	x->accOut = outlet_new(&x->x_obj, &s_list);
#else /* Max */	
	t_akawiiremote *x;
	
	x = (t_akawiiremote *)newobject(akawiiremote_class);
	
	x->wiiremote = (WiiRemoteRef)getbytes(sizeof(WiiRemoteRec));		// add in 1.0B4
	if (x->wiiremote != nil)
		wiiremote_init(x->wiiremote);
	
	x->clock = clock_new(x, (method)akawiiremote_clock);
	
	x->statusOut = outlet_new(x, 0);
	x->buttonsOut = intout(x);
	x->irOut = listout(x);
	x->accOut = listout(x);
#endif /* PD */
	x->trial = 0;
	x->interval	= kInterval;

	return x;
}

void akawiiremote_free(t_akawiiremote *x)
{
	if (x->wiiremote != nil)							// add in 1.0B4
	{
		wiiremote_disconnect(x->wiiremote);
		freebytes(x->wiiremote, sizeof(WiiRemoteRec));	// add in 1.0B4
	}
	
	clock_unset(x->clock);
#ifdef PD
	clock_free(x->clock);
#else /* Max */	
	freeobject((t_object *)x->clock); 
#endif /* PD */
}

