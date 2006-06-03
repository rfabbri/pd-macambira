/* hidin.c - read in data from USB HID device */
/* Copyright 2003 Olaf Matthes, see README for a detailed licence */

/* 
   'hidin' is used to read in data from any HID (human interface device) connected 
   to the computers USB port(s).
   However, it does not work with mice or keyboards ('could not open' error)!

   Needs the Windows Driver Development Kit (DDK) to compile!

 */

#define     REQUEST_NOTHING 0
#define     REQUEST_READ 1
#define     REQUEST_QUIT 2

#include "m_pd.h"
#define SETSYM SETSYMBOL

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pthread.h"

#include "hidin.h"

static char *hidin_version = "hidin v1.0test2, Human Interface Device on USB, (c) 2003-2004 Olaf Matthes";

static t_class *hidin_class;

typedef struct _hidin
{
    t_object      x_obj;
	t_outlet      *x_outlet;            /* outlet for received data */
	t_outlet      *x_devout;            /* outlet for list of devices */
	long          x_interval;           /* interval in ms to read */
	long          *x_data;				/* data read from the interface */
	long          *x_prev_data;			/* data we read the time bofore */
	void	      *x_qelem;             /* qelem for outputing the results */
	long          x_device;             /* the HID device number we're connecting to */
	short         x_elements;           /* number of elements (i.e. buttons, axes...)
	
		/* HID specific */
	t_hid_device  *x_hid;               /* a struct containing all the needed stuff about the HID device */

		/* tread stuff */
    long              x_requestcode;	      /* pending request from parent to I/O thread */
    pthread_mutex_t   x_mutex;
    pthread_cond_t    x_requestcondition;
    pthread_cond_t    x_answercondition;
    pthread_t         x_thread;
} t_hidin;


/* Prototypes and short descriptions */

#ifndef PD
static void *hidin_output(t_hidin *x);
static void *hidin_doit(void *z);
static void hidin_tick(t_hidin *x);
static void hidin_clock_reset(t_hidin *x);
static void hidin_int(t_hidin *x, long l);
/* what/when to read */
static void hidin_interval(t_hidin *x, long l);	/* set time interval for reading ports */
/* direct response to user */
static void hidin_show(t_hidin *x);
static void hidin_start(t_hidin *x);
static void hidin_stop(t_hidin *x);
static void hidin_bang(t_hidin *x);
static void hidin_open(t_hidin *x, long l);
/* helper functions */
static void hidin_alloc(t_hidin *x, short elem);
/* standard Max/MSP related stuff */
static void *hidin_new(t_symbol *s, short argc, t_atom *argv);
static void hidin_free(t_hidin *x);
static void hidin_info(t_hidin *x, void *p, void *c);
static void hidin_assist(t_hidin *x, void *b, long m, long a, char *s);
void main(void);
#endif


/* -------------------  general support routines  --------------------- */

static void thread_post(t_hidin *x, char *p)
{
    post("hidin: %s", p);
}

/* ---------------------  data I/O support stuff  --------------------- */

/* output values using qelem */
static void *hidin_output(t_hidin *x)
{
	int i;
	static t_atom list[2];	/* output list format is: '(int)<port> (int)<value>' */
	long d;

	pthread_mutex_lock(&x->x_mutex);
	/* check every element for new value */
	for (i = 0; i < x->x_elements; i++)
	{
		d = x->x_data[i];
		if (d != x->x_prev_data[i])
		{
			SETFLOAT(list, (t_float)i+1);
			SETFLOAT(list+1, (t_float)d);
			outlet_list(x->x_outlet, NULL, 2, list);
			x->x_prev_data[i] = d;
		}
	}
	pthread_cond_signal(&x->x_requestcondition);	/* request previous state again */
	pthread_mutex_unlock(&x->x_mutex);
	return NULL;
}

