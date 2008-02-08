#ifndef PORT_MIDI_H
#define PORT_MIDI_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * PortMidi Portable Real-Time Audio Library
 * PortMidi API Header File
 * Latest version available at: http://www.cs.cmu.edu/~music/portmidi/
 *
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 * Copyright (c) 2001 Roger B. Dannenberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/* CHANGELOG FOR PORTMIDI -- THIS VERSION IS 1.0
 *
 * 21Jan02 RBD Added tests in Pm_OpenInput() and Pm_OpenOutput() to
 *               prevent opening an input as output and vice versa.
 *             Added comments and documentation.
 *             Implemented Pm_Terminate().
 *
 * 27Jun03 X. J. Scott (XJS)
 *             - Adding void arg to Pm_GetHostError() to stop compiler gripe.
 */

#ifndef FALSE
    #define FALSE 0
#endif
#ifndef TRUE
    #define TRUE 1
#endif


typedef enum {
    pmNoError = 0,

    pmHostError = -10000,
    pmInvalidDeviceId, /* out of range or 
                          output device when input is requested or
                          input device when output is requested */
    //pmInvalidFlag,
    pmInsufficientMemory,
    pmBufferTooSmall,
    pmBufferOverflow,
    pmBadPtr,
    pmInternalError
} PmError;

/*
    Pm_Initialize() is the library initialisation function - call this before
    using the library.
*/

PmError Pm_Initialize( void );

/*
    Pm_Terminate() is the library termination function - call this after
    using the library.
*/

PmError Pm_Terminate( void );

/*
    Return host specific error number. All host-specific errors are translated
    to the single error class pmHostError. To find out the original error
    number, call Pm_GetHostError().
    This can be called after a function returns a PmError equal to pmHostError.
*/
int Pm_GetHostError( void ); /* xjs - void param to stop compiler gripe */

/*
    Translate the error number into a human readable message.
*/
const char *Pm_GetErrorText( PmError errnum );


/*
    Device enumeration mechanism.

    Device ids range from 0 to Pm_CountDevices()-1.

    Devices may support input, output or both. Device 0 is always the "default"
    device. Other platform specific devices are specified by positive device
    ids.
*/

typedef int PmDeviceID;
#define pmNoDevice -1

typedef struct {
    int structVersion; 
    char const *interf;
    char const *name;
    int input; /* true iff input is available */
    int output; /* true iff output is available */
} PmDeviceInfo;


int Pm_CountDevices( void );
/*
    Pm_GetDefaultInputDeviceID(), Pm_GetDefaultOutputDeviceID()

    Return the default device ID or pmNoDevice if there is no devices.
    The result can be passed to Pm_OpenMidi().
    
    On the PC, the user can specify a default device by
    setting an environment variable. For example, to use device #1.

        set PM_RECOMMENDED_OUTPUT_DEVICE=1
    
    The user should first determine the available device ID by using
    the supplied application "pm_devs".
*/
PmDeviceID Pm_GetDefaultInputDeviceID( void );
PmDeviceID Pm_GetDefaultOutputDeviceID( void );

/*
    PmTimestamp is used to represent a millisecond clock with arbitrary
    start time. The type is used for all MIDI timestampes and clocks.
*/

typedef long PmTimestamp;

/* TRUE if t1 before t2? */
#define PmBefore(t1,t2) ((t1-t2) < 0)


/*
    Pm_GetDeviceInfo() returns a pointer to a PmDeviceInfo structure
    referring to the device specified by id.
    If id is out of range the function returns NULL.

    The returned structure is owned by the PortMidi implementation and must
    not be manipulated or freed. The pointer is guaranteed to be valid
    between calls to Pm_Initialize() and Pm_Terminate().
*/

const PmDeviceInfo* Pm_GetDeviceInfo( PmDeviceID id );


/*
    A single PortMidiStream is a descriptor for an open MIDI device.
*/

typedef void PortMidiStream;
#define PmStream PortMidiStream

typedef PmTimestamp (*PmTimeProcPtr)(void *time_info);


