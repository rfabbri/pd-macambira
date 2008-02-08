#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PD_PLUSPLUS_FACE
#include "desire.h"
#include "m_simd.h"
#include <jack/jack.h>
#include <regex.h>
#include <errno.h>
#define MAX_CLIENTS 100
#define NUM_JACK_PORTS 32
#define BUF_JACK 4096
static jack_nframes_t jack_out_max;
#define JACK_OUT_MAX  64
static jack_nframes_t jack_filled = 0;
static float jack_outbuf[NUM_JACK_PORTS*BUF_JACK];
static float  jack_inbuf[NUM_JACK_PORTS*BUF_JACK];
static int jack_started = 0;
static jack_port_t * input_port[NUM_JACK_PORTS];
static jack_port_t *output_port[NUM_JACK_PORTS];
static int outport_count = 0;
static jack_client_t *jack_client = 0;
char *jack_client_names[MAX_CLIENTS];
static int jack_dio_error;
static int jack_scheduler;
pthread_mutex_t jack_mutex;
pthread_cond_t jack_sem;
t_int jack_save_connection_state(t_int* dummy);
static void jack_restore_connection_state();
void run_all_idle_callbacks();

static int process (jack_nframes_t nframes, void *arg) {
	jack_out_max = max(int(nframes),JACK_OUT_MAX);
	if (jack_filled >= nframes) {
		if (jack_filled != nframes) post("Partial read");
		for (int j = 0; j < sys_outchannels;  j++) {
			float *out = (float *)jack_port_get_buffer(output_port[j], nframes);
			memcpy(out, jack_outbuf + (j * BUF_JACK), sizeof(float)*nframes);
		}
		for (int j = 0; j < sys_inchannels; j++) {
			float *in  = (float *)jack_port_get_buffer( input_port[j], nframes);
			memcpy(jack_inbuf + (j * BUF_JACK), in,   sizeof(float)*nframes);
		}
		jack_filled -= nframes;
	} else { /* PD could not keep up ! */
		if (jack_started) jack_dio_error = 1;
		for (int j = 0; j < outport_count;  j++) {
			float *out = (float *)jack_port_get_buffer (output_port[j], nframes);
			memset(out, 0, sizeof (float) * nframes);
		}
		memset(jack_outbuf,0,sizeof(jack_outbuf));
		jack_filled = 0;
	}
        /* tb: wait in the scheduler */
        /* pthread_cond_broadcast(&jack_sem); */
	return 0;
}

void sys_peakmeters();
extern int sys_meters;          /* true if we're metering */
static int dspticks_per_jacktick;
static void (*copyblock)(t_sample *dst,t_sample *src,int n);
static void (*zeroblock)(t_sample *dst,int n);
extern int canvas_dspstate;

static int cb_process (jack_nframes_t nframes, void *arg) {
	int timeout = int(nframes * 1e6 / sys_dacsr);
	if (canvas_dspstate == 0) {
		/* dsp is switched off, the audio is open ... */
		for (int j=0; j<sys_outchannels;  j++) {
			t_sample *out = (t_sample *)jack_port_get_buffer (output_port[j], nframes);
			zeroblock(out, dspticks_per_jacktick * sys_dacblocksize);
		}
		return 0;
	}
	int status = sys_timedlock(timeout);
	if (status)
		if (status == ETIMEDOUT) {
			/* we're late ... lets hope that jack doesn't kick us out */
			error("timeout %d", (timeout));
			sys_log_error(ERR_SYSLOCK);
			return 0;
		} else {
			post("sys_timedlock returned %d", status);
			return 0;
		}
	for (int i = 0; i != dspticks_per_jacktick; ++i) {
		for (int j=0; j<sys_inchannels; j++) {
			t_sample *in = (t_sample *)jack_port_get_buffer(input_port[j], nframes);
			copyblock(sys_soundin + j * sys_dacblocksize, in + i * sys_dacblocksize, sys_dacblocksize);
		}
		sched_tick(sys_time + sys_time_per_dsp_tick);
		for (int j=0; j<sys_outchannels;  j++) {
			t_sample *out = (t_sample *)jack_port_get_buffer (output_port[j], nframes);
			copyblock(out + i * sys_dacblocksize, sys_soundout + j * sys_dacblocksize, sys_dacblocksize);
		}
		if (sys_meters) sys_peakmeters();
		zeroblock(sys_soundout, sys_outchannels * sys_dacblocksize);
	}
    run_all_idle_callbacks();
    sys_unlock();
    return 0;
}

