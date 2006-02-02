/* dssi~ - A DSSI host for PD 
 * 
 * Copyright 2006 Jamie Bullock and others 
 *
 * This file incorporates code from the following sources:
 * 
 * jack-dssi-host (BSD-style license): Copyright 2004 Chris Cannam, Steve Harris and Sean Bolton.
 *		   
 * Hexter (GPL license): Copyright (C) 2004 Sean Bolton and others.
 * 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/



/* TODO:
   1. Fix memory allocation issues.
   2. Restore Trivial Synth compatibility, i.e. is GUI fails, don't crash. - DONE!
   3. Reomve alsa.h dependency - Included header file!
   4. Get Hexter/Fluidsynth working - DONE!
   5. Get LTS working (full functionality i.e Pitch Bend, Control etc.) - DONE!
   6. Do OSC/GUI - DONE!
   7. Multiple instances -DONE!
   8. If OSC is received from another source - update the GUI.
   9. Patch saving/loading. -DONE (for Hexter)!
   10. Make GUI close when app closes, or new plugin loaded. -DONE!
   11. Make ll-scope work.
   12. Fix note hangs. - DONE - this was due to precedence problems in the PD patch!
   13. Fix inability to run two instances of the dssi~ external. - DONE!
   14. Fix exiting handler -DONE!
   15. Fix GUI close when PD quit - don't leave defunct process
   16. Fix free() call if plugin isn't loaded successfully -DONE!
   17. Implement GUI show/hide. -DONE!
   18. Fix segfault if dssi~ recieves on MIDI channel it doesn't have an instance for -DONE!
   19. Fix program crash when patch load if audio running. -DONE!
   20. Make info logged to pd console more meaningful
   21. Add more error checking.
   22. Fix: global polyphony handling - DONE!
   23. Fix: config malloc bugs
   24. Fix: exit call instance deactivate function - DONE!
   25. Check DSSI spec conformity!
   26. FIX: Why does is incorrect patch name chown when program is changed from GUI? - DONE! - query_programs must be sent for each configure call
   27. FIX: Generic valgrind error - also in jack-dssi-host.
 */


#include "dssi~.h"



static t_class *dssi_tilde_class;

/*From dx7_voice_data.c by Sean Bolton */

static char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*From dx7_voice.h by Sean Bolton */

typedef struct _dx7_patch_t {
	uint8_t data[128];
} dx7_patch_t;

typedef struct _dssi_instance {
   
   long             currentBank;
   long             currentProgram;
   int              pendingBankLSB;
   int              pendingBankMSB;
   int              pendingProgramChange;
   
   int plugin_ProgramCount;
   DSSI_Program_Descriptor *pluginPrograms;

   lo_address       uiTarget; /*osc stuff */
   int              ui_hidden;
   int              ui_show;
   int              uiNeedsProgramUpdate;
   char            *ui_osc_control_path;
   char            *ui_osc_configure_path;
   char            *ui_osc_program_path;
   char            *ui_osc_show_path;
   char            *ui_osc_hide_path;
   
   int *plugin_PortControlInNumbers; /*not sure if this should go here?*/
  
   char *osc_url_path;
   pid_t	gui_pid;
   
} t_dssi_instance;

typedef struct _dssi_tilde {
  t_object  x_obj;
  char *dll_path;/*abs path to plugin*/
  void *dll_handle;
  char *dir; /* project dircetory */
  LADSPA_Handle *instanceHandles; /*was handle*/
  t_dssi_instance *instances; 
  int n_instances;
  unsigned long *instanceEventCounts;
  snd_seq_event_t **instanceEventBuffers;

snd_seq_event_t midiEventBuf[EVENT_BUFSIZE];
/*static snd_seq_event_t **instanceEventBuffers;*/
int bufWriteIndex, bufReadIndex;
pthread_mutex_t midiEventBufferMutex;
/*static pthread_mutex_t listHandlerMutex = PTHREAD_MUTEX_INITIALIZER;*/
   
  DSSI_Descriptor_Function desc_func;
  const DSSI_Descriptor *descriptor;
  
  t_int	ports_in, ports_out, ports_controlIn, ports_controlOut;
  t_int plugin_ins;/* total audio input ports for plugin*/
  t_int plugin_outs;/* total audio output ports plugin*/
  t_int plugin_controlIns;/* total control input ports*/
  t_int plugin_controlOuts;/* total control output ports */

  unsigned long *plugin_ControlInPortNumbers; /*Array of input port numbers for the plugin */

  t_float **plugin_InputBuffers, **plugin_OutputBuffers; /* arrays of arrays for buffering audio for each audio port */
  t_float *plugin_ControlDataInput, *plugin_ControlDataOutput; /*arrays for control data for each port (1 item per 'run')*/
  lo_server_thread osc_thread;
  char *osc_url_base;
  char *dll_name;
  
  t_int time_ref; /*logical time reference */
  t_int sr;
  t_float sr_inv;
  t_int blksize;
  t_float f;
  
  t_float **outlets;
  
} t_dssi_tilde;

static void dssi_load_gui(t_dssi_tilde *x, int instance);

static int osc_message_handler(const char *path, const char *types, 
		lo_arg **argv, int argc, void *data, void *user_data);
static LADSPA_Data get_port_default(t_dssi_tilde *x, int port);
static void MIDIbuf(int type, int chan, int param, int val, t_dssi_tilde *x);



/*static void init_MidiEventBuf(snd_seq_event_t *MidiEventBuf){
	int i;
	for(i = 0; i < EVENT_BUFSIZE; i++){
		MidiEventBuf[i].*/


/*
 * encode_7in6
** Taken from gui_data.c by Sean Bolton **
 *
 * encode a block of 7-bit data, in base64-ish style
 */
char *
encode_7in6(uint8_t *data, int length)
{
    char *buffer;
    int in, reg, above, below, shift, out;
    int outchars = (length * 7 + 5) / 6;
    unsigned int sum = 0;

    if (!(buffer = (char *)malloc(25 + outchars)))
        return NULL;

    out = snprintf(buffer, 12, "%d ", length);

    in = reg = above = below = 0;
    while (outchars) {
        if (above == 6) {
            buffer[out] = base64[reg >> 7];
            reg &= 0x7f;
            above = 0;
            out++;
            outchars--;
        }
        if (below == 0) {
            if (in < length) {
                reg |= data[in] & 0x7f;
                sum += data[in];
            }
            below = 7;
            in++;
        }
        shift = 6 - above;
        if (below < shift) shift = below;
        reg <<= shift;
        above += shift;
        below -= shift;
    }

    snprintf(buffer + out, 12, " %d", sum);

    return buffer;
}




static void dssi_load(char *dll_path, void **dll_handle){
	
	*dll_handle = dlopen(dll_path, RTLD_NOW);
	if (*dll_handle){
		post("%s loaded successfully", dll_path);
	}
	else
		post("Failed: %s", dlerror());
	
}
	
static void portInfo(t_dssi_tilde *x){
	t_int i;
	for (i = 0; i < (t_int)x->descriptor->LADSPA_Plugin->PortCount; i++) {
		LADSPA_PortDescriptor pod =	
			x->descriptor->LADSPA_Plugin->PortDescriptors[i];
#if DEBUG
		post("Port %d: %s", i, x->descriptor->LADSPA_Plugin->PortNames[i]);
#endif
		if (LADSPA_IS_PORT_AUDIO(pod)) {
			if (LADSPA_IS_PORT_INPUT(pod)) ++x->plugin_ins;
			else if (LADSPA_IS_PORT_OUTPUT(pod)) ++x->plugin_outs;
		} 
		else if (LADSPA_IS_PORT_CONTROL(pod)) {
			if (LADSPA_IS_PORT_INPUT(pod)) ++x->plugin_controlIns;
			else if (LADSPA_IS_PORT_OUTPUT(pod)) ++x->plugin_controlOuts;
		}
	}
#if DEBUG
post("%d inputs, %d outputs, %d control inputs, %d control outs", x->plugin_ins, x->plugin_outs, x->plugin_controlIns, x->plugin_controlOuts);
#endif
}