static void hidin_parse_input(t_hidin *x)
{
	ULONG		i, j;
	PUSAGE		pUsage;
	t_hid_data	*inputData;

	unpackReport(x->x_hid->inputReportBuffer, x->x_hid->caps.InputReportByteLength, HidP_Input,
		x->x_hid->inputData, x->x_hid->inputDataLength, x->x_hid->ppd);

	inputData = x->x_hid->inputData;

    for (j = 0; j < x->x_hid->inputDataLength; j++)
    {
		if (inputData->IsButtonData)	// state of buttons changed
		{
			// post("Usage Page: 0x%x, Usages: ", inputData->UsagePage);

			// set all buttons to zero
			for (i = 0; i < inputData->ButtonData.MaxUsageLength; i++)
				x->x_data[i + x->x_hid->caps.NumberInputValueCaps] = 0;

			for (i = 0, pUsage = inputData->ButtonData.Usages;
						 i < inputData->ButtonData.MaxUsageLength;
							 i++, pUsage++) 
			{
				if (0 == *pUsage)
				{
					break; // A usage of zero is a non button.
				}
				else
				{
					x->x_data[*pUsage + x->x_hid->caps.NumberInputValueCaps - 1] = 1;
				}
			}   
		}
		else	// values changed
		{
			/* post("Usage Page: 0x%x, Usage: 0x%x, Scaled: %d Value: %d",
					  inputData->UsagePage,
					  inputData->ValueData.Usage,
					  inputData->ValueData.ScaledValue,
					  inputData->ValueData.Value); */
			// x->x_data[j - x->x_hid->caps.NumberInputButtonCaps] = inputData->ValueData.ScaledValue;
			x->x_data[j - x->x_hid->caps.NumberInputButtonCaps] = inputData->ValueData.Value;
		}
        inputData++;
    }
}

/*
 * this is the actual code that reads from the file handle to the HID device.
 * it's a second thread to avoid audio dropouts because it might take some time
 * for the read call to complete.
 */
static void *hidin_doit(void *z)
{
	t_hidin *x = (t_hidin *)z;
	int i, interval;
	long bytes;
	long ret;
	
	if (x->x_hid->event == 0)
	{
		x->x_hid->event = CreateEvent(NULL, TRUE, FALSE, "");
	}

	/* prepare overlapped structute */
	x->x_hid->overlapped.Offset     = 0; 
	x->x_hid->overlapped.OffsetHigh = 0; 
	x->x_hid->overlapped.hEvent     = x->x_hid->event; 
 
	pthread_mutex_lock(&x->x_mutex);
	while (1)
	{
		if (x->x_requestcode == REQUEST_NOTHING)
		{
			pthread_cond_signal(&x->x_answercondition);
			pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
		}
		else if (x->x_requestcode == REQUEST_READ)
		{
			interval = x->x_interval;
			if (x->x_device != -1)
			{
				pthread_mutex_unlock(&x->x_mutex);

				/* read in data from device */
				bytes = readHidOverlapped(x->x_hid);

				ret = WaitForSingleObject(x->x_hid->event, interval);
 
				pthread_mutex_lock(&x->x_mutex);
				if (ret == WAIT_OBJECT_0)	/* hey, we got signalled ! => data */
				{
					hidin_parse_input(x);	/* parse received data */
					clock_delay(x->x_qelem, 0);	/* we don't have qelems on PD ;-(  */
					/* wait for the data output to complete */
					pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
				}
				else	/* if no data, cancel read */
				{
					if (!CancelIo(x->x_hid->device) && (x->x_device != -1))
						thread_post(x, "-- error cancelling read");
				}
				if (!ResetEvent(x->x_hid->event))
					thread_post(x, "-- error resetting event");

				pthread_cond_signal(&x->x_answercondition);
			}
			else
			{
				/* if device is closed don't try to read and don't alter
				   the request code, stay in current state of operation */
				pthread_cond_signal(&x->x_answercondition);
				pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
			}
		}
		else if (x->x_requestcode == REQUEST_QUIT)
		{
			x->x_requestcode = REQUEST_NOTHING;
			pthread_cond_signal(&x->x_answercondition);
			break;
		}
		else	/* error if we get here! */
		{
			error("hidin: -- internal error, please report to <olaf.matthes@gmx.de>!");
		}
	}
	pthread_mutex_unlock(&x->x_mutex);
	return (0);
}

