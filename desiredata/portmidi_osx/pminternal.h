/* pminternal.h -- header for interface implementations */

/* this file is included by files that implement library internals */

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/* these are defined in system-specific file */
void *pm_alloc(size_t s);
void pm_free(void *ptr);

struct pm_internal_struct;

/* these do not use PmInternal because it is not defined yet... */
typedef PmError (*pm_write_fn)(struct pm_internal_struct *midi, 
                              PmEvent *buffer, long length);
typedef PmError (*pm_open_fn)(struct pm_internal_struct *midi, 
                              void *driverInfo);
typedef PmError (*pm_abort_fn)(struct pm_internal_struct *midi);
typedef PmError (*pm_close_fn)(struct pm_internal_struct *midi);

typedef struct {
    pm_write_fn write;
    pm_open_fn open;
    pm_abort_fn abort;
    pm_close_fn close;
} pm_fns_node, *pm_fns_type;

/* when open fails, the dictionary gets this set of functions: */
extern pm_fns_node pm_none_dictionary;

typedef struct {
    PmDeviceInfo pub;
    void *descriptor; /* system-specific data to open device */
    pm_fns_type dictionary;
} descriptor_node, *descriptor_type;


#define pm_descriptor_max 32
extern descriptor_node descriptors[pm_descriptor_max];
extern int descriptor_index;


typedef unsigned long (*time_get_proc_type)(void *time_info);

typedef struct pm_internal_struct {
    short write_flag;	/* MIDI_IN, or MIDI_OUT */
    int device_id; /* which device is open (index to descriptors) */
    PmTimeProcPtr time_proc; /* where to get the time */
    void *time_info;    /* pass this to get_time() */
    PmEvent *buffer; /* input or output buffer */
    long buffer_len; /* how big is the buffer */
    long latency; /* time delay in ms between timestamps and actual output */
                  /* set to zero to get immediate, simple blocking output */
                  /* if latency is zero, timestamps will be ignored */
    int overflow;    /* set to non-zero if input is dropped */
    int flush;  /* flag to drop incoming sysex data because of overflow */
    int sysex_in_progress; /* use for overflow management */
    struct pm_internal_struct *thru;
    PmTimestamp last_msg_time;   /* timestamp of last message */
    long head;
    long tail;
    pm_fns_type dictionary; /* implementation functions */
    void *descriptor;         /* system-dependent state */
} PmInternal;


typedef struct {
    long head;
    long tail;
    long len;
    long msg_size;
    long overflow;
    char *buffer;
} PmQueueRep;


PmError pm_init(void); /* defined in a system-specific file */
PmError pm_term(void); /* defined in a system-specific file */
int pm_in_device(int n, char *interf, char *device);
int pm_out_device(int n, char *interf, char *device);
PmError none_write(PmInternal *midi, PmEvent *buffer, long length);
PmError pm_success_fn(PmInternal *midi);
PmError pm_fail_fn(PmInternal *midi);
long pm_in_poll(PmInternal *midi);
long pm_out_poll(PmInternal *midi);

PmError pm_add_device(char *interf, char *name, int input, void *descriptor,
                      pm_fns_type dictionary);

void pm_enqueue(PmInternal *midi, PmEvent *event);


#ifdef __cplusplus
}
#endif