static void dssi_assignPorts(t_dssi_tilde *x){
	int i;

#if DEBUG
	post("%d instances", x->n_instances);
#endif

    x->plugin_ins *= x->n_instances;
    x->plugin_outs *= x->n_instances;
    x->plugin_controlIns *= x->n_instances;
    x->plugin_controlOuts *= x->n_instances;

#if DEBUG
	post("%d plugin outs", x->plugin_outs);
#endif
	
	x->plugin_InputBuffers = 
		(float **)malloc(x->plugin_ins * sizeof(float *));
	x->plugin_OutputBuffers = 
		(float **)malloc(x->plugin_outs * sizeof(float *));
	x->plugin_ControlDataInput = 
		(float *)calloc(x->plugin_controlIns, sizeof(float));
	x->plugin_ControlDataOutput = 
		(float *)calloc(x->plugin_controlOuts, sizeof(float));
	for(i = 0; i < x->plugin_ins; i++)
		x->plugin_InputBuffers[i] = 
			(float *)calloc(x->blksize, sizeof(float));
	for(i = 0; i < x->plugin_outs; i++)
		x->plugin_OutputBuffers[i] = 
			(float *)calloc(x->blksize, sizeof(float));
	x->instanceEventBuffers = 
		(snd_seq_event_t **)malloc(x->n_instances * sizeof(snd_seq_event_t *));

	
    x->instanceHandles = (LADSPA_Handle *)malloc(x->n_instances *
                                              sizeof(LADSPA_Handle));
    x->instanceEventCounts = (unsigned long *)malloc(x->n_instances *
                                                  sizeof(unsigned long));

	for(i = 0; i < x->n_instances; i++){
		x->instanceEventBuffers[i] = (snd_seq_event_t *)malloc(EVENT_BUFSIZE *
								sizeof(snd_seq_event_t));

		x->instances[i].plugin_PortControlInNumbers = 
			(int *)malloc(x->descriptor->LADSPA_Plugin->PortCount * 
						  sizeof(int));/* hmmm... as we don't support instances of differing plugin types, we probably don't need to do this dynamically*/
	}
	
	x->plugin_ControlInPortNumbers = 
		(unsigned long *)malloc(sizeof(unsigned long) * x->plugin_controlIns);

#if DEBUG		
	post("Buffers assigned!");
#endif

}

static void dssi_init(t_dssi_tilde *x, t_int instance){

	x->instances[instance].pluginPrograms = NULL;
	x->instances[instance].currentBank = 0;
	x->instances[instance].currentProgram = 0;
	x->instances[instance].uiTarget = NULL;
	x->instances[instance].ui_osc_control_path = NULL;
	x->instances[instance].ui_osc_program_path = NULL;
	x->instances[instance].ui_osc_show_path = NULL;
	x->instances[instance].ui_osc_hide_path = NULL;
	x->instances[instance].ui_osc_configure_path = NULL;
	x->instances[instance].uiNeedsProgramUpdate = 0;
	x->instances[instance].pendingProgramChange = -1;
	x->instances[instance].pendingBankMSB = -1;
	x->instances[instance].pendingBankLSB = -1;
	x->instances[instance].ui_hidden = 1;
	x->instances[instance].ui_show = 0;
#if DEBUG		
	post("Instance %d initialized!", instance);
#endif


}

static void dssi_connectPorts(t_dssi_tilde *x, t_int instance){

	t_int i;

	for(i = 0; i < (t_int)x->descriptor->LADSPA_Plugin->PortCount; i++){
#if DEBUG
		post("PortCount: %d of %d", i, 
				x->descriptor->LADSPA_Plugin->PortCount);
#endif
		LADSPA_PortDescriptor pod =
					 x->descriptor->LADSPA_Plugin->PortDescriptors[i];

		x->instances[instance].plugin_PortControlInNumbers[i] = -1;
		
		if (LADSPA_IS_PORT_AUDIO(pod)) {
			if (LADSPA_IS_PORT_INPUT(pod)) {
				x->descriptor->LADSPA_Plugin->connect_port
					(x->instanceHandles[instance], i, 
					 x->plugin_InputBuffers[x->ports_in++]);
			} 
			else if (LADSPA_IS_PORT_OUTPUT(pod)) {
				x->descriptor->LADSPA_Plugin->connect_port
					(x->instanceHandles[instance], i, 
					 x->plugin_OutputBuffers[x->ports_out++]);
#if DEBUG
				post("Outport port %d connected", x->ports_out);
#endif
			}
		} 
		else if (LADSPA_IS_PORT_CONTROL(pod)) {
			if (LADSPA_IS_PORT_INPUT(pod)) {
				x->plugin_ControlInPortNumbers[x->ports_controlIn] = (unsigned long) i;
				x->instances[instance].plugin_PortControlInNumbers[i] = x->ports_controlIn;
				x->plugin_ControlDataInput[x->ports_controlIn] = 
					(t_float) get_port_default(x, i);
#if DEBUG
				post("default for port %d, controlIn, %d is %.2f",i,
						x->ports_controlIn, x->plugin_ControlDataInput[x->ports_controlIn]);
#endif

				x->descriptor->LADSPA_Plugin->connect_port
					(x->instanceHandles[instance], i, 
					 &x->plugin_ControlDataInput[x->ports_controlIn++]);

			} else if (LADSPA_IS_PORT_OUTPUT(pod)) {
				x->descriptor->LADSPA_Plugin->connect_port
					(x->instanceHandles[instance], i, 
					 &x->plugin_ControlDataOutput[x->ports_controlOut++]);
			}
		}
	}
	
#if DEBUG
post("ports connected!");
#endif
	
}

static void dssi_activate(t_dssi_tilde *x, t_int instance){

		x->descriptor->LADSPA_Plugin->activate(x->instanceHandles[instance]);
#if DEBUG
		post("plugin activated!");
#endif
}

static void osc_error(int num, const char *msg, const char *where)
{
    post("osc error %d in path %s: %s\n",num, where, msg);
}

static void query_programs(t_dssi_tilde *x, t_int instance) {
    int i;
#if DEBUG
post("querying programs");
#endif
    /* free old lot */
    if (x->instances[instance].pluginPrograms) {
        for (i = 0; i < x->instances[instance].plugin_ProgramCount; i++)
			free((void *)x->instances[instance].pluginPrograms[i].Name);
		x->instances[instance].pluginPrograms = NULL;
		x->instances[instance].plugin_ProgramCount = 0;
    }

    x->instances[instance].pendingBankLSB = -1;
    x->instances[instance].pendingBankMSB = -1;
    x->instances[instance].pendingProgramChange = -1;

    if (x->descriptor->get_program &&
        x->descriptor->select_program) {

	/* Count the plugins first */
		for (i = 0; x->descriptor->
					   get_program(x->instanceHandles[instance], i); ++i);

		if (i > 0) {
			x->instances[instance].plugin_ProgramCount = i;
			x->instances[instance].pluginPrograms = 
				(DSSI_Program_Descriptor *)malloc(i * sizeof(DSSI_Program_Descriptor)); 
			while (i > 0) {
				const DSSI_Program_Descriptor *descriptor;
				--i;
				descriptor = x->descriptor->
					get_program(x->instanceHandles[instance], i);
				x->instances[instance].pluginPrograms[i].Bank = 
					descriptor->Bank;
				x->instances[instance].pluginPrograms[i].Program = 
					descriptor->Program;
				x->instances[instance].pluginPrograms[i].Name = 
					strdup(descriptor->Name);
#if DEBUG
				post("program %d is MIDI bank %lu program %lu, named '%s'",i,
				   x->instances[instance].pluginPrograms[i].Bank,
				   x->instances[instance].pluginPrograms[i].Program,
				   x->instances[instance].pluginPrograms[i].Name);
#endif
			}
		}
	}
}