static int jack_srate (jack_nframes_t srate, void *arg) {
    sys_dacsr = srate;
    return 0;
}

void jack_close_audio(void);
static int jack_ignore_graph_callback = 0;
static t_int jack_shutdown_handler(t_int* none) {
	error("jack kicked us out ... trying to reconnect");
	jack_ignore_graph_callback = 1;
	/* clean up */
	jack_close_audio();
	/* try to reconnect to jack server */
	jack_open_audio(sys_inchannels, sys_outchannels, int(sys_dacsr), jack_scheduler);
	/* restore last connection state */
 	jack_restore_connection_state();
	jack_ignore_graph_callback = 0;
	return 0;
}

/* register idle callback in scheduler */
static void jack_shutdown (void *arg) {sys_callback(jack_shutdown_handler,0,0);}
static int jack_graph_order_callback(void* arg) {sys_callback(jack_save_connection_state,0,0); return 0;}

static char** jack_get_clients() {
    int num_clients = 0;
    regex_t port_regex;
    const char **jack_ports = jack_get_ports(jack_client, "", "", 0);
    regcomp(&port_regex, "^[^:]*", REG_EXTENDED);
    jack_client_names[0] = 0;
    /* Build a list of clients from the list of ports */
    for (int i=0; jack_ports[i] != 0; i++) {
        regmatch_t match_info;
        char tmp_client_name[100];
        /* extract the client name from the port name, using a regex that parses the clientname:portname syntax */
        regexec(&port_regex, jack_ports[i], 1, &match_info, 0);
        memcpy(tmp_client_name, &jack_ports[i][match_info.rm_so], match_info.rm_eo-match_info.rm_so);
        tmp_client_name[ match_info.rm_eo - match_info.rm_so ] = '\0';
        /* do we know about this port's client yet? */
        int client_seen = 0;
        for (int j=0; j<num_clients; j++) if (strcmp(tmp_client_name, jack_client_names[j])==0) client_seen = 1;
        if (!client_seen) {
            jack_client_names[num_clients] = (char*)getbytes(strlen(tmp_client_name) + 1);
            /* The alsa_pcm client should go in spot 0.  If this is the alsa_pcm client AND we are NOT about to put
               it in spot 0 put it in spot 0 and move whatever was already in spot 0 to the end. */
            if (strcmp("alsa_pcm",tmp_client_name)==0 && num_clients>0) {
                /* alsa_pcm goes in spot 0 */
		char* tmp = jack_client_names[ num_clients ];
		jack_client_names[num_clients] = jack_client_names[0];
		jack_client_names[0] = tmp;
		strcpy(jack_client_names[0], tmp_client_name);
            } else {
                /* put the new client at the end of the client list */
                strcpy(jack_client_names[num_clients], tmp_client_name);
            }
            num_clients++;
        }
    }
    /* for (int i=0; i<num_clients; i++) post("client: %s",jack_client_names[i]); */
    free(jack_ports);
    return jack_client_names;
}

/* Wire up all the ports of one client. */
static int jack_connect_ports(char *client) {
	char regex_pattern[100];
	static int entered = 0;
	if (entered) return 0;
	entered = 1;
	if (strlen(client) > 96)  return -1;
	sprintf(regex_pattern, "%s:.*", client);
	const char **jack_ports = jack_get_ports(jack_client, regex_pattern, 0, JackPortIsOutput);
	if (jack_ports)
		for (int i=0;jack_ports[i] != 0 && i < sys_inchannels;i++)
			if (jack_connect (jack_client, jack_ports[i], jack_port_name (input_port[i])))
				error("cannot connect input ports %s -> %s", jack_ports[i],jack_port_name(input_port[i]));
	free(jack_ports);
	jack_ports = jack_get_ports(jack_client, regex_pattern, 0, JackPortIsInput);
	if (jack_ports)
		for (int i=0;jack_ports[i] != 0 && i < sys_outchannels;i++)
			if (jack_connect (jack_client, jack_port_name (output_port[i]), jack_ports[i]))
				error("cannot connect output ports %s -> %s",jack_port_name(output_port[i]),jack_ports[i]);
	free(jack_ports);
	return 0;
}

