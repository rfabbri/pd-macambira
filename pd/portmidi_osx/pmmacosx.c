/*
 * Platform interface to the MacOS X CoreMIDI framework
 * 
 * Jon Parise <jparise@cmu.edu>
 *
 * $Id: pmmacosx.c,v 1.3 2004-09-06 20:20:33 millerpuckette Exp $
 *
 * 27Jun02 XJS (X. J. Scott)
 *   - midi_length():
 *     fixed bug that gave bad lengths for system messages
 *
 *   / pm_macosx_init():
 *     Now allocates the device names. This fixes bug before where
 *     it assigned same string buffer on stack to all devices.
 *   - pm_macosx_term(), deleteDeviceName():
 *     devices strings allocated during pm_macosx_init() are deallocated.
 *
 *   + pm_macosx_init(), newDeviceName():
 *     registering kMIDIPropertyManufacturer + kMIDIPropertyModel + kMIDIPropertyName
 *     for name strings instead of just name.
 *
 *   / pm_macosx_init(): unsigned i to quiet compiler griping
 *   - get_timestamp():
 *     no change right here but type of Pt_Time() was altered in porttime.h
 *     so it matches type PmTimeProcPtr in assignment in this function.
 *   / midi_write():
 *     changed unsigned to signed to stop compiler griping
 */ 

#include "portmidi.h"
#include "pminternal.h"
#include "porttime.h"
#include "pmmacosx.h"

#include <stdio.h>
#include <string.h>

#include <CoreServices/CoreServices.h>
#include <CoreMIDI/MIDIServices.h>

#define PM_DEVICE_NAME_LENGTH 64

#define PACKET_BUFFER_SIZE 1024

static MIDIClientRef	client = NULL;	/* Client handle to the MIDI server */
static MIDIPortRef		portIn = NULL;	/* Input port handle */
static MIDIPortRef		portOut = NULL;	/* Output port handle */

extern pm_fns_node pm_macosx_in_dictionary;
extern pm_fns_node pm_macosx_out_dictionary;

static char * newDeviceName(MIDIEndpointRef endpoint);
static void deleteDeviceName(char **szDeviceName_p);

static int
midi_length(long msg)
{
    int status, high, low;
    static int high_lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1,         /* 0x00 through 0x70 */
        3, 3, 3, 3, 2, 2, 3, 1          /* 0x80 through 0xf0 */
    };
    static int low_lengths[] = {
        1, 1, 3, 2, 1, 1, 1, 1,         /* 0xf0 through 0xf8 */
        1, 1, 1, 1, 1, 1, 1, 1          /* 0xf9 through 0xff */
    };

    status = msg & 0xFF;
    high = status >> 4;
    low = status & 15;
//    return (high != 0xF0) ? high_lengths[high] : low_lengths[low];
   return (high != 0x0F) ? high_lengths[high] : low_lengths[low]; // fixed 6/27/03, xjs
}

static PmTimestamp
get_timestamp(PmInternal *midi)
{
	PmTimeProcPtr time_proc;

	/* Set the time procedure accordingly */
	time_proc = midi->time_proc;
    if (time_proc == NULL) {
		time_proc = Pt_Time;
	}

	return (*time_proc)(midi->time_info);
}

/* called when MIDI packets are received */
static void
readProc(const MIDIPacketList *newPackets, void *refCon, void *connRefCon)
{
	PmInternal *midi;
	PmEvent event;
	MIDIPacket *packet;
	unsigned int packetIndex;

	/* Retrieve the context for this connection */
	midi = (PmInternal *) connRefCon;

	packet = (MIDIPacket *) &newPackets->packet[0];
	for (packetIndex = 0; packetIndex < newPackets->numPackets; packetIndex++) {

		/* Build the PmMessage for the PmEvent structure */
		switch (packet->length) {
			case 1:
				event.message = Pm_Message(packet->data[0], 0, 0);
				break;
			case 2:
				event.message = Pm_Message(packet->data[0], packet->data[1], 0);
				break;
			case 3:
				event.message = Pm_Message(packet->data[0], packet->data[1],
										   packet->data[2]);
				break;
			default:
				/* Skip packets that are too large to fit in a PmMessage */
				continue;
		}

		/* Set the timestamp and dispatch this message */
		event.timestamp = get_timestamp(midi);
		pm_enqueue(midi, &event);

		/* Advance to the next packet in the packet list */
		packet = MIDIPacketNext(packet);
	}
}

static PmError
midi_in_open(PmInternal *midi, void *driverInfo)
{
	MIDIEndpointRef endpoint;

	endpoint = (MIDIEndpointRef) descriptors[midi->device_id].descriptor;
	if (endpoint == NULL) {
		return pmInvalidDeviceId;
	}

	if (MIDIPortConnectSource(portIn, endpoint, midi) != noErr) {
		return pmHostError;
	}

	return pmNoError;
}