static LADSPA_Data get_port_default(t_dssi_tilde *x, int port)
{
	LADSPA_Descriptor *plugin = (LADSPA_Descriptor *)x->descriptor->LADSPA_Plugin;
    LADSPA_PortRangeHint hint = plugin->PortRangeHints[port];
    float lower = hint.LowerBound *
	(LADSPA_IS_HINT_SAMPLE_RATE(hint.HintDescriptor) ? x->sr : 1.0f);
    float upper = hint.UpperBound *
	(LADSPA_IS_HINT_SAMPLE_RATE(hint.HintDescriptor) ? x->sr : 1.0f);

    if (!LADSPA_IS_HINT_HAS_DEFAULT(hint.HintDescriptor)) {
	if (!LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor) ||
	    !LADSPA_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
	    /* No hint, its not bounded, wild guess */
	    return 0.0f;
	}

	if (lower <= 0.0f && upper >= 0.0f) {
	    /* It spans 0.0, 0.0 is often a good guess */
	    return 0.0f;
	}

	/* No clues, return minimum */
	return lower;
    }

    /* Try all the easy ones */
    
    if (LADSPA_IS_HINT_DEFAULT_0(hint.HintDescriptor)) {
	return 0.0f;
    } else if (LADSPA_IS_HINT_DEFAULT_1(hint.HintDescriptor)) {
	return 1.0f;
    } else if (LADSPA_IS_HINT_DEFAULT_100(hint.HintDescriptor)) {
	return 100.0f;
    } else if (LADSPA_IS_HINT_DEFAULT_440(hint.HintDescriptor)) {
	return 440.0f;
    }

    /* All the others require some bounds */

    if (LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
	if (LADSPA_IS_HINT_DEFAULT_MINIMUM(hint.HintDescriptor)) {
	    return lower;
	}
    }
    if (LADSPA_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
	if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(hint.HintDescriptor)) {
	    return upper;
	}
	if (LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
	    if (LADSPA_IS_HINT_DEFAULT_LOW(hint.HintDescriptor)) {
		return lower * 0.75f + upper * 0.25f;
	    } else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hint.HintDescriptor)) {
		return lower * 0.5f + upper * 0.5f;
	    } else if (LADSPA_IS_HINT_DEFAULT_HIGH(hint.HintDescriptor)) {
		return lower * 0.25f + upper * 0.75f;
	    }
	}
    }

    /* fallback */
    return 0.0f;
}



static int osc_debug_handler(const char *path, const char *types, lo_arg **argv,
                      int argc, void *data, t_dssi_tilde *x)
{
    int i;
    printf("got unhandled OSC message:\npath: <%s>\n", path);
    for (i=0; i<argc; i++) {
        printf("arg %d '%c' ", i, types[i]);
        lo_arg_pp(types[i], argv[i]);
        printf("\n");
    }
    return 1;
}

static void dssi_ProgramChange(t_dssi_tilde *x, int instance){
/* jack-dssi-host queues program changes by using  pending program change variables. In the audio callback, if a program change is received via MIDI it over writes the pending value (if any) set by the GUI. If unset, or processed the value will default back to -1. The following call to select_program is then made. I don't think it eventually needs to be done this way - i.e. do we need 'pending'? */ 
#if DEBUG
	post("executing program change");
#endif
	if (x->instances[instance].pendingProgramChange >= 0){           
		if (x->instances[instance].pendingBankLSB >= 0) {
			if (x->instances[instance].pendingBankMSB >= 0) {
				x->instances[instance].currentBank = x->instances[instance].pendingBankLSB + 128 * x->instances[instance].pendingBankMSB;
			} 
			else {
				x->instances[instance].currentBank = x->instances[instance].pendingBankLSB + 
					128 * (x->instances[instance].currentBank / 128);
			}
		} 
		else if (x->instances[instance].pendingBankMSB >= 0) {
			x->instances[instance].currentBank = (x->instances[instance].currentBank % 128) + 128 * x->instances[instance].pendingBankMSB;
		}

		x->instances[instance].currentProgram = x->instances[instance].pendingProgramChange;

	   if (x->descriptor->select_program) {
			x->descriptor->select_program(x->instanceHandles[instance],
							   x->instances[instance].currentBank, x->instances[instance].currentProgram);
	   }
	   if (x->instances[instance].uiNeedsProgramUpdate){
#if DEBUG
		   post("Updating GUI program");
#endif 
		   lo_send(x->instances[instance].uiTarget, 
				   x->instances[instance].ui_osc_program_path, "ii", 
				   x->instances[instance].currentBank, 
				   x->instances[instance].currentProgram);
	   }
		x->instances[instance].uiNeedsProgramUpdate = 0;
	    x->instances[instance].pendingProgramChange = -1;
		x->instances[instance].pendingBankMSB = -1;
		x->instances[instance].pendingBankLSB = -1;
	}
}

static int osc_program_handler(t_dssi_tilde *x, lo_arg **argv, int instance)
{
    unsigned long bank = argv[0]->i;
    unsigned long program = argv[1]->i;
    int i;
    int found = 0;

#if DEBUG
post("osc_program_hander active!");

post("%d programs", x->instances[instance].plugin_ProgramCount);
	
#endif
    for (i = 0; i < x->instances[instance].plugin_ProgramCount; ++i) {
		if (x->instances[instance].pluginPrograms[i].Bank == bank &&
			x->instances[instance].pluginPrograms[i].Program == program) {
			post("OSC: setting bank %u, program %u, name %s\n",
				bank, program, x->instances[instance].pluginPrograms[i].Name);
	    
			found = 1;
			break;
		}
    }

    if (!found) {
	printf(": OSC:  UI requested unknown program: bank %d, program %u: sending to plugin anyway (plugin should ignore it)\n", (int)bank,(int)program);
    }

    x->instances[instance].pendingBankMSB = bank / 128;
    x->instances[instance].pendingBankLSB = bank % 128;
    x->instances[instance].pendingProgramChange = program;
#if DEBUG
	post("bank = %d, program = %d, BankMSB = %d BankLSB = %d", bank, program, x->instances[instance].pendingBankMSB, x->instances[instance].pendingBankLSB);
#endif
	dssi_ProgramChange(x, instance);
	
    return 0;
}


static int osc_control_handler(t_dssi_tilde *x, lo_arg **argv, int instance)
{
    int port = argv[0]->i;
    LADSPA_Data value = argv[1]->f;

    x->plugin_ControlDataInput[x->instances[instance].plugin_PortControlInNumbers[port]] = value;
#if DEBUG
	post("OSC: port %d = %f", port, value);
#endif
    
    return 0;
}

static int osc_midi_handler(t_dssi_tilde *x, lo_arg **argv, t_int instance)
{

	int ev_type = 0, chan = 0;
/*	div_t divstruct; *//*division strucutre for integer division*/
#if DEBUG
	post("OSC: got midi request for"
	       "(%02x %02x %02x %02x)",
	       argv[0]->m[0], argv[0]->m[1], argv[0]->m[2], argv[0]->m[3]);
#endif
/*	divstruct = div(argv[0]->m[1], 16);
	chan = divstruct.rem + 1; */ /*Wrong! the GUI's all use channel 0 */
	chan = instance;
#if DEBUG
	post("channel: %d", chan);
#endif

	if(argv[0]->m[1] <= 239){
		if(argv[0]->m[1] >= 224)
			ev_type = SND_SEQ_EVENT_PITCHBEND;
		else if(argv[0]->m[1] >= 208)
			ev_type = SND_SEQ_EVENT_CHANPRESS;
		else if(argv[0]->m[1] >= 192)
			ev_type = SND_SEQ_EVENT_PGMCHANGE;
		else if(argv[0]->m[1] >= 176)
			ev_type = SND_SEQ_EVENT_CONTROLLER;
		else if(argv[0]->m[1] >= 160)
			ev_type = SND_SEQ_EVENT_KEYPRESS;
		else if(argv[0]->m[1] >= 144)
			ev_type = SND_SEQ_EVENT_NOTEON;
		else if(argv[0]->m[1] >= 128)
			ev_type = SND_SEQ_EVENT_NOTEOFF;
	}
	if(ev_type != 0)
		MIDIbuf(ev_type, chan, argv[0]->m[2], argv[0]->m[3], x);

    return 0;
}