static void jack_error(const char *desc) {}

int jack_open_audio_2(int inchans, int outchans, int rate, int scheduler);
int jack_open_audio(int inchans, int outchans, int rate, int scheduler) {
    jack_dio_error = 0;
    if (inchans==0 && outchans==0) return 0;
    int ret = jack_open_audio_2(inchans,outchans,rate,scheduler);
    if (ret) sys_setscheduler(0);
    return ret;
}

int jack_open_audio_2(int inchans, int outchans, int rate, int scheduler) {
    char port_name[80] = "";
    int new_jack = 0;
    if (outchans > NUM_JACK_PORTS) {post("%d output ports not supported, setting to %d",outchans, NUM_JACK_PORTS); outchans = NUM_JACK_PORTS;}
    if ( inchans > NUM_JACK_PORTS) {post( "%d input ports not supported, setting to %d", inchans, NUM_JACK_PORTS);  inchans = NUM_JACK_PORTS;}
    if (jack_client && scheduler != sys_getscheduler()) {
	jack_client_close(jack_client);
	jack_client = 0;
    }
    sys_setscheduler(scheduler);
    jack_scheduler = scheduler;
    /* set block copy/zero functions */
    if(SIMD_CHKCNT(sys_dacblocksize) && simd_runtime_check()) {
        copyblock = (void (*)(t_sample *,t_sample *,int))&copyvec_simd;
        zeroblock = &zerovec_simd;
    } else {
        copyblock = (void (*)(t_sample *,t_sample *,int))&copyvec;
        zeroblock = &zerovec;
    }
    /* try to become a client of the JACK server (we allow two pd's)*/
    if (!jack_client) {
        int client_iterator = 0;
	do {
            sprintf(port_name,"pure_data_%d",client_iterator);
            client_iterator++;
	} while (((jack_client = jack_client_new (port_name)) == 0) && client_iterator < 2);
        // jack spits out enough messages already, do not warn
	if (!jack_client) {sys_inchannels = sys_outchannels = 0; return 1;}
	jack_get_clients();
	/* tell the JACK server to call `process()' whenever there is work to be done.
           tb: adapted for callback based scheduling */
	if (scheduler == 1) {
		dspticks_per_jacktick = jack_get_buffer_size(jack_client) / sys_schedblocksize;
		jack_set_process_callback (jack_client, cb_process, 0);
	} else jack_set_process_callback (jack_client, process, 0);
	jack_set_error_function (jack_error);
#ifdef JACK_XRUN
	jack_set_xrun_callback (jack_client, jack_xrun, 0);
#endif
	jack_set_graph_order_callback(jack_client, jack_graph_order_callback, 0);
	/* tell the JACK server to call `srate()' whenever the sample rate of the system changes. */
	jack_set_sample_rate_callback (jack_client, jack_srate, 0);
	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it just decides to stop calling us. */
	jack_on_shutdown (jack_client, jack_shutdown, 0);
	for (int j=0;j<NUM_JACK_PORTS;j++) {
		input_port[j]=0;
		output_port[j]=0;
	}
	new_jack = 1;
    }
    /* display the current sample rate. once the client is activated
       (see below), you should rely on your own sample rate callback (see above) for this value. */
    int srate = jack_get_sample_rate (jack_client);
    sys_dacsr = srate;
    /* create the ports */
    for (int j = 0; j < inchans; j++) {
	sprintf(port_name, "input%d", j);
	if (!input_port[j])  input_port[j]  = jack_port_register(jack_client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    }
    for (int j = 0; j < outchans; j++) {
	sprintf(port_name, "output%d", j);
	if (!output_port[j]) output_port[j] = jack_port_register(jack_client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    }
    outport_count = outchans;
    /* tell the JACK server that we are ready to roll */
    if (new_jack) {
	if (jack_activate (jack_client)) {error("cannot activate client"); sys_inchannels = sys_outchannels = 0; return 1;}
	memset(jack_outbuf,0,sizeof(jack_outbuf));
	if (jack_client_names[0]) jack_connect_ports(jack_client_names[0]);
	pthread_mutex_init(&jack_mutex,0);
	pthread_cond_init(&jack_sem,0);
    }
    /* tb: get advance from jack server */
    sys_schedadvance = int((float)jack_port_get_total_latency(jack_client,output_port[0]) * 1000. / sys_dacsr * 1000);
    return 0;
}

void jack_close_audio() {
    if (!jack_client) return;
    jack_deactivate(jack_client);
    jack_started = 0;
    jack_client_close(jack_client);
    jack_client = 0;
    for (int i=0; i<NUM_JACK_PORTS; i++) {
	input_port[i] = 0;
	output_port[i] = 0;
    }
}

int jack_send_dacs() {
	int rtnval = SENDDACS_YES;
	int timeref = int(sys_getrealtime());
	if (!jack_client) return SENDDACS_NO;
	if (!sys_inchannels && !sys_outchannels) return SENDDACS_NO;
	if (jack_dio_error) {
		sys_log_error(ERR_RESYNC);
		jack_dio_error = 0;
	}
	if (jack_filled >= jack_out_max) return SENDDACS_NO;
	/* 	tb: wait in the scheduler */
/* 		pthread_cond_wait(&jack_sem,&jack_mutex); */
	jack_started = 1;
	float *fp = sys_soundout;
	for (int j=0; j<sys_outchannels; j++) {
		memcpy(jack_outbuf + j*BUF_JACK + jack_filled, fp, sys_dacblocksize*sizeof(float));
		fp += sys_dacblocksize;
	}
	fp = sys_soundin;
	for (int j=0; j<sys_inchannels; j++) {
		memcpy(fp, jack_inbuf + j*BUF_JACK + jack_filled,  sys_dacblocksize*sizeof(float));
		fp += sys_dacblocksize;
	}
	int timenow = int(sys_getrealtime());
	if (timenow-timeref > sys_sleepgrain*1e-6) rtnval = SENDDACS_SLEPT;
	memset(sys_soundout,0,sys_dacblocksize*sizeof(float)*sys_outchannels);
	jack_filled += sys_dacblocksize;
	return rtnval;
}

void jack_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize) {
    *canmulti = 0; /* supports multiple devices */
    int ndev = 1;
    for (int i=0; i<ndev; i++) {
        sprintf( indevlist + i * devdescsize, "JACK");
        sprintf(outdevlist + i * devdescsize, "JACK");
    }
    *nindevs = *noutdevs = ndev;
}

void jack_listdevs () {error("device listing not implemented for jack yet");}

static const char ** jack_in_connections[NUM_JACK_PORTS]; /* ports connected to the inputs */
static const char **jack_out_connections[NUM_JACK_PORTS]; /* ports connected to the outputs */

/* tb: save the current state of pd's jack connections */
t_int jack_save_connection_state(t_int* dummy) {
	if (jack_ignore_graph_callback) return 0;
	for (int i=0; i<NUM_JACK_PORTS; i++) {
		/* saving the inputs connections */
		if ( jack_in_connections[i]) free( jack_in_connections[i]);
		jack_in_connections[i] = i< sys_inchannels ? jack_port_get_all_connections(jack_client, input_port[i])  : 0;
		/* saving the outputs connections */
		if (jack_out_connections[i]) free(jack_out_connections[i]);
		jack_out_connections[i]= i<sys_outchannels ? jack_port_get_all_connections(jack_client, output_port[i]) : 0;
	}
	return 0;
}

/* todo: don't try to connect twice if we're both input and output host */
static void jack_restore_connection_state() {
	post("restoring connections");
	for (int i=0; i<NUM_JACK_PORTS; i++) {
		/* restoring the inputs connections */
		if (jack_in_connections[i]) {
			for (int j=0;;j++) {
				const char *port = jack_in_connections[i][j];
				if (!port) break; /* we've connected all incoming ports */
				int status = jack_connect(jack_client, port, jack_port_name(input_port[i]));
				if (status) error("cannot connect input ports %s -> %s", port, jack_port_name (input_port[i]));
			}
		}
		/* restoring the output connections */
		if (jack_out_connections[i]) {
			for (int j=0;;j++) {
				const char *port = jack_out_connections[i][j];
				if (!port) break; /* we've connected all outgoing ports */
				int status = jack_connect(jack_client, jack_port_name(output_port[i]), port);
				if (status) error("cannot connect output ports %s -> %s", jack_port_name(output_port[i]), port);
			}
		}
	}
}

struct t_audioapi jack_api = {
	0 /*jack_open_audio*/,
	jack_close_audio,
	jack_send_dacs,
	jack_getdevs,
};