static PmError
midi_in_close(PmInternal *midi)
{
	MIDIEndpointRef endpoint;

	endpoint = (MIDIEndpointRef) descriptors[midi->device_id].descriptor;
	if (endpoint == NULL) {
		return pmInvalidDeviceId;
	}

	if (MIDIPortDisconnectSource(portIn, endpoint) != noErr) {
		return pmHostError;
	}

	return pmNoError;
}

static PmError
midi_out_open(PmInternal *midi, void *driverInfo)
{
	/*
	 * MIDISent() only requires an output port (portOut) and a valid MIDI
	 * endpoint (which we've already created and stored in the PmInternal
	 * structure).  Therefore, no additional work needs to be done here to
	 * open the device for output.
	 */

	return pmNoError;
}

static PmError
midi_out_close(PmInternal *midi)
{
	return pmNoError;
}

static PmError
midi_abort(PmInternal *midi)
{
	return pmNoError;
}

static PmError
midi_write(PmInternal *midi, PmEvent *events, long length)
{
	Byte packetBuffer[PACKET_BUFFER_SIZE];
	MIDIEndpointRef endpoint;
	MIDIPacketList *packetList;
	MIDIPacket *packet;
	MIDITimeStamp timestamp;
	PmTimeProcPtr time_proc;
	PmEvent event;
	unsigned int pm_time;
  long eventIndex; // xjs: long instead of unsigned int, to match type of 'length' which compares against it
	unsigned int messageLength;
	Byte message[3];

	endpoint = (MIDIEndpointRef) descriptors[midi->device_id].descriptor;
	if (endpoint == NULL) {
		return pmInvalidDeviceId;
	}

	/* Make sure the packetBuffer is large enough */
	if (length > PACKET_BUFFER_SIZE) {
		return pmHostError;
	}

	/*
	 * Initialize the packet list. Each packet contains bytes that are to
	 * be played at the same time.
	 */
	packetList = (MIDIPacketList *) packetBuffer;
	if ((packet = MIDIPacketListInit(packetList)) == NULL) {
		return pmHostError;
	}

	/* Set the time procedure accordingly */
	time_proc = midi->time_proc;
    if (time_proc == NULL) {
		time_proc = Pt_Time;
	}

	/* Extract the event data and pack it into the message buffer */
	for (eventIndex = 0; eventIndex < length; eventIndex++) {
        	event = events[eventIndex];

		/* Compute the timestamp */
		pm_time = (*time_proc)(midi->time_info);
		timestamp = pm_time + midi->latency;

		messageLength = midi_length(event.message);
		message[0] = Pm_MessageStatus(event.message);
		message[1] = Pm_MessageData1(event.message);
		message[2] = Pm_MessageData2(event.message);

		/* Add this message to the packet list */
		packet = MIDIPacketListAdd(packetList, sizeof(packetBuffer), packet,
								   timestamp, messageLength, message);
		if (packet == NULL) {
			return pmHostError;
		}
	}

	if (MIDISend(portOut, endpoint, packetList) != noErr) {
		return pmHostError;
	}

	return pmNoError;
}

/* newDeviceName()    -- create a string that describes a MIDI endpoint device
 * deleteDeviceName() -- dispose of string created.
 *
 * Concatenates manufacturer, model and name of endpoint and returns
 * within freshly allocated space, to be registered in pm_add_device().
 *
 * 27Jun03: XJS -- extracted and extended from pm_macosx_init().
 * 11Nov03: XJS -- safely handles cases where any string properties are
 *   not present, such as is the case with the virtual ports created
 *   by many programs.
 */