static int osc_configure_handler(t_dssi_tilde *x, lo_arg **argv, int instance)
{
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;
    char *message;

#if DEBUG
	post("osc_configure_handler active!");
#endif
    
    if (x->descriptor->configure) {

	if (!strncmp(key, DSSI_RESERVED_CONFIGURE_PREFIX,
		     strlen(DSSI_RESERVED_CONFIGURE_PREFIX))) {
	    fprintf(stderr, ": OSC: UI for plugin '' attempted to use reserved configure key \"%s\", ignoring\n", key);
	    return 0;
	}
	
	    message = x->descriptor->configure(x->instanceHandles[instance], key, value);
	    if (message) {
		printf(": on configure  '%s', plugin '' returned error '%s'\n",
		        key, message);
		free(message);
	    }
		
	    query_programs(x, instance);

	}	    
    

    return 0;
}





static int osc_exiting_handler(t_dssi_tilde *x, lo_arg **argv, int instance){

#if DEBUG
	post("exiting handler called: Freeing ui_osc");
#endif
	lo_address_free(x->instances[instance].uiTarget);
     free(x->instances[instance].ui_osc_control_path);
     free(x->instances[instance].ui_osc_configure_path);
     free(x->instances[instance].ui_osc_hide_path);
     free(x->instances[instance].ui_osc_program_path);
     free(x->instances[instance].ui_osc_show_path); 
     x->instances[instance].uiTarget = NULL;
     x->instances[instance].ui_osc_control_path = NULL;
     x->instances[instance].ui_osc_configure_path = NULL;
     x->instances[instance].ui_osc_hide_path = NULL;
     x->instances[instance].ui_osc_program_path = NULL;
     x->instances[instance].ui_osc_show_path = NULL;

	x->instances[instance].ui_hidden = 1;

	return 0;
}

static int osc_update_handler(t_dssi_tilde *x, lo_arg **argv, int instance)
{
    const char *url = (char *)&argv[0]->s;
    const char *path;
    t_int i;
    char *host, *port;

#if DEBUG
	post("OSC: got update request from <%s>, instance %d", url, instance);
#endif


    if (x->instances[instance].uiTarget) 
		lo_address_free(x->instances[instance].uiTarget);
    host = lo_url_get_hostname(url);
    port = lo_url_get_port(url);
    x->instances[instance].uiTarget = lo_address_new(host, port);
    free(host);
    free(port);

    path = lo_url_get_path(url);

    if (x->instances[instance].ui_osc_control_path) 
		free(x->instances[instance].ui_osc_control_path);
    x->instances[instance].ui_osc_control_path = 
		(char *)malloc(strlen(path) + 10);
    sprintf(x->instances[instance].ui_osc_control_path, "%s/control", path);

    if (x->instances[instance].ui_osc_configure_path) 
		free(x->instances[instance].ui_osc_configure_path);
    x->instances[instance].ui_osc_configure_path = 
		(char *)malloc(strlen(path) + 12);
    sprintf(x->instances[instance].ui_osc_configure_path, "%s/configure", path);

    if (x->instances[instance].ui_osc_program_path) 
		free(x->instances[instance].ui_osc_program_path); 
    x->instances[instance].ui_osc_program_path = 
		(char *)malloc(strlen(path) + 10);
    sprintf(x->instances[instance].ui_osc_program_path, "%s/program", path);

    if (x->instances[instance].ui_osc_show_path) 
		free(x->instances[instance].ui_osc_show_path); 
    x->instances[instance].ui_osc_show_path = (char *)malloc(strlen(path) + 10);
    sprintf(x->instances[instance].ui_osc_show_path, "%s/show", path);
    
    if (x->instances[instance].ui_osc_hide_path) 
		free(x->instances[instance].ui_osc_hide_path); 
    x->instances[instance].ui_osc_hide_path = (char *)malloc(strlen(path) + 10);
    sprintf(x->instances[instance].ui_osc_hide_path, "%s/hide", path);

    free((char *)path);

    /* At this point a more substantial host might also call
     * configure() on the UI to set any state that it had remembered
     * for the plugin x.  But we don't remember state for
     * plugin xs (see our own configure() implementation in
     * osc_configure_handler), and so we have nothing to send except
     * the optional project directory. */

    if (x->dir) 
	lo_send(x->instances[instance].uiTarget, x->instances[instance].ui_osc_configure_path, "ss",DSSI_PROJECT_DIRECTORY_KEY, x->dir);
    

    /* Send current bank/program  (-FIX- another race...) */
	if (x->instances[instance].pendingProgramChange >= 0)
		dssi_ProgramChange(x, instance);
#if DEBUG
	post("pendingProgramChange = %d", x->instances[instance].pendingProgramChange);
#endif
	if (x->instances[instance].pendingProgramChange < 0) {
        unsigned long bank = x->instances[instance].currentBank;
        unsigned long program = x->instances[instance].currentProgram;
        x->instances[instance].uiNeedsProgramUpdate = 0;
        if (x->instances[instance].uiTarget) {
            lo_send(x->instances[instance].uiTarget, 
					x->instances[instance].ui_osc_program_path, 
					"ii", bank, program);
        }
    }

    /* Send control ports */
    for (i = 0; i < x->plugin_controlIns; i++) {
	lo_send(x->instances[instance].uiTarget, x->instances[instance].ui_osc_control_path, "if", 
			x->plugin_ControlInPortNumbers[i], x->plugin_ControlDataInput[i]);  
#if DEBUG
	post("Port: %d, Default value: %.2f", x->plugin_ControlInPortNumbers[i], x->plugin_ControlDataInput[i]);
#endif
    }

/* Send 'show' */
    if (x->instances[instance].ui_show) {
        lo_send(x->instances[instance].uiTarget, x->instances[instance].ui_osc_show_path, "");
        x->instances[instance].ui_hidden = 0;
        x->instances[instance].ui_show = 0;
    }



    return 0;
}

static void dssi_osc_setup(t_dssi_tilde *x, int instance){

	if(instance == 0){
		x->osc_thread = lo_server_thread_new(NULL, osc_error);
		char *osc_url_tmp;
		osc_url_tmp = lo_server_thread_get_url(x->osc_thread);
#if DEBUG
		post("string length of osc_url_tmp:%d", strlen(osc_url_tmp));
#endif
		x->osc_url_base = (char *)malloc(sizeof(char) 
				* (strlen(osc_url_tmp) + strlen("dssi") + 1)); 
		sprintf(x->osc_url_base, "%s%s", osc_url_tmp, "dssi");
		free(osc_url_tmp);
		lo_server_thread_add_method(x->osc_thread, NULL, NULL, 
				osc_message_handler, x);
		lo_server_thread_start(x->osc_thread);
	}
	x->instances[instance].osc_url_path = (char *)malloc(sizeof(char) * 
			(strlen(x->dll_name) + strlen(x->descriptor->LADSPA_Plugin->Label) + 			strlen("chan00") + 3));
	sprintf(x->instances[instance].osc_url_path, "%s/%s/chan%02d", x->dll_name, 
			x->descriptor->LADSPA_Plugin->Label, instance); 
#if DEBUG
post("OSC Path is: %s", x->instances[instance].osc_url_path);
	post("OSC thread started: %s", x->osc_url_base);
#endif
}

static void dssi_programs(t_dssi_tilde *x, int instance){

#if DEBUG
	post("Setting up program data");
#endif
	query_programs(x, instance);
	if (x->descriptor->select_program &&
			x->instances[instance].plugin_ProgramCount > 0) {

		/* select program at index 0 */
			unsigned long bank = 
				x->instances[instance].pluginPrograms[0].Bank;
			x->instances[instance].pendingBankMSB = bank / 128;
			x->instances[instance].pendingBankLSB = bank % 128;
			x->instances[instance].pendingProgramChange = 
				x->instances[instance].pluginPrograms[0].Program;
			x->instances[instance].uiNeedsProgramUpdate = 1;
	}
}

