/*
 * Platform interface to the MacOS X CoreMIDI framework
 * 
 * Jon Parise <jparise@cmu.edu>
 *
 * $Id: pmmacosx.c,v 1.1.1.1 2002-07-29 17:06:16 ggeiger Exp $
 */ 

#include "portmidi.h"
#include "pminternal.h"
#include "porttime.h"
#include "pmmacosx.h"

#include <stdio.h>
#include <string.h>

#include <CoreServices/CoreServices.h>
#include <CoreMIDI/MIDIServices.h>

#define PACKET_BUFFER_SIZE 1024

static MIDIClientRef	client = NULL;	/* Client handle to the MIDI server */
static MIDIPortRef		portIn = NULL;	/* Input port handle */
static MIDIPortRef		portOut = NULL;	/* Output port handle */

extern pm_fns_node pm_macosx_in_dictionary;
extern pm_fns_node pm_macosx_out_dictionary;

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

    return (high != 0xF0) ? high_lengths[high] : low_lengths[low];
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
	unsigned int eventIndex;
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
	CFStringEncoding defaultEncoding;
	CFStringRef deviceName;
	char nameBuf[256];
	int i;

	/* Determine the number of MIDI devices on the system */
	numDevices = MIDIGetNumberOfDevices();
	numInputs = MIDIGetNumberOfSources();
	numOutputs = MIDIGetNumberOfDestinations();

	/* Return prematurely if no devices exist on the system */
	if (numDevices <= 0) {
		return pmHostError;
	}

	/* Determine the default system character encording */
	defaultEncoding = CFStringGetSystemEncoding();

	/* Iterate over the MIDI input devices */
	for (i = 0; i < numInputs; i++) {
		endpoint = MIDIGetSource(i);
		if (endpoint == NULL) {
			continue;
		}

		/* Get the name of this device */
		MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &deviceName);
		CFStringGetCString(deviceName, nameBuf, 256, defaultEncoding);
		CFRelease(deviceName);

		/* Register this device with PortMidi */
		pm_add_device("CoreMIDI", nameBuf, TRUE, (void *)endpoint,
					  &pm_macosx_in_dictionary);
	}

	/* Iterate over the MIDI output devices */
	for (i = 0; i < numOutputs; i++) {
		endpoint = MIDIGetDestination(i);
		if (endpoint == NULL) {
			continue;
		}

		/* Get the name of this device */
		MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &deviceName);
		CFStringGetCString(deviceName, nameBuf, 256, defaultEncoding);
		CFRelease(deviceName);

		/* Register this device with PortMidi */
		pm_add_device("CoreMIDI", nameBuf, FALSE, (void *)endpoint,
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
	if (client != NULL)		MIDIClientDispose(client);
	if (portIn != NULL)		MIDIPortDispose(portIn);
	if (portOut != NULL)	MIDIPortDispose(portOut);

	return pmNoError;
}