/*
 * set the update interval time in ms
 */
static void hidin_interval(t_hidin *x, t_floatarg l)
{
	int n = (int)l;

	pthread_mutex_lock(&x->x_mutex);
	if (n >= 2)
	{
		x->x_interval = n;
		post("hidin: >> interval: polling every %d msec", n);
	}
	else post("hidin: -- interval: wrong parameter value (minimum 2)");
	pthread_mutex_unlock(&x->x_mutex);
}

/*
 * print the actual config on the console window
 * LATER reduce to the absolute minimum, this is just for easier debugging!
 */
static void hidin_show(t_hidin *x)
{
	pthread_mutex_lock(&x->x_mutex);
	if (x->x_device == -1)
	{
		post("hidin: -- no open HID device");
	}
	else
	{
		post("hidin: ****** HID device %d ******", x->x_device);
		post("hidin: ** usage page: %X", x->x_hid->caps.UsagePage);
		post("hidin: ** input report byte length: %d", x->x_hid->caps.InputReportByteLength);
		post("hidin: ** output report byte length: %d", x->x_hid->caps.OutputReportByteLength);
		post("hidin: ** feature report byte length: %d", x->x_hid->caps.FeatureReportByteLength);
		post("hidin: ** number of link collection nodes: %d", x->x_hid->caps.NumberLinkCollectionNodes);
		post("hidin: ** number of input button caps: %d", x->x_hid->caps.NumberInputButtonCaps);
		post("hidin: ** number of inputValue caps: %d", x->x_hid->caps.NumberInputValueCaps);
		post("hidin: ** number of inputData indices: %d", x->x_hid->caps.NumberInputDataIndices);
		post("hidin: ** number of output button caps: %d", x->x_hid->caps.NumberOutputButtonCaps);
		post("hidin: ** number of output value caps: %d", x->x_hid->caps.NumberOutputValueCaps);
		post("hidin: ** number of output data indices: %d", x->x_hid->caps.NumberOutputDataIndices);
		post("hidin: ** number of feature button caps: %d", x->x_hid->caps.NumberFeatureButtonCaps);
		post("hidin: ** number of feature value caps: %d", x->x_hid->caps.NumberFeatureValueCaps);
		post("hidin: ** number of feature data indices: %d", x->x_hid->caps.NumberFeatureDataIndices);
	}
	pthread_mutex_unlock(&x->x_mutex);
}

/*
 * start / stop reading
 */
static void hidin_int(t_hidin *x, t_floatarg l)
{
	pthread_mutex_lock(&x->x_mutex);
	if (l)
	{
		x->x_requestcode = REQUEST_READ;
		pthread_cond_signal(&x->x_requestcondition);
	}
	else
	{
		x->x_requestcode = REQUEST_NOTHING;
		pthread_cond_signal(&x->x_requestcondition);
	}
	pthread_mutex_unlock(&x->x_mutex);
}

	/* start reading */
static void hidin_start(t_hidin *x)
{
	pthread_mutex_lock(&x->x_mutex);
	x->x_requestcode = REQUEST_READ;
	pthread_cond_signal(&x->x_requestcondition);
	pthread_mutex_unlock(&x->x_mutex);
}

	/* stop reading */
static void hidin_stop(t_hidin *x)
{
	pthread_mutex_lock(&x->x_mutex);
	x->x_requestcode = REQUEST_NOTHING;
	pthread_cond_signal(&x->x_requestcondition);
	pthread_mutex_unlock(&x->x_mutex);
}

	/* get list of devices */
static void hidin_bang(t_hidin *x)
{
	t_atom list[2];
	int numdev, i;

	pthread_mutex_lock(&x->x_mutex);
	// check for connected devices
	numdev = findHidDevices();
	post("hidin: ** found %d devices on your system", numdev);

	SETFLOAT(list, -1);
	SETSYM(list+1, gensym("None"));
	outlet_list(x->x_devout, NULL, 2, list);
	for (i = 0; i < numdev; i++)
	{
		SETFLOAT(list, i + 1);
		SETSYM(list+1, findDeviceName(i));
		outlet_list(x->x_devout, NULL, 2, list);
	}
	pthread_mutex_unlock(&x->x_mutex);
}