static void dssi_load_gui(t_dssi_tilde *x, int instance){
	t_int err = 0;
    	char *osc_url;
	char *gui_path;
	struct dirent *dir_entry = NULL;
	char *gui_base;
	size_t baselen;
	DIR *dp;
	char *gui_str;
	
	gui_base = (char *)malloc(baselen = sizeof(char) * (strlen(x->dll_path) - strlen(".so")));

	strncpy(gui_base, x->dll_path, baselen);
/*	gui_base = strndup(x->dll_path, baselen);*/
#if DEBUG
post("gui_base: %s", gui_base);
#endif

	gui_str = (char *)malloc(sizeof(char) * (strlen("channel 00") + 1));
	sprintf (gui_str,"channel %02d", instance);

#if DEBUG
	post("GUI name string, %s", gui_str);
#endif

	if(!(dp = opendir(gui_base))){
		post("can't open %s, unable to find GUI", gui_base);
		return;
	}
	else {
		while((dir_entry = readdir(dp))){
			if (dir_entry->d_name[0] == '.') continue;
			break;
		}
#if DEBUG
		post("GUI filename: %s", dir_entry->d_name);
#endif
	
	}

	gui_path = (char *)malloc(sizeof(char) * (strlen(gui_base) + strlen("/") + 
				strlen(dir_entry->d_name) + 1));
	

	sprintf(gui_path, "%s/%s", gui_base, dir_entry->d_name);
			
	free(gui_base);
#if DEBUG
	post("gui_path: %s", gui_path);
#endif
	
	osc_url = (char *)malloc
		(sizeof(char) * (strlen(x->osc_url_base) + 
						 strlen(x->instances[instance].osc_url_path) + 2));


	

	
/*	char osc_url[1024];*/
	sprintf(osc_url, "%s/%s", x->osc_url_base, 
			x->instances[instance].osc_url_path);
	post("Instance %d OSC URL is: %s",instance, osc_url);
#if DEBUG
	post("Trying to open GUI!");
#endif
	x->instances[instance].gui_pid = fork();
	if (x->instances[instance].gui_pid == 0){
		err = execlp(gui_path, gui_path, osc_url, dir_entry->d_name, 
				x->descriptor->LADSPA_Plugin->Label, gui_str, NULL);
	    perror("exec failed");
		exit(1); /* terminates the process */ 
	}

#if DEBUG
post("errorcode = %d", err);
#endif

	free(gui_path);
	free(osc_url);
	free(gui_str);
	if(dp){

#if DEBUG
		post("directory handle closed = %d", closedir(dp));
#endif
	}
}

static void MIDIbuf(int type, int chan, int param, int val, t_dssi_tilde *x){


	if(chan > x->n_instances - 1 || chan < 0){
		post("Note discarded: MIDI data is destined for a channel that doesn't exist");
		return;
	}

	
	t_int time_ref = x->time_ref;
	
pthread_mutex_lock(&x->midiEventBufferMutex);
	
	x->midiEventBuf[x->bufWriteIndex].time.time.tv_sec = 
		(t_int)(clock_gettimesince(time_ref) * .001); 
	x->midiEventBuf[x->bufWriteIndex].time.time.tv_nsec = 
		(t_int)(clock_gettimesince(time_ref) * 1000); /*actually usec - we can't store this in nsec! */
	
	if ((type == SND_SEQ_EVENT_NOTEON && val != 0) || 
			type != SND_SEQ_EVENT_NOTEON) {
		x->midiEventBuf[x->bufWriteIndex].type = type;
		switch (type) {
		case SND_SEQ_EVENT_NOTEON:
			x->midiEventBuf[x->bufWriteIndex].data.note.channel = chan;
			x->midiEventBuf[x->bufWriteIndex].data.note.note = param;
			x->midiEventBuf[x->bufWriteIndex].data.note.velocity = val;
			break;
		case SND_SEQ_EVENT_NOTEOFF:
			x->midiEventBuf[x->bufWriteIndex].data.note.channel = chan;
			x->midiEventBuf[x->bufWriteIndex].data.note.note = param;
			x->midiEventBuf[x->bufWriteIndex].data.note.velocity = val;
			break;
		case SND_SEQ_EVENT_CONTROLLER:
			x->midiEventBuf[x->bufWriteIndex].data.control.channel = chan;
			x->midiEventBuf[x->bufWriteIndex].data.control.param = param;
			x->midiEventBuf[x->bufWriteIndex].data.control.value = val;
			break;
		case SND_SEQ_EVENT_PITCHBEND:
			x->midiEventBuf[x->bufWriteIndex].data.control.channel = chan;
			x->midiEventBuf[x->bufWriteIndex].data.control.param = 0;
			x->midiEventBuf[x->bufWriteIndex].data.control.value = val;
			break;
		case SND_SEQ_EVENT_PGMCHANGE:
			x->instances[chan].pendingBankMSB = (param - 1) / 128;
			x->instances[chan].pendingBankLSB = (param - 1) % 128;
			x->instances[chan].pendingProgramChange = val;
			x->instances[chan].uiNeedsProgramUpdate = 1; 
#if DEBUG
			post("pgm chabge received in buffer: MSB: %d, LSB %d, prog: %d",
					x->instances[chan].pendingBankMSB, x->instances[chan].pendingBankLSB, val);
#endif
			dssi_ProgramChange(x, chan);
			break;
		}
	}
	else if (type == SND_SEQ_EVENT_NOTEON && val == 0) {
		x->midiEventBuf[x->bufWriteIndex].type = SND_SEQ_EVENT_NOTEOFF;
		x->midiEventBuf[x->bufWriteIndex].data.note.channel = chan;
		x->midiEventBuf[x->bufWriteIndex].data.note.note = param;
		x->midiEventBuf[x->bufWriteIndex].data.note.velocity = val;
	}

#if DEBUG
			post("MIDI received in buffer: chan %d, param %d, val %d",
					chan, param, val);
#endif
	    x->bufWriteIndex = (x->bufWriteIndex + 1) % EVENT_BUFSIZE;
    pthread_mutex_unlock(&x->midiEventBufferMutex); /**release mutex*/
}

static void dssi_show(t_dssi_tilde *x, t_int instance, t_int toggle){
	if(x->instances[instance].uiTarget){
		if (x->instances[instance].ui_hidden && toggle) {
			lo_send(x->instances[instance].uiTarget, 
				x->instances[instance].ui_osc_show_path, "");
			x->instances[instance].ui_hidden = 0;
		 }
		else if (!x->instances[instance].ui_hidden && !toggle) {
			lo_send(x->instances[instance].uiTarget, 
				x->instances[instance].ui_osc_hide_path, "");
			x->instances[instance].ui_hidden = 1;
		}
	}
	else if(toggle){
		x->instances[instance].ui_show = 1;
		dssi_load_gui(x, instance);
	}
	/*	if(x->instances[instance].uiTarget){
			lo_send(x->instances[instance].uiTarget, 
				x->instances[instance].ui_osc_show_path, "");
			x->instances[instance].ui_hidden = 0;
		}
	}*/

}


static void dssi_list(t_dssi_tilde *x, t_symbol *s, int argc, t_atom *argv) {
	char *msg_type;
	int ev_type = 0;
	msg_type = (char *)malloc(TYPE_STRING_SIZE);
	atom_string(argv, msg_type, TYPE_STRING_SIZE);
	int chan = (int)atom_getfloatarg(1, argc, argv) - 1;
	int param = (int)atom_getfloatarg(2, argc, argv);
	int val = (int)atom_getfloatarg(3, argc, argv);
	switch (msg_type[0]){
		case ASCII_n: ev_type = SND_SEQ_EVENT_NOTEON;
					  break;
		case ASCII_c: ev_type = SND_SEQ_EVENT_CONTROLLER;
					  break;
		case ASCII_p: ev_type = SND_SEQ_EVENT_PGMCHANGE;
					  break;
		case ASCII_b: ev_type = SND_SEQ_EVENT_PITCHBEND;
					  break;
	}
#if DEBUG
	post("initial midi NOTE:, arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d",ev_type,chan,param,val);
#endif
	if(ev_type != 0){
		MIDIbuf(ev_type, chan, param, val, x);
	}
	free(msg_type);
}