/*
    Pm_Open() opens a device; for either input or output.

    Port is the address of a PortMidiStream pointer which will receive
    a pointer to the newly opened stream.

    inputDevice is the id of the device used for input (see PmDeviceID above.)

    inputDriverInfo is a pointer to an optional driver specific data structure
    containing additional information for device setup or handle processing.
    inputDriverInfo is never required for correct operation. If not used
    inputDriverInfo should be NULL.

    outputDevice is the id of the device used for output (see PmDeviceID above.)

    outputDriverInfo is a pointer to an optional driver specific data structure
    containing additional information for device setup or handle processing.
    outputDriverInfo is never required for correct operation. If not used
    outputDriverInfo should be NULL.

    latency is the delay in milliseconds applied to timestamps to determine 
    when the output should actually occur.

    time_proc is a pointer to a procedure that returns time in milliseconds. It
    may be NULL, in which case a default millisecond timebase is used.

    time_info is a pointer passed to time_proc. 

    thru points to a PmMidi descriptor opened for output; Midi input will be
    copied to this output. To disable Midi thru, use NULL.

    return value:
    Upon success Pm_Open() returns PmNoError and places a pointer to a
    valid PortMidiStream in the stream argument.
    If a call to Pm_Open() fails a nonzero error code is returned (see
    PMError above) and the value of port is invalid.

*/

PmError Pm_OpenInput( PortMidiStream** stream,
                PmDeviceID inputDevice,
                void *inputDriverInfo,
                long bufferSize,
                PmTimeProcPtr time_proc,
                void *time_info,
                PmStream* thru );


PmError Pm_OpenOutput( PortMidiStream** stream,
                PmDeviceID outputDevice,
                void *outputDriverInfo,
                long bufferSize,
                PmTimeProcPtr time_proc,
                void *time_info,
                long latency );


/*
    Pm_Abort() terminates outgoing messages immediately
 */
PmError Pm_Abort( PortMidiStream* stream );
     
/*
    Pm_Close() closes a midi stream, flushing any pending buffers.
*/

PmError Pm_Close( PortMidiStream* stream );


/*
    Pm_Message() encodes a short Midi message into a long word. If data1
    and/or data2 are not present, use zero. The port parameter is the
    index of the Midi port if the device supports more than one.

    Pm_MessagePort(), Pm_MessageStatus(), Pm_MessageData1(), and 
    Pm_MessageData2() extract fields from a long-encoded midi message.
*/

#define Pm_Message(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

#define Pm_MessageStatus(msg) ((msg) & 0xFF)
#define Pm_MessageData1(msg) (((msg) >> 8) & 0xFF)
#define Pm_MessageData2(msg) (((msg) >> 16) & 0xFF)

/* All midi data comes in the form of PmEvent structures. A sysex
   message is encoded as a sequence of PmEvent structures, with each
   structure carrying 4 bytes of the message, i.e. only the first
   PmEvent carries the status byte.

   When receiving sysex messages, the sysex message is terminated
   by either an EOX status byte (anywhere in the 4 byte message) or
   by a non-real-time status byte in the low order byte of message.
   If you get a non-real-time status byte, it means the sysex message
   was somehow truncated. It is permissible to interleave real-time
   messages within sysex messages.
 */

typedef long PmMessage;

typedef struct {
    PmMessage      message;
    PmTimestamp    timestamp;
} PmEvent;


/*
    Pm_Read() retrieves midi data into a buffer, and returns the number
    of events read. Result is a non-negative number unless an error occurs, 
    in which case a PmError value will be returned.

    Buffer Overflow

    The problem: if an input overflow occurs, data will be lost, ultimately 
    because there is no flow control all the way back to the data source. 
    When data is lost, the receiver should be notified and some sort of 
    graceful recovery should take place, e.g. you shouldn't resume receiving 
    in the middle of a long sysex message.

    With a lock-free fifo, which is pretty much what we're stuck with to 
    enable portability to the Mac, it's tricky for the producer and consumer 
    to synchronously reset the buffer and resume normal operation.

    Solution: the buffer managed by PortMidi will be flushed when an overflow
    occurs. The consumer (Pm_Read()) gets an error message (pmBufferOverflow)
    and ordinary processing resumes as soon as a new message arrives. The
    remainder of a partial sysex message is not considered to be a "new
    message" and will be flushed as well.

*/

PmError Pm_Read( PortMidiStream *stream, PmEvent *buffer, long length );

/*
    Pm_Poll() tests whether input is available, returning TRUE, FALSE, or
    an error value. 
*/

PmError Pm_Poll( PortMidiStream *stream);

/* 
    Pm_Write() writes midi data from a buffer. This may contain short
    messages or sysex messages that are converted into a sequence of PmEvent
    structures. Use Pm_WriteSysEx() to write a sysex message stored as a
    contiguous array of bytes.
*/

PmError Pm_Write( PortMidiStream *stream, PmEvent *buffer, long length );

/*
    Pm_WriteShort() writes a timestamped non-system-exclusive midi message.
*/

PmError Pm_WriteShort( PortMidiStream *stream, PmTimestamp when, long msg);

/*
    Pm_WriteSysEx() writes a timestamped system-exclusive midi message.
*/
PmError Pm_WriteSysEx( PortMidiStream *stream, PmTimestamp when, char *msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* PORT_MIDI_H */