/*
 * allocate memory for output data (according to number of elements)
 */
static void hidin_alloc(t_hidin *x, short elem)
{
	int i;

	if (x->x_device != -1)
	{
		// free memory in case we already have some
		if (x->x_data && x->x_elements > 0)
		{
			freebytes(x->x_data, (short)(x->x_elements * sizeof(long)));
		}
		if (x->x_prev_data && x->x_elements > 0)
		{
			freebytes(x->x_prev_data, (short)(x->x_elements * sizeof(long)));
		}

		if (elem > 0)
		{
			// allocate memory to new size
			x->x_data = (long *)getbytes((short)(elem * sizeof(long)));
			x->x_prev_data = (long *)getbytes((short)(elem * sizeof(long)));
			if (!x->x_data || !x->x_prev_data)
			{
				post("hidin: -- out of memory");
				x->x_elements = 0;
				return;
			}
			// set newly allocated memory to zero
			for (i = 0; i < elem; i++)
			{
				x->x_data[i] = 0;
				x->x_prev_data[i] = 0;
			}
		}
		x->x_elements = elem;
	}
}

/*
 * init the device data structures
 */
static void hidin_initdevice(t_hidin *x)
{
	int i;
	short numElements;     
	short numValues;     
    short numCaps;
    PHIDP_BUTTON_CAPS   buttonCaps;
    PHIDP_VALUE_CAPS    valueCaps;
    USAGE               usage;
	t_hid_data			*data;

	// get device info and show some printout
	getDeviceInfo(x->x_hid->device);
	
	// read in device's capabilities and allocate memory accordingly
	getDeviceCapabilities(x->x_hid);

	// allocate memory for input field
	x->x_hid->inputReportBuffer = (char*)getbytes((short)(x->x_hid->caps.InputReportByteLength*sizeof(char)));

	// allocate memory for input info
	x->x_hid->inputButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)getbytes((short)(x->x_hid->caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS)));
	x->x_hid->inputValueCaps = valueCaps = (PHIDP_VALUE_CAPS)getbytes((short)(x->x_hid->caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS)));

    //
    // Have the HidP_X functions fill in the capability structure arrays.
    //

    numCaps = x->x_hid->caps.NumberInputButtonCaps;

    HidP_GetButtonCaps (HidP_Input,
                        buttonCaps,
                        &numCaps,
                        x->x_hid->ppd);

    numCaps = x->x_hid->caps.NumberInputValueCaps;

    HidP_GetValueCaps (HidP_Input,
                       valueCaps,
                       &numCaps,
                       x->x_hid->ppd);
	//
	// Depending on the device, some value caps structures may represent more
	// than one value.  (A range).  In the interest of being verbose, over
	// efficient, we will expand these so that we have one and only one
	// struct _HID_DATA for each value.
	//
	// To do this we need to count up the total number of values are listed
	// in the value caps structure.  For each element in the array we test
	// for range if it is a range then UsageMax and UsageMin describe the
	// usages for this range INCLUSIVE.
	//

	numValues = 0;
	for (i = 0; i < x->x_hid->caps.NumberInputValueCaps; i++, valueCaps++) 
	{
		if (valueCaps->IsRange) 
		{
			numValues += valueCaps->Range.UsageMax - valueCaps->Range.UsageMin + 1;
		}
		else
		{
			numValues++;
		}
	}
	valueCaps = x->x_hid->inputValueCaps;

	numElements = x->x_hid->caps.NumberInputValueCaps + 
					x->x_hid->caps.NumberInputButtonCaps * x->x_hid->caps.NumberInputDataIndices;

	post("hidin: >> device has %i input elements", numElements);
	hidin_alloc(x, (short)(numElements));	// allocate memory for all these elements

	x->x_hid->inputDataLength = numValues + x->x_hid->caps.NumberInputButtonCaps;

    x->x_hid->inputData = data = (t_hid_data *)getbytes((short)(x->x_hid->inputDataLength * sizeof(t_hid_data)));
	
    //
    // Fill in the button data
    //

    for (i = 0;
         i < x->x_hid->caps.NumberInputButtonCaps;
         i++, data++, buttonCaps++) 
    {
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;
        if (buttonCaps->IsRange) 
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else
        {
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }
        
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength(
                                                HidP_Input,
                                                buttonCaps->UsagePage,
                                                x->x_hid->ppd);
        data->ButtonData.Usages = (PUSAGE)
            calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
    }

    //
    // Fill in the value data
    //

    for (i = 0; i < numValues; i++, valueCaps++)
    {
        if (valueCaps->IsRange) 
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++) 
            {
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps->ReportID;
                data++;
            }
        } 
        else
        {
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps->ReportID;
            data++;
        }
    }

	// getFeature(x->x_hid);
}