/* Method for list data through right inlet */
/*FIX: rewrite at some point - this is bad*/
static t_int dssi_config(t_dssi_tilde *x, t_symbol *s, int argc, t_atom *argv) {
	char *msg_type, *debug, *filename, *key, *value;
	msg_type = (char *)malloc(TYPE_STRING_SIZE);
	int instance = -1, pathlen, toggle; 
	int n_instances = x->n_instances;
	int count, maxpatches;
	t_float val;
	long filelength = 0;
	unsigned char *raw_patch_data = NULL;
	FILE *fp;
	size_t filename_length;
	dx7_patch_t *patchbuf, *firstpatch;
	atom_string(argv, msg_type, TYPE_STRING_SIZE);
	debug = NULL;
	key = NULL;	
	value = NULL;
	maxpatches = 128; 
	patchbuf = malloc(32 * sizeof(dx7_patch_t));
	firstpatch = &patchbuf[0];
	val = 0;


	/*FIX: Temporary - at the moment we always load the first 32 patches to 0 */
	if(strcmp(msg_type, "configure")){
		instance = (int)atom_getfloatarg(2, argc, argv) - 1;
		


	
	
/*	if(!strcmp(msg_type, "load") && x->descriptor->configure){
		filename = argv[1].a_w.w_symbol->s_name;
		instance = (int)atom_getfloatarg(2, argc, argv) - 1;
		post("loading patch: %s for instance %d", filename, instance);
		if(instance == -1){
			while(n_instances--)		
				debug = 
				x->descriptor->configure(
			x->instanceHandles[n_instances], "bank", filename);
		
		}
		else{
			debug = x->descriptor->configure(
			x->instanceHandles[instance], "bank", filename);
			dssi_programs(x, instance);
			lo_send(x->instances[instance].uiTarget, 
				x->instances[instance].ui_osc_program_path,"ii", 				0,0);
			x->instances[instance].uiNeedsProgramUpdate = 0;
		    	x->instances[instance].pendingProgramChange = -1;
			x->instances[instance].pendingBankMSB = -1;
			x->instances[instance].pendingBankLSB = -1;
		}

	}
	
*/
	if(!strcmp(msg_type, "load") && x->descriptor->configure){
		filename = argv[1].a_w.w_symbol->s_name;
		key = malloc(10 * sizeof(char)); /* holds "patchesN" */
		strcpy(key, "patches0");
		post("loading patch: %s for instance %d", filename, instance);
		fp = fopen(filename, "rb");
		
	/*From dx7_voice_data by Sean Bolton */
		if(fp == NULL){
			post("Unable to open patch file: %s", filename);
			return 0;
		}
		if (fseek(fp, 0, SEEK_END) || 
			(filelength = ftell(fp)) == -1 ||
			fseek(fp, 0, SEEK_SET)) {
				post("couldn't get length of patch file: %s", 
					filename);
			fclose(fp);
			return 0;
    		}
		if (filelength == 0) {
			post("patch file has zero length");
			fclose(fp);
			return 0;
		} else if (filelength > 16384) {
			post("patch file is too large");
			fclose(fp);
			return 0;
		}
		if (!(raw_patch_data = (unsigned char *)malloc(filelength))) 		     {
			post("couldn't allocate memory for raw patch file");
			fclose(fp);
			return 0;
		}
		if (fread(raw_patch_data, 1, filelength, fp) 
						!= (size_t)filelength) {
			post("short read on patch file: %s", filename);
			free(raw_patch_data);
			fclose(fp);
			return 0;
	    	}
		fclose(fp);
	/* At the moment we only support Hexter patches */
		if(strcmp(x->descriptor->LADSPA_Plugin->Label, "hexter"))
			post("Sorry dssi~ only supports	dx7 patches at the moment.");
		else{
		 /* figure out what kind of file it is */
		    filename_length = strlen(filename);
		    if (filename_length > 4 &&
			       !strcmp(filename + filename_length - 4, ".dx7") &&
			       filelength % DX7_VOICE_SIZE_PACKED == 0) {  
				/* It's a raw DX7 patch bank */

			count = filelength / DX7_VOICE_SIZE_PACKED;
			if (count > maxpatches)
			    count = maxpatches;
			memcpy(firstpatch, raw_patch_data, count * 
				DX7_VOICE_SIZE_PACKED);

		    } else if (filelength > 6 &&
			       raw_patch_data[0] == 0xf0 &&
			       raw_patch_data[1] == 0x43 &&
			       (raw_patch_data[2] & 0xf0) == 0x00 &&
			       raw_patch_data[3] == 0x09 &&
			       (raw_patch_data[4] == 0x10 || 
					raw_patch_data[4] == 0x20) &&  
			   /* 0x10 is actual, 0x20 matches typo in manual */
			       raw_patch_data[5] == 0x00) {  
			    /* It's a DX7 sys-ex 32 voice dump */

			if (filelength != DX7_DUMP_SIZE_BULK ||
			    raw_patch_data[DX7_DUMP_SIZE_BULK - 1] != 0xf7) {
			    post("badly formatted DX7 32 voice dump!");
			    count = 0;
		
#ifdef CHECKSUM_PATCH_FILES_ON_LOAD
			} else if (dx7_bulk_dump_checksum(&raw_patch_data[6],
				DX7_VOICE_SIZE_PACKED * 32) !=
				   raw_patch_data[DX7_DUMP_SIZE_BULK - 2]) {

			    post("DX7 32 voice dump with bad checksum!");
			    count = 0;

#endif
			} else {

			    count = 32;
			    if (count > maxpatches)
				count = maxpatches;
			    memcpy(firstpatch, raw_patch_data + 6, count * DX7_VOICE_SIZE_PACKED);

			}
		    } else {

			/* unsuccessful load */
			post("unknown patch bank file format!");
			count = 0;

		    }

		    free(raw_patch_data);
/*		    return count;*/
		    
		if(count == 32)
 		    value = encode_7in6((uint8_t *)&patchbuf[0].data[0], 
					count * DX7_VOICE_SIZE_PACKED);
	/*	    post("value = %s", value);	*/

/*FIX: make this generic do key value pairs in one go at end of dssi_config after they have been determined */	

				
				
	     }
        }	
	/*FIX: tidy up */
	if(!strcmp(msg_type, "dir") && x->descriptor->configure){
	/*	if(x->dir)
			free(x->dir); *//*Should never happen */
		pathlen = strlen(argv[1].a_w.w_symbol->s_name) + 2;
		x->dir = malloc((pathlen) * sizeof(char));
		atom_string(&argv[1], x->dir, pathlen);
	/*	instance = (int)atom_getfloatarg(2, argc, argv) - 1;*/
		post("Project directory for instance %d has been set to: %s", instance, x->dir);
		key = DSSI_PROJECT_DIRECTORY_KEY;
		value = x->dir;
	}
			/*	if(x->instances[instance].uiTarget)
			post("uiTarget: %s",x->instances[instance].uiTarget);
		if(x->instances[instance].ui_osc_configure_path)
			post("osc_config_path: %s",x->instances[instance].ui_osc_configure_path);
*//*		 lo_send(x->instances[instance].uiTarget, 
			x->instances[instance].ui_osc_configure_path, 
			"ss", DSSI_PROJECT_DIRECTORY_KEY, path);
*/	

	
	else if(!strcmp(msg_type, "dir"))
		post("%s %s: operation not supported", msg_type, 
			argv[1].a_w.w_symbol->s_name);
	
	if(!strcmp(msg_type, "show") || !strcmp(msg_type, "hide")){
		instance = (int)atom_getfloatarg(1, argc, argv) - 1;
		if(!strcmp(msg_type, "show"))
			toggle = 1;
		else
			toggle = 0;

		if(instance == -1){
			while(n_instances--)
				dssi_show(x, n_instances, toggle);
		}
		else
			dssi_show(x, instance, toggle);
	  }
	 if(!strcmp(msg_type, "reset")){
		instance = (int)atom_getfloatarg(1, argc, argv) - 1;
		if (instance == -1){
			for(instance = 0; instance < x->n_instances; instance++) 			{
			    if (x->descriptor->LADSPA_Plugin->deactivate &&
				  x->descriptor->LADSPA_Plugin->activate){ 
				x->descriptor->LADSPA_Plugin->deactivate
				    (x->instanceHandles[instance]);
				x->descriptor->LADSPA_Plugin->activate
				    (x->instanceHandles[instance]);
			    }
			}
		}
		else if (x->descriptor->LADSPA_Plugin->deactivate && 
			  x->descriptor->LADSPA_Plugin->activate) {
			x->descriptor->LADSPA_Plugin->deactivate
			    (x->instanceHandles[instance]);
			x->descriptor->LADSPA_Plugin->activate
			    (x->instanceHandles[instance]);
		}
        }
    }


	
	/*Use this to send arbitrary configure message to plugin */
	else if(!strcmp(msg_type, "configure")){
		key = 
		  malloc(strlen(argv[1].a_w.w_symbol->s_name) * sizeof(char)); 
		atom_string(&argv[1], key, TYPE_STRING_SIZE);
		
		if (argv[2].a_type == A_FLOAT){
			val = atom_getfloatarg(2, argc, argv);
			value = malloc(TYPE_STRING_SIZE * sizeof(char));
			sprintf(value, "%.2f", val);
		}
		else if(argv[2].a_type == A_SYMBOL){
			value = 
		         malloc(strlen(argv[2].a_w.w_symbol->s_name) * 
					 sizeof(char)); 
			atom_string(&argv[2], value, TYPE_STRING_SIZE);
		}		
		if(argv[3].a_type == A_FLOAT)
			instance = atom_getfloatarg(3, argc, argv) - 1;
	/*	else if(argv[3].a_type == A_SYMBOL)
			post("Argument 4 should be an integer!");
	*/
	}

	if(key != NULL && value != NULL){
		if(instance == -1){
			while(n_instances--){
				debug = 
				   x->descriptor->configure(
				    x->instanceHandles[n_instances], 
				     key, value);
				/*FIX - sort out 'pending' system so that new GUI's for instances get updated */
				if(x->instances[n_instances].uiTarget != NULL){
	/*		if(!strcmp(msg_type, "load"))*/
					lo_send(x->instances[n_instances].uiTarget, x->instances[n_instances].ui_osc_configure_path, "ss", key, value);
					query_programs(x, n_instances);
					/*FIX: better way to do this - OSC handler?*/
	 			}
			}
		}
		/*FIX: Put some error checking in here to make sure instance is valid*/
		else{
			debug = 
			  x->descriptor->configure(
			   x->instanceHandles[instance], 
			    key, value);
		/*	if(!strcmp(msg_type, "load"))*/
			if(x->instances[n_instances].uiTarget != NULL)
				lo_send(x->instances[instance].uiTarget, 
				  x->instances[instance].ui_osc_configure_path,
				   "ss", key, value);
			query_programs(x, instance);
		}
	}
#if DEBUG
	post("The plugin returned %s", debug);
#endif
/*		if(debug != NULL)
			free(debug);*/
free(msg_type);
free(patchbuf);
free(value);

if(key != NULL)
	free(key);
if(x->dir != NULL)
	free(x->dir);
return 0;
	
}

