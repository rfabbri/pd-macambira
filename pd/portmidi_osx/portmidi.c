#include "stdlib.h"
#include "portmidi.h"
#include "pminternal.h"

#define is_empty(midi) ((midi)->tail == (midi)->head)

static int pm_initialized = FALSE;

int descriptor_index = 0;
descriptor_node descriptors[pm_descriptor_max];


/* pm_add_device -- describe interface/device pair to library 
 *
 * This is called at intialization time, once for each 
 * interface (e.g. DirectSound) and device (e.g. SoundBlaster 1)
 * The strings are retained but NOT COPIED, so do not destroy them!
 *
 * returns pmInvalidDeviceId if device memory is exceeded
 * otherwise returns pmNoError
 */
PmError pm_add_device(char *interf, char *name, int input, 
                      void *descriptor, pm_fns_type dictionary)
{
    if (descriptor_index >= pm_descriptor_max) {
        return pmInvalidDeviceId;
    }
    descriptors[descriptor_index].pub.interf = interf;
    descriptors[descriptor_index].pub.name = name;
    descriptors[descriptor_index].pub.input = input;
    descriptors[descriptor_index].pub.output = !input;
    descriptors[descriptor_index].descriptor = descriptor;
    descriptors[descriptor_index].dictionary = dictionary;
    descriptor_index++;
    return pmNoError;
}


PmError Pm_Initialize( void )
{
    if (!pm_initialized) {
        PmError err = pm_init(); /* defined by implementation specific file */
        if (err) return err;
        pm_initialized = TRUE;
    }
    return pmNoError;
}


PmError Pm_Terminate( void )
{
    PmError err = pmNoError;
    if (pm_initialized) {
        err = pm_term(); /* defined by implementation specific file */
        /* note that even when pm_term() fails, we mark portmidi as
           not initialized */
        pm_initialized = FALSE;
    }
    return err;
}


int Pm_CountDevices( void )
{
    PmError err = Pm_Initialize();
    if (err) return err;

    return descriptor_index;
}


const PmDeviceInfo* Pm_GetDeviceInfo( PmDeviceID id )
{
    PmError err = Pm_Initialize();
    if (err) return NULL;

    if (id >= 0 && id < descriptor_index) {
        return &descriptors[id].pub;
    }
    return NULL;
}


/* failure_fn -- "noop" function pointer */
/**/
PmError failure_fn(PmInternal *midi)
{
    return pmBadPtr;
}


/* pm_success_fn -- "noop" function pointer */
/**/
PmError pm_success_fn(PmInternal *midi)
{
    return pmNoError;
}


PmError none_write(PmInternal *midi, PmEvent *buffer, long length)
{
    return length; /* if we return 0, caller might get into a loop */
}

PmError pm_fail_fn(PmInternal *midi)
{
    return pmBadPtr;
}

static PmError none_open(PmInternal *midi, void *driverInfo)
{
    return pmBadPtr;
}

#define none_abort pm_fail_fn

#define none_close pm_fail_fn


pm_fns_node pm_none_dictionary = {
    none_write, none_open,
    none_abort, none_close };


/* Pm_Read -- read up to length longs from source into buffer */
/*
 * returns number of longs actually read, or error code
 When the reader wants data:
    if overflow_flag:
        do not get anything
        empty the buffer (read_ptr = write_ptr)
        clear overflow_flag
        return pmBufferOverflow
    get data
    return number of messages


 */
PmError Pm_Read( PortMidiStream *stream, PmEvent *buffer, long length)
{
    PmInternal *midi = (PmInternal *) stream;
    int n = 0;
    long head = midi->head;
    while (head != midi->tail && n < length) {
        *buffer++ = midi->buffer[head++];
        if (head == midi->buffer_len) head = 0;
        n++;
    }
    midi->head = head;
    if (midi->overflow) {
        midi->head = midi->tail;
        midi->overflow = FALSE;
        return pmBufferOverflow;
    }
    return n;
}


PmError Pm_Poll( PortMidiStream *stream )
{
    PmInternal *midi = (PmInternal *) stream;
    return midi->head != midi->tail;
}


PmError Pm_Write( PortMidiStream *stream, PmEvent *buffer, long length)
{
    PmInternal *midi = (PmInternal *) stream;
    return (*midi->dictionary->write)(midi, buffer, length);
}


PmError Pm_WriteShort( PortMidiStream *stream, long when, long msg)
{
    PmEvent event;
    event.timestamp = when;
    event.message = msg;
    return Pm_Write(stream, &event, 1);
}