static void hidin_doopen(t_hidin *x, long deviceID)
{
	short elements;	// number of elements of HID device
	long oldrequest;

	pthread_mutex_lock(&x->x_mutex);
	oldrequest = x->x_requestcode;
	x->x_requestcode = REQUEST_NOTHING;
	pthread_cond_signal(&x->x_requestcondition);
	pthread_cond_wait(&x->x_answercondition, &x->x_mutex);

	// close old device if any
	if (x->x_device != -1)
	{
		CloseHandle(x->x_hid->device);

		// free allocated memory 
		freebytes(x->x_hid->inputData, (short)(x->x_hid->inputDataLength * sizeof(t_hid_data)));
		freebytes(x->x_hid->inputButtonCaps, (short)(x->x_hid->caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS)));
		freebytes(x->x_hid->inputValueCaps, (short)(x->x_hid->caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS)));
		freebytes(x->x_hid->inputReportBuffer, (short)(x->x_hid->caps.InputReportByteLength * sizeof(char)));
		x->x_hid->caps.InputReportByteLength = 0;
	}

	if (deviceID != -1)
	{
		// open new device
		x->x_hid->device = connectDeviceNumber(deviceID);

		if (x->x_hid->device != INVALID_HANDLE_VALUE)
		{
			x->x_device = deviceID;
			hidin_initdevice(x);
		}
		else	// open failed...
		{
			hidin_alloc(x, 0);	// free data memory
			x->x_device = -1;
		}
	}
	else
	{
		hidin_alloc(x, 0);	// free data memory
		post("hidin: << device closed");
		x->x_device = -1;
	}

	x->x_requestcode = oldrequest;	/* set back to old requestcode */
	pthread_cond_signal(&x->x_requestcondition);	/* go on again */
	pthread_mutex_unlock(&x->x_mutex);
}

/*
 * select & open device to use
 */
static void hidin_open(t_hidin *x, t_floatarg l)
{
	long device = (long)l - 1;	/* get new internal device number */
	hidin_doopen(x, device);
}

/*
 * close open device
 */
static void hidin_close(t_hidin *x)
{
	if (x->x_device != -1)
		hidin_doopen(x, -1);
}

/*
 * the object's new function
 */