static void dssi_bang(t_dssi_tilde *x)
{
	post("Running instances of %s", x->descriptor->LADSPA_Plugin->Label);
}

static t_int *dssi_tilde_perform(t_int *w)
{
	  int N = (int)(w[1]);
	  t_dssi_tilde *x = (t_dssi_tilde *)(w[2]);
	  int i, n, timediff, framediff, instance = 0; 
 
	/*FIX -hmmm do we need this */
	if(x->n_instances){
	  for (i = 0; i < x->n_instances; i++)
		  x->instanceEventCounts[i] = 0;
	  
	  /*Instances = (LADSPA_Handle *)malloc(sizeof(LADSPA_Handle));*/
									
	  for (;x->bufReadIndex != x->bufWriteIndex; x->bufReadIndex = 
			  (x->bufReadIndex + 1) % EVENT_BUFSIZE) {
			
		    instance = x->midiEventBuf[x->bufReadIndex].data.note.channel;
		    /*This should never happen, but check anyway*/
		    if(instance > x->n_instances || instance < 0){
				post("%s: discarding spurious MIDI data, for instance %d", x->descriptor->LADSPA_Plugin->Label, instance);
#if DEBUG
				post("n_instances = %d", x->n_instances);
#endif
				continue;
			}
			  
			if (x->instanceEventCounts[instance] == EVENT_BUFSIZE){
				post("MIDI overflow on channel %d", instance);
				continue;
			}
			
			timediff = (t_int)(clock_gettimesince(x->time_ref) * 1000) - 
				 x->midiEventBuf[x->bufReadIndex].time.time.tv_nsec;
			framediff = (t_int)((t_float)timediff * .000001 / x->sr_inv); 
			
			if (framediff >= N || framediff < 0) 
				x->midiEventBuf[x->bufReadIndex].time.tick = 0;
			else
				x->midiEventBuf[x->bufReadIndex].time.tick = N - framediff - 1;
			
			x->instanceEventBuffers[instance][x->instanceEventCounts[instance]]
				= x->midiEventBuf[x->bufReadIndex];
#if DEBUG
post("%s, note received on channel %d", x->descriptor->LADSPA_Plugin->Label, x->instanceEventBuffers[instance][x->instanceEventCounts[instance]].data.note.channel);
#endif
			x->instanceEventCounts[instance]++; 
			
#if DEBUG
					post("Instance event count for instance %d of %d: %d\n",
					instance + 1, x->n_instances, x->instanceEventCounts[instance]);
#endif

	  }

	  	i = 0;
		while(i < x->n_instances){
			if(x->instanceHandles[i] && 
					x->descriptor->run_multiple_synths){
				  x->descriptor->run_multiple_synths
					  (x->n_instances, x->instanceHandles, 
					   (unsigned long)N, x->instanceEventBuffers,
					   &x->instanceEventCounts[0]);
				break; 
			}
			else if (x->instanceHandles[i]){
				  x->descriptor->run_synth(x->instanceHandles[i], 
						  (unsigned long)N, x->instanceEventBuffers[i],
						  x->instanceEventCounts[i]); 
				  i++;
			}
		}
		
		for(i = 0; i < x->n_instances; i++){
/*			t_float *out = x->outlets[i];*/
			for(n = 0; n < N; n++)
				x->outlets[i][n] = x->plugin_OutputBuffers[i][n];
/*				x->outlets[i][n] = 1;*/
		}
	}
	return (w+3);
}

static void dssi_tilde_dsp(t_dssi_tilde *x, t_signal **sp)
{
	/*FIX -hmmm what's the best way to do this? */
	if(x->n_instances){
		int n;
		t_float **outlets = x->outlets;

		for(n = 0; n < x->n_instances; n++)
			*outlets++ = sp[n+1]->s_vec;
	}		
	    dsp_add(dssi_tilde_perform, 2, sp[0]->s_n, x);
	
	
}