PmError Pm_OpenInput( PortMidiStream** stream,
                PmDeviceID inputDevice,
                void *inputDriverInfo,
                long bufferSize,
                PmTimeProcPtr time_proc,
                void *time_info,
                PmStream *thru)
{
    PmInternal *midi;

    PmError err = Pm_Initialize();
    if (err) return err;

    if (inputDevice < 0 || inputDevice >= descriptor_index) {
        return pmInvalidDeviceId;
    }

    if (!descriptors[inputDevice].pub.input) {
        return pmInvalidDeviceId;
    }

    midi = (PmInternal *) malloc(sizeof(PmInternal));
    *stream = midi;             
    if (!midi) return pmInsufficientMemory;

    midi->head = 0;
    midi->tail = 0;
    midi->dictionary = &pm_none_dictionary;
    midi->overflow = FALSE;
    midi->flush = FALSE;
    midi->sysex_in_progress = FALSE;
    midi->buffer_len = bufferSize;
    midi->buffer = (PmEvent *) pm_alloc(sizeof(PmEvent) * midi->buffer_len);
    if (!midi->buffer) return pmInsufficientMemory;
    midi->latency = 0;
    midi->thru = thru;
    midi->time_proc = time_proc;
    midi->time_info = time_info;
    midi->device_id = inputDevice;
    midi->dictionary = descriptors[inputDevice].dictionary;
    midi->write_flag = FALSE;
    err = (*midi->dictionary->open)(midi, inputDriverInfo);
    if (err) {
        pm_free(midi->buffer);      
        *stream = NULL;
    }
    return err;
}


PmError Pm_OpenOutput( PortMidiStream** stream,
                       PmDeviceID outputDevice,
                       void *outputDriverInfo,
                       long bufferSize,
                       PmTimeProcPtr time_proc,
                       void *time_info,
                       long latency )
{
    PmInternal *midi;

    PmError err = Pm_Initialize();
    if (err) return err;

    if (outputDevice < 0 || outputDevice >= descriptor_index) {
        return pmInvalidDeviceId;
    }

    if (!descriptors[outputDevice].pub.output) {
        return pmInvalidDeviceId;
    }

    midi = (PmInternal *) pm_alloc(sizeof(PmInternal));
    *stream = midi;                 
    if (!midi) return pmInsufficientMemory;

    midi->head = 0;
    midi->tail = 0;
    midi->buffer_len = bufferSize;
    midi->buffer = NULL;
    midi->device_id = outputDevice;
    midi->dictionary = descriptors[outputDevice].dictionary;
    midi->time_proc = time_proc;
    midi->time_info = time_info;
    midi->latency = latency;
    midi->write_flag = TRUE;
    err = (*midi->dictionary->open)(midi, outputDriverInfo);
    if (err) {
        *stream = NULL;
        pm_free(midi); // Fixed by Ning Hu, Sep.2001
    }
    return err;
}


PmError Pm_Abort( PortMidiStream* stream )
{
    PmInternal *midi = (PmInternal *) stream;
    return (*midi->dictionary->abort)(midi);
}


PmError Pm_Close( PortMidiStream *stream )
{
    PmInternal *midi = (PmInternal *) stream;
    return (*midi->dictionary->close)(midi);
}


const char *Pm_GetErrorText( PmError errnum )
{
    const char *msg;

    switch(errnum)
    {
    case pmNoError:                  msg = "Success"; break;
    case pmHostError:                msg = "Host error."; break;
    case pmInvalidDeviceId:          msg = "Invalid device ID."; break;
    case pmInsufficientMemory:       msg = "Insufficient memory."; break;
    case pmBufferTooSmall:           msg = "Buffer too small."; break;
    case pmBadPtr:                   msg = "Bad pointer."; break;
    case pmInternalError:            msg = "Internal PortMidi Error."; break;
    default:                         msg = "Illegal error number."; break;
    }
    return msg;
}


long pm_next_time(PmInternal *midi)
{
    return midi->buffer[midi->head].timestamp;
}


/* source should not enqueue data if overflow is set */
/*
  When producer has data to enqueue:
    if buffer is full:
        set overflow_flag and flush_flag
        return
    else if overflow_flag:
        return
    else if flush_flag:
        if sysex message is in progress:
            return
        else:
            clear flush_flag
            // fall through to enqueue data
    enqueue the data

 */
void pm_enqueue(PmInternal *midi, PmEvent *event)
{
    long tail = midi->tail;
    midi->buffer[tail++] = *event;
    if (tail == midi->buffer_len) tail = 0;
    if (tail == midi->head || midi->overflow) {
        midi->overflow = TRUE;
        midi->flush = TRUE;
        return;
    }
    if (midi->flush) {
        if (midi->sysex_in_progress) return;
        else midi->flush = FALSE;
    }
    midi->tail = tail;
}


int pm_queue_full(PmInternal *midi)
{
    long tail = midi->tail + 1;
    if (tail == midi->buffer_len) tail = 0;
    return tail == midi->head;
}