static char * newDeviceName(MIDIEndpointRef endpoint)
{
  CFStringEncoding defaultEncoding;
  CFStringRef deviceCFString;
  char manufBuf[PM_DEVICE_NAME_LENGTH];
  char modelBuf[PM_DEVICE_NAME_LENGTH];
  char nameBuf[PM_DEVICE_NAME_LENGTH];
  char manufModelNameBuf[PM_DEVICE_NAME_LENGTH * 3 + 1];
  char *szDeviceName;
  size_t length;
  OSStatus iErr;

  /* Determine the default system character encording */

  defaultEncoding = CFStringGetSystemEncoding();

  /* Get the manufacturer, model and name of this device and combine into one string. */

  iErr = MIDIObjectGetStringProperty(endpoint, kMIDIPropertyManufacturer, &deviceCFString);
  if (noErr == iErr) {
      CFStringGetCString(deviceCFString, manufBuf, sizeof(manufBuf), defaultEncoding);
      CFRelease(deviceCFString);
      }
   else
     strcpy(manufBuf, "<undef. manuf>");

  iErr = MIDIObjectGetStringProperty(endpoint, kMIDIPropertyModel, &deviceCFString);
  if (noErr == iErr) {
      CFStringGetCString(deviceCFString, modelBuf, sizeof(modelBuf), defaultEncoding);
      CFRelease(deviceCFString);
      }
   else
     strcpy(modelBuf, "<undef. model>");

  iErr = MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &deviceCFString);
  if (noErr == iErr) {
      CFStringGetCString(deviceCFString, nameBuf, sizeof(nameBuf), defaultEncoding);
      CFRelease(deviceCFString);
      }
   else
     strcpy(nameBuf, "<undef. name>");

  sprintf(manufModelNameBuf, "%s %s: %s", manufBuf, modelBuf, nameBuf);
  length = strlen(manufModelNameBuf);

  /* Allocate a new string and return. */

  szDeviceName = (char *)pm_alloc(length + 1);
  strcpy(szDeviceName, manufModelNameBuf);

  return szDeviceName;
}

static void deleteDeviceName(char **szDeviceName_p)
{
  pm_free(*szDeviceName_p);
  *szDeviceName_p = NULL;
  return;
}

pm_fns_node pm_macosx_in_dictionary = {
    none_write,
    midi_in_open,
    midi_abort,
    midi_in_close
};

pm_fns_node pm_macosx_out_dictionary = {
    midi_write, 
    midi_out_open, 
    midi_abort, 
    midi_out_close
};

PmError
pm_macosx_init(void)
{
	OSStatus status;
	ItemCount numDevices, numInputs, numOutputs;
	MIDIEndpointRef endpoint;
	unsigned int i; // xjs, unsigned
  char *szDeviceName;

	/* Determine the number of MIDI devices on the system */
	numDevices = MIDIGetNumberOfDevices();
	numInputs  = MIDIGetNumberOfSources();
	numOutputs = MIDIGetNumberOfDestinations();

	/* Return prematurely if no devices exist on the system */
	if (numDevices <= 0) {
		return pmHostError;
	}


	/* Iterate over the MIDI input devices */
	for (i = 0; i < numInputs; i++) {
		endpoint = MIDIGetSource(i);
		if (endpoint == NULL) {
			continue;
		}

    /* Get the manufacturer, model and name of this device and combine into one string. */
    szDeviceName = newDeviceName(endpoint); // xjs

		/* Register this device with PortMidi */
    // xjs: szDeviceName is allocated memory since each has to be different and is not copied in pm_add_device()
    pm_add_device("CoreMIDI", szDeviceName, TRUE, (void *)endpoint,
					  &pm_macosx_in_dictionary);
	}

	/* Iterate over the MIDI output devices */
	for (i = 0; i < numOutputs; i++) {
		endpoint = MIDIGetDestination(i);
		if (endpoint == NULL) {
			continue;
		}

		/* Get the manufacturer & model of this device */
    szDeviceName = newDeviceName(endpoint); // xjs

		/* Register this device with PortMidi */
		pm_add_device("CoreMIDI", szDeviceName, FALSE, (void *)endpoint, // xjs, szDeviceName (as above)
            &pm_macosx_out_dictionary);

	}

	/* Initialize the client handle */
	status = MIDIClientCreate(CFSTR("PortMidi"), NULL, NULL, &client);
	if (status != noErr) {
		fprintf(stderr, "Could not initialize client: %d\n", (int)status);
		return pmHostError;
	}

	/* Create the input port */
	status = MIDIInputPortCreate(client, CFSTR("Input port"), readProc, NULL,
								 &portIn);
	if (status != noErr) {
		fprintf(stderr, "Could not create input port: %d\n", (int)status);
		return pmHostError;
	}

	/* Create the output port */
	status = MIDIOutputPortCreate(client, CFSTR("Output port"), &portOut);
	if (status != noErr) {
		fprintf(stderr, "Could not create output port: %d\n", (int)status);
		return pmHostError;
	}

	return pmNoError;
}

PmError
pm_macosx_term(void)
{
  int i;
  int device_count;
  const PmDeviceInfo *deviceInfo;

  /* release memory allocated for device names */
  device_count = Pm_CountDevices();
  for (i = 0; i < device_count; i++) {
    deviceInfo = Pm_GetDeviceInfo(i);
    deleteDeviceName((char **)&deviceInfo->name);
  }

	if (client != NULL)		MIDIClientDispose(client);
	if (portIn != NULL)		MIDIPortDispose(portIn);
	if (portOut != NULL)	MIDIPortDispose(portOut);

	return pmNoError;
}