static void *dssi_tilde_new(t_symbol *s, t_int argc, t_atom *argv){

	t_dssi_tilde *x = (t_dssi_tilde *)pd_new(dssi_tilde_class);
	post("dssi~ %.2f\n a DSSI host for Pure Data\n by Jamie Bullock\nIncluding Code from jack-dssi-host\n by Chris Cannam, Steve Harris and Sean Bolton", VERSION);
	
    if (argc){
	int i, stop = 0;
	

	/*x->midiEventBufferMutex = PTHREAD_MUTEX_INITIALIZER;
	*/

	pthread_mutex_init(&x->midiEventBufferMutex, NULL);
	
	x->sr = (t_int)sys_getsr();
	x->sr_inv = 1 / (t_float)x->sr;

	x->dir = NULL;
	x->bufWriteIndex = x->bufReadIndex = 0;
	
	x->dll_name = (char *)malloc(sizeof(char) * 8);/* HARD CODED! */
	sprintf(x->dll_name, "%s", "plugin");       /* for now - Fix! */
	
	if(argc >= 2)
		x->n_instances = (t_int)argv[1].a_w.w_float;
	else
		x->n_instances = 1;
	
	x->instances = (t_dssi_instance *)malloc(sizeof(t_dssi_instance) * 
			x->n_instances);
	
#if DEBUG
	post("sr_inv = %.6f", x->sr_inv);
#endif
	x->time_ref = (t_int)clock_getlogicaltime;
	x->blksize = sys_getblksize();
	x->ports_in = x->ports_out = x->ports_controlIn = x->ports_controlOut = 0;
	
		x->dll_path = argv[0].a_w.w_symbol->s_name;
		dssi_load(x->dll_path, &x->dll_handle);
		if (x->dll_handle){
			x->desc_func = (DSSI_Descriptor_Function)dlsym(x->dll_handle, 
					"dssi_descriptor");
			if(x->descriptor = x->desc_func(0)){
#if DEBUG
				post("%s loaded successfully!", 
						x->descriptor->LADSPA_Plugin->Label);
#endif
				portInfo(x);
				dssi_assignPorts(x);
				for(i = 0; i < x->n_instances; i++){
					x->instanceHandles[i] = x->descriptor->LADSPA_Plugin->instantiate(x->descriptor->LADSPA_Plugin, x->sr);
					if (!x->instanceHandles[i]){
						post("instantiation of instance %d failed", i);
						stop = 1;
						break;
					}
				}
				if(!stop){
					for(i = 0;i < x->n_instances; i++)
						dssi_init(x, i);
					for(i = 0;i < x->n_instances; i++)
						dssi_connectPorts(x, i); /* fix this */
					for(i = 0;i < x->n_instances; i++)
						dssi_osc_setup(x, i);
					for(i = 0;i < x->n_instances; i++)
						dssi_activate(x, i);
					for(i = 0;i < x->n_instances; i++)
						dssi_programs(x, i);
#if LOADGUI
					for(i = 0;i < x->n_instances; i++)
						dssi_load_gui(x, i);
#endif
					for(i = 0;i < x->n_instances; i++)
						outlet_new(&x->x_obj, &s_signal);
					
					x->outlets = (t_float **)calloc(x->n_instances, 
							sizeof(t_float *));
				}
			}

			/*Create right inlet for configuration messages */
			inlet_new(&x->x_obj, &x->x_obj.ob_pd, 
					&s_list, gensym("config"));
		}
	
		else
			post("unknown error");
	}
	else
		post("No arguments given, please supply a path");
  return (void *)x;
}

static void dssi_free(t_dssi_tilde *x){

t_int instance;
  for(instance = 0; instance < x->n_instances; instance++) {
        /* no -- see comment in osc_exiting_handler */
        /* if (!instances[i].inactive) { */
            if (x->descriptor->LADSPA_Plugin->deactivate) {
                x->descriptor->LADSPA_Plugin->deactivate
                    (x->instanceHandles[instance]);
            }
        /* } */
        if (x->descriptor->LADSPA_Plugin &&
            x->descriptor->LADSPA_Plugin->cleanup) {
            x->descriptor->LADSPA_Plugin->cleanup
                (x->instanceHandles[instance]);
        }
    }

	
#if DEBUG
		  post("Calling dssi_free");
#endif
 if(x->dll_handle){ 
	instance = x->n_instances;
	free((LADSPA_Handle)x->instanceHandles);
	  free(x->plugin_ControlInPortNumbers); 
	  free((t_float *)x->plugin_InputBuffers);
	  free((t_float *)x->plugin_OutputBuffers);
	  free((snd_seq_event_t *)x->instanceEventBuffers);
	  free(x->instanceEventCounts);
	  free(x->plugin_ControlDataInput);
	  free(x->plugin_ControlDataOutput);

	  while(instance--){
		if(x->instances[instance].gui_pid){
#if DEBUG
		  post("Freeing GUI process PID = %d", x->instances[instance].gui_pid);
#endif
			kill(x->instances[instance].gui_pid, SIGKILL);
		} 
		  free(x->instances[instance].ui_osc_control_path);
		  free(x->instances[instance].ui_osc_configure_path);
		  free(x->instances[instance].ui_osc_program_path);
		  free(x->instances[instance].ui_osc_show_path);
		  free(x->instances[instance].ui_osc_hide_path);
		  free(x->instances[instance].osc_url_path);
		  free(x->instances[instance].plugin_PortControlInNumbers);
	  }
	  free((t_float *)x->outlets);
	  free(x->osc_url_base);
	  free(x->dll_name);
 }
}



void dssi_tilde_setup(void) {
	
	dssi_tilde_class = class_new(gensym("dssi~"), (t_newmethod)dssi_tilde_new,(t_method)dssi_free, sizeof(t_dssi_tilde), 0, A_GIMME, 0);
	class_addlist(dssi_tilde_class, dssi_list);
	class_addbang(dssi_tilde_class, dssi_bang);
  class_addmethod(dssi_tilde_class, (t_method)dssi_tilde_dsp, gensym("dsp"), 0);
	class_addmethod(dssi_tilde_class, (t_method)dssi_config, 
			gensym("config"), A_GIMME, 0);
  	class_sethelpsymbol(dssi_tilde_class, gensym("help-dssi"));
  CLASS_MAINSIGNALIN(dssi_tilde_class, t_dssi_tilde,f);
}

static int osc_message_handler(const char *path, const char *types, 
		lo_arg **argv,int argc, void *data, void *user_data)
{
#if DEBUG
	post("osc_message_handler active");
#endif
    int i, instance = 0;
    const char *method;
	char chantemp[2];
	t_dssi_tilde *x = (t_dssi_tilde *)(user_data);
	
    if (strncmp(path, "/dssi/", 6)){
#if DEBUG
		post("calling osc_debug_handler"); 
#endif
        return osc_debug_handler(path, types, argv, argc, data, x);
	}
	for (i = 0; i < x->n_instances; i++) {
		if (!strncmp(path + 6, x->instances[i].osc_url_path,
					 strlen(x->instances[i].osc_url_path))) {
			instance = i;
			break;
		}
	}
#if DEBUG
	for(i = 0; i < argc; i++){
		post("got osc request %c from instance %d, path: %s", 
			types[i],instance,path);
	}
#endif
	
    if (!x->instances[instance].osc_url_path){
#if DEBUG
		post("calling osc_debug_handler"); 
#endif
        return osc_debug_handler(path, types, argv, argc, data, x);
	}
    method = path + 6 + strlen(x->instances[instance].osc_url_path);
    if (*method != '/' || *(method + 1) == 0){
#if DEBUG
		post("calling osc_debug_handler"); 
#endif
        return osc_debug_handler(path, types, argv, argc, data, x);
	}
    method++;

    if (!strcmp(method, "configure") && argc == 2 && !strcmp(types, "ss")) {

#if DEBUG
		post("calling osc_configure_handler");
#endif
        return osc_configure_handler(x, argv, instance);

    } else if (!strcmp(method, "control") && argc == 2 && !strcmp(types, "if")) {
#if DEBUG
		post("calling osc_control_handler");
#endif
        return osc_control_handler(x, argv, instance);
	}

     else if (!strcmp(method, "midi") && argc == 1 && !strcmp(types, "m")) {
		
#if DEBUG
		post("calling osc_midi_handler");
#endif
        return osc_midi_handler(x, argv, instance);

    } else if (!strcmp(method, "program") && argc == 2 && !strcmp(types, "ii")){
#if DEBUG
		post("calling osc_program_handler"); 
#endif
        return osc_program_handler(x, argv, instance);

    } else if (!strcmp(method, "update") && argc == 1 && !strcmp(types, "s")){
#if DEBUG
		post("calling osc_update_handler"); 
#endif
        return osc_update_handler(x, argv, instance);
	

    } else if (!strcmp(method, "exiting") && argc == 0) {

        return osc_exiting_handler(x, argv, instance);
    }

    return osc_debug_handler(path, types, argv, argc, data, x);
}