static void *hidin_new(t_symbol *s, short argc, t_atom *argv)
{
	int i, k, n;
	int deviceID, intv;

	t_hidin *x;

	deviceID = -1;	/* default HID device number: none */
	intv = 10;		/* read every 10 ms */
	
	if (argc >= 1 && argv->a_type == A_FLOAT)	/* just one argument (long): device ID */
	{
		deviceID = (int)(argv[0].a_w.w_float - 1);
		if (argc >= 2 && argv[1].a_type == A_FLOAT)	/* second argument (long): interval */
		{
			intv = (int)argv[1].a_w.w_float;
		}
	}

	x = (t_hidin*)pd_new(hidin_class);
		
	// zero out the struct, to be careful
	if (x)
	{
    	for (i = sizeof(t_object); i < sizeof(t_hidin); i++)
    		((char*)x)[i] = 0;
	}

	x->x_outlet = outlet_new(&x->x_obj, gensym("list"));	// outputs received data
	x->x_devout = outlet_new(&x->x_obj, gensym("list"));	// outputs list of devices

	x->x_qelem = clock_new(x, (t_method)hidin_output);

    pthread_mutex_init(&x->x_mutex, 0);
    pthread_cond_init(&x->x_requestcondition, 0);
    pthread_cond_init(&x->x_answercondition, 0);

	x->x_requestcode = REQUEST_NOTHING;

	x->x_interval = intv;
	x->x_device = -1;
	x->x_elements = 0;
	x->x_data = NULL;
	x->x_prev_data = NULL;

	// allocate memory for the t_hid_device struct 
	x->x_hid = (t_hid_device *)getbytes(sizeof(t_hid_device));

	// create I/O thread
	pthread_create(&x->x_thread, 0, hidin_doit, x);

	if (deviceID != -1)
		hidin_doopen(x, deviceID);

	return (x);
}

static void hidin_free(t_hidin *x)
{
    void *threadrtn;
	t_atom atom;

	// close the HID device
	hidin_doopen(x, -1);

	// stop IO thread
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = REQUEST_QUIT;
    pthread_cond_signal(&x->x_requestcondition);
	startpost("hidin: >> stopping HID I/O thread...");
	SETSYMBOL(&atom, gensym("signalling..."));
    while (x->x_requestcode != REQUEST_NOTHING)
    {
		postatom(1, &atom);
		pthread_cond_signal(&x->x_requestcondition);
    	pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    pthread_mutex_unlock(&x->x_mutex);
    if (pthread_join(x->x_thread, &threadrtn))
    	error("shoutcast_free: join failed");
	SETSYMBOL(&atom, gensym("done."));
	postatom(1, &atom);
	endpost();

	freebytes(x->x_hid, sizeof(t_hid_device));

	clock_free(x->x_qelem);

    pthread_cond_destroy(&x->x_requestcondition);
    pthread_cond_destroy(&x->x_answercondition);
    pthread_mutex_destroy(&x->x_mutex);
}

/*
 * the object's info function
 */
static void hidin_info(t_hidin *x)
{
	post(hidin_version);
	pthread_mutex_lock(&x->x_mutex);
	if (x->x_device != -1)
	{
		post("hidin: ** using device #%d: \"%s\"", x->x_device + 1, 
			 findDeviceName(x->x_device)->s_name);
		post("hidin: ** device has %d values and %d buttons", 
			 x->x_hid->caps.NumberInputValueCaps, 
			 x->x_hid->caps.NumberInputButtonCaps * x->x_hid->caps.NumberInputDataIndices);
		// getDeviceInfo(x->x_hid->device);
	}
	else post("hidin: -- no open device");
	pthread_mutex_unlock(&x->x_mutex);
}

void hidin_setup(void)
{
    hidin_class = class_new(gensym("hidin"),(t_newmethod)hidin_new, (t_method)hidin_free,
    	sizeof(t_hidin), 0, A_GIMME, 0);
	class_addbang(hidin_class, (t_method)hidin_bang);
	class_addfloat(hidin_class, (t_method)hidin_int);
	class_addmethod(hidin_class, (t_method)hidin_start, gensym("start"), 0);
	class_addmethod(hidin_class, (t_method)hidin_stop, gensym("stop"), 0);
	class_addmethod(hidin_class, (t_method)hidin_show, gensym("show"), 0);
	class_addmethod(hidin_class, (t_method)hidin_info, gensym("print"), 0);
	class_addmethod(hidin_class, (t_method)hidin_open, gensym("open"), A_FLOAT, 0);
	class_addmethod(hidin_class, (t_method)hidin_close, gensym("close"), 0);
	class_addmethod(hidin_class, (t_method)hidin_interval, gensym("interval"), A_FLOAT, 0);
	class_sethelpsymbol(hidin_class, gensym("help-hidin.pd"));
	post(hidin_version);
}

