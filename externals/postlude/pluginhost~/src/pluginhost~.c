/* pluginhost~ - A plugin host for Pd
 *
 * Copyright (C) 2006 Jamie Bullock and others
 *
 * This file incorporates code from the following sources:
 *
 * jack-dssi-host (BSD-style license): Copyright 2004 Chris Cannam, Steve Harris and Sean Bolton.
 *
 * Hexter (GPL license): Copyright (C) 2004 Sean Bolton and others.
 *
 * plugin~ (GPL license): Copyright (C) 2000 Jarno Seppänen, remIXed 2005
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

#include <stdarg.h>
#include <assert.h>

#include "pluginhost~.h"
#include "jutils.h"

static t_class *ph_tilde_class;

/*From dx7_voice_data.c by Sean Bolton */

static char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
 * encode_7in6
 ** Taken from gui_data.c by Sean Bolton **
 *
 * encode a block of 7-bit data, in base64-ish style
 */
    static char *
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

/*
 * dx7_bulk_dump_checksum
 ** Taken from dx7_voice_data.c by Sean Bolton **
 */
    static int
dx7_bulk_dump_checksum(uint8_t *data, int length)
{
    int sum = 0;
    int i;

    for (i = 0; i < length; sum -= data[i++]);
    return sum & 0x7F;
}

static DSSI_Descriptor *ladspa_to_dssi(LADSPA_Descriptor *ladspaDesc)
{
    DSSI_Descriptor *dssiDesc;
    dssiDesc = (DSSI_Descriptor *)calloc(1, sizeof(DSSI_Descriptor));
    ((DSSI_Descriptor *)dssiDesc)->DSSI_API_Version = 1;
    ((DSSI_Descriptor *)dssiDesc)->LADSPA_Plugin = 
        (LADSPA_Descriptor *)ladspaDesc;
    return (DSSI_Descriptor *)dssiDesc;
}

static void ph_tilde_port_info(ph_tilde *x)
{
    t_int i;

    for (i = 0; i < (t_int)x->descriptor->LADSPA_Plugin->PortCount; i++) {

        x->port_info[i].type.a_type = A_SYMBOL;
        x->port_info[i].data_type.a_type = A_SYMBOL;
        x->port_info[i].name.a_type = A_SYMBOL;
        x->port_info[i].upper_bound.a_type = A_FLOAT;
        x->port_info[i].lower_bound.a_type = A_FLOAT;
        x->port_info[i].p_default.a_type = A_FLOAT;

        LADSPA_PortDescriptor pod =	
            x->descriptor->LADSPA_Plugin->PortDescriptors[i];
        ph_debug_post("Port %d: %s", i, x->descriptor->LADSPA_Plugin->PortNames[i]);

        if (LADSPA_IS_PORT_AUDIO(pod)) {
            x->port_info[i].data_type.a_w.w_symbol = 
                gensym("audio");
            if (LADSPA_IS_PORT_INPUT(pod)){
                x->port_info[i].type.a_w.w_symbol = 
                    gensym("in");
                ++x->plugin_ins;
            }
            else if (LADSPA_IS_PORT_OUTPUT(pod)){
                x->port_info[i].type.a_w.w_symbol = 
                    gensym("out");
                ++x->plugin_outs;
            }
        } 
        else if (LADSPA_IS_PORT_CONTROL(pod)) {
            x->port_info[i].data_type.a_w.w_symbol = 
                gensym("control");
            if (LADSPA_IS_PORT_INPUT(pod)){
                x->port_info[i].type.a_w.w_symbol = 
                    gensym("in");
                ++x->plugin_controlIns;
            }
            else if (LADSPA_IS_PORT_OUTPUT(pod)){
                ++x->plugin_controlOuts;
                x->port_info[i].type.a_w.w_symbol = 
                    gensym("out");
            }
        }
        if (LADSPA_IS_HINT_BOUNDED_BELOW(
                    x->descriptor->LADSPA_Plugin->PortRangeHints[i].HintDescriptor))
            x->port_info[i].lower_bound.a_w.w_float = 
                x->descriptor->LADSPA_Plugin->
                PortRangeHints[i].LowerBound;
        else
            x->port_info[i].lower_bound.a_w.w_float = 0;

        if (LADSPA_IS_HINT_BOUNDED_ABOVE(
                    x->descriptor->LADSPA_Plugin->PortRangeHints[i].HintDescriptor))
            x->port_info[i].upper_bound.a_w.w_float = 
                x->descriptor->LADSPA_Plugin->
                PortRangeHints[i].UpperBound;
        else
            x->port_info[i].lower_bound.a_w.w_float = 1;

        x->port_info[i].p_default.a_w.w_float = (float)
            get_port_default(x, i);

        x->port_info[i].name.a_w.w_symbol = 
            gensym ((char *)
                    x->descriptor->LADSPA_Plugin->PortNames[i]);
    }
    ph_debug_post("%d inputs, %d outputs, %d control inputs, %d control outs", x->plugin_ins, x->plugin_outs, x->plugin_controlIns, x->plugin_controlOuts);

}

static void ph_tilde_assign_ports(ph_tilde *x)
{
    unsigned int i;

    ph_debug_post("%d instances", x->n_instances);


    x->plugin_ins *= x->n_instances;
    x->plugin_outs *= x->n_instances;
    x->plugin_controlIns *= x->n_instances;
    x->plugin_controlOuts *= x->n_instances;

    ph_debug_post("%d plugin outs", x->plugin_outs);


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

    ph_debug_post("Buffers assigned!");


}

static void ph_tilde_init_instance(ph_tilde *x, t_int instance)
{

    x->instances[instance].pluginPrograms = NULL;
    x->instances[instance].currentBank = 0;
    x->instances[instance].currentProgram = 0;
    x->instances[instance].ui_osc_control_path = NULL;
    x->instances[instance].ui_osc_program_path = NULL;
    x->instances[instance].ui_osc_show_path = NULL;
    x->instances[instance].ui_osc_hide_path = NULL;
    x->instances[instance].ui_osc_quit_path = NULL;
    x->instances[instance].ui_osc_configure_path = NULL;
    x->instances[instance].uiNeedsProgramUpdate = 0;
    x->instances[instance].pendingProgramChange = -1;
    x->instances[instance].plugin_ProgramCount = 0;
    x->instances[instance].pendingBankMSB = -1;
    x->instances[instance].pendingBankLSB = -1;
    x->instances[instance].ui_hidden = 1;
    x->instances[instance].ui_show = 0;

    ph_debug_post("Instance %d initialized!", instance);

}

static void ph_tilde_connect_ports(ph_tilde *x, t_int instance)
{

    t_int i;

    for(i = 0; i < (t_int)x->descriptor->LADSPA_Plugin->PortCount; i++){
        ph_debug_post("PortCount: %d of %d", i, 
                x->descriptor->LADSPA_Plugin->PortCount);

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
                ph_debug_post("Audio Input port %d connected", x->ports_in);
                post("Audio Output port %d connected", x->ports_out);

            }
        } 
        else if (LADSPA_IS_PORT_CONTROL(pod)) {
            if (LADSPA_IS_PORT_INPUT(pod)) {
                x->plugin_ControlInPortNumbers[x->ports_controlIn] = (unsigned long) i;
                x->instances[instance].plugin_PortControlInNumbers[i] = x->ports_controlIn;
                x->plugin_ControlDataInput[x->ports_controlIn] = 
                    (t_float) get_port_default(x, i);
                ph_debug_post("default for port %d, controlIn, %d is %.2f",i,
                        x->ports_controlIn, x->plugin_ControlDataInput[x->ports_controlIn]);


                x->descriptor->LADSPA_Plugin->connect_port
                    (x->instanceHandles[instance], i, 
                     &x->plugin_ControlDataInput[x->ports_controlIn++]);

            } else if (LADSPA_IS_PORT_OUTPUT(pod)) {
                x->descriptor->LADSPA_Plugin->connect_port
                    (x->instanceHandles[instance], i, 
                     &x->plugin_ControlDataOutput[x->ports_controlOut++]);
            }
            ph_debug_post("Control Input port %d connected", x->ports_controlIn);
            post("Control Output port %d connected", x->ports_controlOut);

        }
    }

    ph_debug_post("ports connected!");


}

static void ph_tilde_activate_plugin(ph_tilde *x, t_int instance)
{

    if(x->descriptor->LADSPA_Plugin->activate){
        ph_debug_post("trying to activate instance: %d", instance);

        x->descriptor->LADSPA_Plugin->activate(x->instanceHandles[instance]);
    }
    ph_debug_post("plugin activated!");

}

static void ph_tilde_deactivate_plugin(ph_tilde *x, t_float instance_f)
{

    t_int instance = (t_int)instance_f;
    if(x->descriptor->LADSPA_Plugin->deactivate)
        x->descriptor->LADSPA_Plugin->deactivate(x->instanceHandles[instance]);
    ph_debug_post("plugin deactivated!");

}

static void osc_error(int num, const char *msg, const char *where)
{
    post("pluginhost~: osc error %d in path %s: %s\n",num, where, msg);
}

static void query_programs(ph_tilde *x, t_int instance)
{
    unsigned int i;
    ph_debug_post("querying programs");

    /* free old lot */
    if (x->instances[instance].pluginPrograms) {
        for (i = 0; i < x->instances[instance].plugin_ProgramCount; i++)
            free((void *)x->instances[instance].pluginPrograms[i].Name);
        free((char *)x->instances[instance].pluginPrograms);
        x->instances[instance].pluginPrograms = NULL;
        x->instances[instance].plugin_ProgramCount = 0;
    }

    x->instances[instance].pendingBankLSB = -1;
    x->instances[instance].pendingBankMSB = -1;
    x->instances[instance].pendingProgramChange = -1;

    if (x->descriptor->get_program &&
            x->descriptor->select_program) {

        /* Count the plugins first */
        /*FIX ?? */
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
                ph_debug_post("program %d is MIDI bank %lu program %lu, named '%s'",i,
                        x->instances[instance].pluginPrograms[i].Bank,
                        x->instances[instance].pluginPrograms[i].Program,
                        x->instances[instance].pluginPrograms[i].Name);

            }
        }
        /* No - it should be 0 anyway - ph_init */
        /*	else
                x->instances[instance].plugin_ProgramCount = 0;
                */	}
}

static LADSPA_Data get_port_default(ph_tilde *x, int port)
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

static unsigned ph_tilde_get_parm_number (ph_tilde *x,
        const char *str)
/* find out if str points to a parameter number or not and return the
   number or zero.  The number string has to begin with a '#' character */
{
    long num = 0;
    char* strend = NULL;

    if (str == NULL) {
        return 0;
    }
    if (str[0] != '#') {
        return 0;
    }
    num = strtol (&str[1], &strend, 10);
    if (str[1] == 0 || *strend != 0) {
        /* invalid string */
        return 0;
    }
    else if (num >= 1 && num <= (long)x->plugin_controlIns) {
        /* string ok and within range */
        return (unsigned)num;
    }
    else {
        /* number out of range */
        return 0;
    }
}

static void ph_tilde_set_control_input_by_index (ph_tilde *x,
        unsigned int ctrl_input_index,
        float value,
        t_int instance)
{
    long port, portno;

    if (ctrl_input_index >= x->plugin_controlIns) {
        post("pluginhost~: control port number %d is out of range [1, %d]",
                ctrl_input_index + 1, x->plugin_controlIns);
        return;
    }

    ph_debug_post("ctrl input number = %d", ctrl_input_index);



    port = x->plugin_ControlInPortNumbers[ctrl_input_index];


    /* FIX - temporary hack */
    if(x->is_DSSI)
        portno = 
            x->instances[instance].plugin_PortControlInNumbers[ctrl_input_index + 1];
    else
        portno = 
            x->instances[instance].plugin_PortControlInNumbers[ctrl_input_index];
    ph_debug_post("Global ctrl input number = %d", ctrl_input_index);
    post("Global ctrl input value = %.2f", value);

    /* set the appropriate control port value */
    x->plugin_ControlDataInput[portno] = value;

    /* Update the UI if there is one */
    if(x->is_DSSI){
        /* FIX:OSC */
        /*if(x->instances[instance].uiTarget == NULL){
            ph_debug_post("pluginhost~: unable to send to NULL target");

            return;
        }
        if(x->instances[instance].ui_osc_control_path == NULL){
            ph_debug_post("pluginhost~: unable to send to NULL control path");

            return;
        }*/
        /* FIX:OSC */
        /* lo_send(x->instances[instance].uiTarget, 
                x->instances[instance].ui_osc_control_path, "if", port, value);
                */
    }


}

static void ph_tilde_set_control_input_by_name (ph_tilde *x,
        const char* name,
        float value,
        t_int instance)
{
    unsigned port_index = 0;
    unsigned ctrl_input_index = 0;
    int found_port = 0; /* boolean */

    if (name == NULL || strlen (name) == 0) {
        post("pluginhost~: no control port name specified");
        return;
    }

    /* compare control name to LADSPA control input ports' names
       case-insensitively */
    found_port = 0;
    ctrl_input_index = 0;
    for (port_index = 0; port_index < x->descriptor->LADSPA_Plugin->PortCount; port_index++)
    {
        LADSPA_PortDescriptor port_type;
        port_type = x->descriptor->LADSPA_Plugin->PortDescriptors[port_index];
        if (LADSPA_IS_PORT_CONTROL (port_type)
                && LADSPA_IS_PORT_INPUT (port_type))
        {
            const char* port_name = NULL;
            unsigned cmp_length = 0;
            port_name = x->descriptor->LADSPA_Plugin->PortNames[port_index];
            cmp_length = MIN (strlen (name), strlen (port_name));
            if (cmp_length != 0
                    && strncasecmp (name, port_name, cmp_length) == 0)
            {
                /* found the first port to match */
                found_port = 1;
                break;
            }
            ctrl_input_index++;
        }
    }

    if (!found_port)
    {
        post("pluginhost~: plugin doesn't have a control input port named \"%s\"",
                name);
        return;
    }

    ph_tilde_set_control_input_by_index (x, ctrl_input_index, value, instance);
}

static void ph_tilde_control (ph_tilde *x, t_symbol* ctrl_name, 
        t_float ctrl_value,
        t_float instance_f)
/* Change the value of a named control port of the plug-in */
{
    unsigned parm_num = 0;
    int instance = (unsigned int)instance_f - 1;
    unsigned int n_instances = x->n_instances;

    if (instance > (int)x->n_instances || instance < -1){
        post("pluginhost~: control: invalid instance number %d", instance);
        return;
    }

    ph_debug_post("Received LADSPA control data for instance %d", instance);

    if (ctrl_name->s_name == NULL || strlen (ctrl_name->s_name) == 0) {
        post("pluginhost~: control messages must have a name and a value");
        return;
    }
    parm_num = ph_tilde_get_parm_number (x, ctrl_name->s_name);
    if (parm_num) {
        if(instance >= 0)
            ph_tilde_set_control_input_by_index (x, parm_num - 1, 
                    ctrl_value, instance);
        else if (instance == -1){
            while(n_instances--)
                ph_tilde_set_control_input_by_index (x, parm_num - 1, 
                        ctrl_value, n_instances);
        }
    }
    else {
        if(instance >= 0)
            ph_tilde_set_control_input_by_name (x, ctrl_name->s_name, 
                    ctrl_value, instance);
        else if (instance == -1){
            while(n_instances--)
                ph_tilde_set_control_input_by_name (x, 
                        ctrl_name->s_name, ctrl_value, n_instances);
        }
    }
}

static void ph_tilde_info (ph_tilde *x)
{
    unsigned int i, 
                 ctrl_portno, 
                 audio_portno;
    t_atom argv[7];

    ctrl_portno = audio_portno = 0;

    if (x->descriptor == NULL)
        return;

    for(i = 0; i < x->descriptor->LADSPA_Plugin->PortCount; i++){
        memcpy(&argv[0], &x->port_info[i].type, 
                sizeof(t_atom));
        memcpy(&argv[1], &x->port_info[i].data_type, 
                sizeof(t_atom));
        memcpy(&argv[3], &x->port_info[i].name, 
                sizeof(t_atom));
        memcpy(&argv[4], &x->port_info[i].lower_bound, 
                sizeof(t_atom));
        memcpy(&argv[5], &x->port_info[i].upper_bound, 
                sizeof(t_atom));
        memcpy(&argv[6], &x->port_info[i].p_default, 
                sizeof(t_atom));
        argv[2].a_type = A_FLOAT;
        if(!strcmp(argv[1].a_w.w_symbol->s_name, "control"))
            argv[2].a_w.w_float = (t_float)++ctrl_portno;

        else if(!strcmp(argv[1].a_w.w_symbol->s_name, "audio"))
            argv[2].a_w.w_float = (t_float)++audio_portno;

        outlet_anything (x->control_outlet, gensym ("port"), 7, argv);
    }
}

static void ph_tilde_ladspa_description(ph_tilde *x, t_atom *at, 
        DSSI_Descriptor *psDescriptor){
    at[0].a_w.w_symbol = 
        gensym ((char*)psDescriptor->LADSPA_Plugin->Name); 
    outlet_anything (x->control_outlet, gensym ("name"), 1, at);
    at[0].a_w.w_symbol = 
        gensym ((char*)psDescriptor->LADSPA_Plugin->Label); 
    outlet_anything (x->control_outlet, gensym ("label"), 1, at);
    at[0].a_type = A_FLOAT;
    at[0].a_w.w_float = psDescriptor->LADSPA_Plugin->UniqueID; 
    outlet_anything (x->control_outlet, gensym ("id"), 1, at);
    at[0].a_type = A_SYMBOL;
    at[0].a_w.w_symbol =
        gensym ((char*)psDescriptor->LADSPA_Plugin->Maker);
    outlet_anything (x->control_outlet, gensym ("maker"), 1, at);
}	

static void ph_tilde_ladspa_describe(const char * pcFullFilename, 
        void * pvPluginHandle,
        DSSI_Descriptor_Function fDescriptorFunction, 
        void* user_data,
        int is_DSSI) {

    ph_tilde *x = (((void**)user_data)[0]);
    t_atom at[1];
    DSSI_Descriptor *psDescriptor;
    long lIndex;

    at[0].a_type = A_SYMBOL;
    at[0].a_w.w_symbol = gensym ((char*)pcFullFilename); 
    outlet_anything (x->control_outlet, gensym ("library"), 1, at);

    if(is_DSSI){
        ph_debug_post("DSSI plugin found by listinfo");

        for (lIndex = 0;
                (psDescriptor = (DSSI_Descriptor *)
                 fDescriptorFunction(lIndex)) != NULL; lIndex++) 
            ph_tilde_ladspa_description(x, &at[0], psDescriptor);
    }

    else if(!is_DSSI)
        lIndex = 0;
    do{
        psDescriptor = ladspa_to_dssi((LADSPA_Descriptor *)fDescriptorFunction(lIndex++));
        if(psDescriptor->LADSPA_Plugin != NULL){
            ph_tilde_ladspa_description(x, &at[0], psDescriptor);
            free((DSSI_Descriptor *)psDescriptor);
        }
        else
            break;
    } while(1);
}

static void ph_tilde_list_plugins (ph_tilde *x)
{
    void* user_data[1];
    user_data[0] = x;
    LADSPAPluginSearch(ph_tilde_ladspa_describe,(void*)user_data);
}

/* FIX:OSC */
#if 0
static int osc_debug_handler(const char *path, const char *types, lo_arg **argv,
        int argc, void *data, ph_tilde *x)
{
    int i;
    printf("got unhandled OSC message:\npath: <%s>\n", path);
    for (i=0; i<argc; i++) {
        printf("arg %d '%c' ", i, types[i]);
        /* FIX:OSC */
        //lo_arg_pp(types[i], argv[i]);
        printf("\n");
    }
    return 1;
}
#endif

static void ph_tilde_get_current_program(ph_tilde *x, int instance)
{
    int i;
    t_atom argv[3];

    argv[0].a_type = A_FLOAT;
    argv[1].a_type = A_FLOAT;
    argv[2].a_type = A_SYMBOL;
    i = x->instances[instance].currentProgram;

    argv[0].a_w.w_float = (t_float)instance;
    argv[1].a_w.w_float = x->instances[instance].pluginPrograms[i].Program;
    argv[2].a_w.w_symbol = 
        gensym ((char*)x->instances[instance].pluginPrograms[i].Name); 
    outlet_anything (x->control_outlet, gensym ("program"), 3, argv);

}

static void ph_tilde_program_change(ph_tilde *x, int instance)
{
    /* jack-dssi-host queues program changes by using  pending program change variables. In the audio callback, if a program change is received via MIDI it over writes the pending value (if any) set by the GUI. If unset, or processed the value will default back to -1. The following call to select_program is then made. I don't think it eventually needs to be done this way - i.e. do we need 'pending'? */ 
    ph_debug_post("executing program change");

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
            ph_debug_post("Updating GUI program");

            /* FIX - this is a hack to make text ui work*/
            /* FIX:OSC */
            /* if(x->instances[instance].uiTarget){
               lo_send(x->instances[instance].uiTarget, 
                        x->instances[instance].ui_osc_program_path, "ii", 
                        x->instances[instance].currentBank, 
                        x->instances[instance].currentProgram);
            } */

        }
        x->instances[instance].uiNeedsProgramUpdate = 0;
        x->instances[instance].pendingProgramChange = -1;
        x->instances[instance].pendingBankMSB = -1;
        x->instances[instance].pendingBankLSB = -1;
    }
    ph_tilde_get_current_program(x, instance);
}

/* FIX:OSC */
#if 0
static int osc_program_handler(ph_tilde *x, lo_arg **argv, int instance)
{
    unsigned long bank = argv[0]->i;
    unsigned long program = argv[1]->i;
    int i;
    int found = 0;

    ph_debug_post("osc_program_hander active!");

    post("%d programs", x->instances[instance].plugin_ProgramCount);


    for (i = 0; i < x->instances[instance].plugin_ProgramCount; ++i) {
        if (x->instances[instance].pluginPrograms[i].Bank == bank &&
                x->instances[instance].pluginPrograms[i].Program == program) {
            post("pluginhost~: OSC: setting bank %u, program %u, name %s\n",
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
    ph_debug_post("bank = %d, program = %d, BankMSB = %d BankLSB = %d", bank, program, x->instances[instance].pendingBankMSB, x->instances[instance].pendingBankLSB);

    ph_tilde_program_change(x, instance);

    return 0;
}

static int osc_control_handler(ph_tilde *x, lo_arg **argv, int instance)
{
    int port = argv[0]->i;
    LADSPA_Data value = argv[1]->f;

    x->plugin_ControlDataInput[x->instances[instance].plugin_PortControlInNumbers[port]] = value;
    ph_debug_post("OSC: port %d = %f", port, value);


    return 0;
}

static int osc_midi_handler(ph_tilde *x, lo_arg **argv, t_int instance)
{

    int ev_type = 0, chan = 0;
    ph_debug_post("OSC: got midi request for"
            "(%02x %02x %02x %02x)",
            argv[0]->m[0], argv[0]->m[1], argv[0]->m[2], argv[0]->m[3]);

    chan = instance;
    ph_debug_post("channel: %d", chan);


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

static int osc_configure_handler(ph_tilde *x, lo_arg **argv, int instance)
{
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;
    char *message;

    ph_debug_post("osc_configure_handler active!");


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

static int osc_exiting_handler(ph_tilde *x, lo_arg **argv, int instance)
{

    ph_debug_post("exiting handler called: Freeing ui_osc");

    if(x->instances[instance].uiTarget){
        lo_address_free(x->instances[instance].uiTarget);
        x->instances[instance].uiTarget = NULL;
    }
    free(x->instances[instance].ui_osc_control_path);
    free(x->instances[instance].ui_osc_configure_path);
    free(x->instances[instance].ui_osc_hide_path);
    free(x->instances[instance].ui_osc_program_path);
    free(x->instances[instance].ui_osc_show_path); 
    free(x->instances[instance].ui_osc_quit_path); 
    x->instances[instance].uiTarget = NULL;
    x->instances[instance].ui_osc_control_path = NULL;
    x->instances[instance].ui_osc_configure_path = NULL;
    x->instances[instance].ui_osc_hide_path = NULL;
    x->instances[instance].ui_osc_program_path = NULL;
    x->instances[instance].ui_osc_show_path = NULL;
    x->instances[instance].ui_osc_quit_path = NULL;

    x->instances[instance].ui_hidden = 1;

    return 0;
}

static int osc_update_handler(ph_tilde *x, lo_arg **argv, int instance)
{
    const char *url = (char *)&argv[0]->s;
    const char *path;
    t_int i;
    char *host, *port;
    ph_configure_pair *p;

    p = x->configure_buffer_head;

    ph_debug_post("OSC: got update request from <%s>, instance %d", url, instance);


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

    if (x->instances[instance].ui_osc_quit_path) 
        free(x->instances[instance].ui_osc_quit_path); 
    x->instances[instance].ui_osc_quit_path = (char *)malloc(strlen(path) + 10);
    sprintf(x->instances[instance].ui_osc_quit_path, "%s/quit", path);

    if (x->instances[instance].ui_osc_show_path) 
        free(x->instances[instance].ui_osc_show_path); 
    x->instances[instance].ui_osc_show_path = (char *)malloc(strlen(path) + 10);
    sprintf(x->instances[instance].ui_osc_show_path, "%s/show", path);

    if (x->instances[instance].ui_osc_hide_path) 
        free(x->instances[instance].ui_osc_hide_path); 
    x->instances[instance].ui_osc_hide_path = (char *)malloc(strlen(path) + 10);
    sprintf(x->instances[instance].ui_osc_hide_path, "%s/hide", path);

    free((char *)path);

    while(p){
        if(p->instance == instance)
            ph_tilde_send_configure(x, p->key, 
                    p->value, instance);
        p = p->next;
    }

    /* Send current bank/program  (-FIX- another race...) */
    if (x->instances[instance].pendingProgramChange >= 0)
        ph_tilde_program_change(x, instance);
    ph_debug_post("pendingProgramChange = %d", x->instances[instance].pendingProgramChange);

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
        ph_debug_post("Port: %d, Default value: %.2f", x->plugin_ControlInPortNumbers[i], x->plugin_ControlDataInput[i]);

    }

    /* Send 'show' */
    if (x->instances[instance].ui_show) {
        lo_send(x->instances[instance].uiTarget, x->instances[instance].ui_osc_show_path, "");
        x->instances[instance].ui_hidden = 0;
        x->instances[instance].ui_show = 0;
    }

    return 0;
}
#endif

static void ph_tilde_osc_setup(ph_tilde *x, int instance)
{

#if 0
    if(instance == 0){
        x->osc_thread = lo_server_thread_new(NULL, osc_error);
        char *osc_url_tmp;
        osc_url_tmp = lo_server_thread_get_url(x->osc_thread);
        ph_debug_post("string length of osc_url_tmp:%d", strlen(osc_url_tmp));

        x->osc_url_base = (char *)malloc(sizeof(char) 
                * (strlen(osc_url_tmp) + strlen("dssi") + 1)); 
        sprintf(x->osc_url_base, "%s%s", osc_url_tmp, "dssi");
        free(osc_url_tmp);
        lo_server_thread_add_method(x->osc_thread, NULL, NULL, 
                osc_message_handler, x);
        lo_server_thread_start(x->osc_thread);
    }
    x->instances[instance].osc_url_path = (char *)malloc(sizeof(char) * 
            (strlen(x->plugin_basename) + strlen(x->descriptor->LADSPA_Plugin->Label) + 			strlen("chan00") + 3));
    sprintf(x->instances[instance].osc_url_path, "%s/%s/chan%02d", x->plugin_basename, 
            x->descriptor->LADSPA_Plugin->Label, instance); 
    ph_debug_post("OSC Path is: %s", x->instances[instance].osc_url_path);
    post("OSC thread started: %s", x->osc_url_base);
#endif

}

static void ph_tilde_init_programs(ph_tilde *x, int instance)
{

    ph_debug_post("Setting up program data");

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

/* FIX:OSC */
#if 0
static void ph_tilde_load_gui(ph_tilde *x, int instance)
{
    t_int err = 0;
    char *gui_path;
    struct dirent *dir_entry = NULL;
    char *gui_base;
    size_t baselen;
    DIR *dp;
    char *gui_str;

    gui_base = (char *)malloc((baselen = sizeof(char) * (strlen(x->plugin_full_path) - strlen(".so"))) + 1);

    strncpy(gui_base, x->plugin_full_path, baselen);
    gui_base[baselen] = '\0';

    /* don't use strndup - GNU only */
    /*	gui_base = strndup(x->plugin_full_path, baselen);*/
    ph_debug_post("gui_base: %s", gui_base);


    gui_str = (char *)malloc(sizeof(char) * (strlen("channel 00") + 1));
    sprintf (gui_str,"channel %02d", instance);

    ph_debug_post("GUI name string, %s", gui_str);


    if(!(dp = opendir(gui_base))){
        post("pluginhost~: unable to find GUI in %s, continuing without...", gui_base);
        return;
    }
    else {
        while((dir_entry = readdir(dp))){
            if (dir_entry->d_name[0] == '.') continue;
            if (strchr(dir_entry->d_name, '_')){
                if (strstr(dir_entry->d_name, "gtk") ||
                        strstr(dir_entry->d_name, "qt") || 
                        strstr(dir_entry->d_name, "text"))
                    break;
            }
        }
        ph_debug_post("GUI filename: %s", dir_entry->d_name);

    }

    gui_path = (char *)malloc(sizeof(char) * (strlen(gui_base) + strlen("/") + 
                strlen(dir_entry->d_name) + 1));

    sprintf(gui_path, "%s/%s", gui_base, dir_entry->d_name);

    free(gui_base);
    ph_debug_post("gui_path: %s", gui_path);


    osc_url = (char *)malloc
        (sizeof(char) * (strlen(x->osc_url_base) + 
                         strlen(x->instances[instance].osc_url_path) + 2));

    sprintf(osc_url, "%s/%s", x->osc_url_base, 
            x->instances[instance].osc_url_path);
    post("pluginhost~: instance %d URL: %s",instance, osc_url);
    ph_debug_post("Trying to open GUI!");


    x->instances[instance].gui_pid = fork();
    if (x->instances[instance].gui_pid == 0){
        err = execlp(gui_path, gui_path, osc_url, dir_entry->d_name, 
                x->descriptor->LADSPA_Plugin->Label, gui_str, NULL);
        perror("exec failed");
        exit(1); /* terminates the process */ 
    }

    ph_debug_post("errorcode = %d", err);


    free(gui_path);
    free(osc_url);
    free(gui_str);
    if(dp){

        ph_debug_post("directory handle closed = %d", closedir(dp));

    }
}
#endif

static void MIDIbuf(int type, unsigned int chan, int param, int val,
        ph_tilde *x)
{

    if(chan > x->n_instances - 1){
        post("pluginhost~: note discarded: MIDI data is destined for a channel that doesn't exist");
        return;
    }

    t_int time_ref = x->time_ref;
    t_int mapped;

    mapped = x->channelMap[chan + 1] - 1;

    x->midiEventBuf[x->bufWriteIndex].time.time.tv_sec = 
        (t_int)(clock_gettimesince(time_ref) * .001); 
    x->midiEventBuf[x->bufWriteIndex].time.time.tv_nsec = 
        (t_int)(clock_gettimesince(time_ref) * 1000); /*actually usec - we can't store this in nsec! */

    if ((type == SND_SEQ_EVENT_NOTEON && val != 0) || 
            type != SND_SEQ_EVENT_NOTEON) {
        x->midiEventBuf[x->bufWriteIndex].type = type;
        switch (type) {
            case SND_SEQ_EVENT_NOTEON:
                x->midiEventBuf[x->bufWriteIndex].data.note.channel = mapped;
                x->midiEventBuf[x->bufWriteIndex].data.note.note = param;
                x->midiEventBuf[x->bufWriteIndex].data.note.velocity = val;
                break;
            case SND_SEQ_EVENT_NOTEOFF:
                x->midiEventBuf[x->bufWriteIndex].data.note.channel = mapped;
                x->midiEventBuf[x->bufWriteIndex].data.note.note = param;
                x->midiEventBuf[x->bufWriteIndex].data.note.velocity = val;
                break;
            case SND_SEQ_EVENT_CONTROLLER:
                x->midiEventBuf[x->bufWriteIndex].data.control.channel = mapped;
                x->midiEventBuf[x->bufWriteIndex].data.control.param = param;
                x->midiEventBuf[x->bufWriteIndex].data.control.value = val;
                break;
            case SND_SEQ_EVENT_PITCHBEND:
                x->midiEventBuf[x->bufWriteIndex].data.control.channel = mapped;
                x->midiEventBuf[x->bufWriteIndex].data.control.param = 0;
                x->midiEventBuf[x->bufWriteIndex].data.control.value = val;
                break;
            case SND_SEQ_EVENT_CHANPRESS:
                x->midiEventBuf[x->bufWriteIndex].data.control.channel = mapped;
                x->midiEventBuf[x->bufWriteIndex].data.control.param = 0;
                x->midiEventBuf[x->bufWriteIndex].data.control.value = val;
                break;
            case SND_SEQ_EVENT_KEYPRESS:
                x->midiEventBuf[x->bufWriteIndex].data.note.channel = mapped;
                x->midiEventBuf[x->bufWriteIndex].data.note.note = param;
                x->midiEventBuf[x->bufWriteIndex].data.note.velocity = val;
                break;
            case SND_SEQ_EVENT_PGMCHANGE:
                x->instances[mapped].pendingBankMSB = (param - 1) / 128;
                x->instances[mapped].pendingBankLSB = (param - 1) % 128;
                x->instances[mapped].pendingProgramChange = val;
                x->instances[mapped].uiNeedsProgramUpdate = 1; 
                ph_debug_post("pgm chabge received in buffer: MSB: %d, LSB %d, prog: %d",
                        x->instances[mapped].pendingBankMSB, x->instances[mapped].pendingBankLSB, val);

                ph_tilde_program_change(x, mapped);
                break;
        }
    }
    else if (type == SND_SEQ_EVENT_NOTEON && val == 0) {
        x->midiEventBuf[x->bufWriteIndex].type = SND_SEQ_EVENT_NOTEOFF;
        x->midiEventBuf[x->bufWriteIndex].data.note.channel = mapped;
        x->midiEventBuf[x->bufWriteIndex].data.note.note = param;
        x->midiEventBuf[x->bufWriteIndex].data.note.velocity = val;
    }

    ph_debug_post("MIDI received in buffer: chan %d, param %d, val %d, mapped to %d",
            chan, param, val, mapped);

    x->bufWriteIndex = (x->bufWriteIndex + 1) % EVENT_BUFSIZE;
}

static void ph_tilde_list(ph_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    char *msg_type;
    int ev_type = 0;
    msg_type = (char *)malloc(TYPE_STRING_SIZE);
    atom_string(argv, msg_type, TYPE_STRING_SIZE);
    int chan = (int)atom_getfloatarg(1, argc, argv) - 1;
    int param = (int)atom_getfloatarg(2, argc, argv);
    int val = (int)atom_getfloatarg(3, argc, argv);
    int n_instances = x->n_instances;

    switch (msg_type[0]){
        case ASCII_n: ev_type = SND_SEQ_EVENT_NOTEON;
                      break;
        case ASCII_c: ev_type = SND_SEQ_EVENT_CONTROLLER;
                      break;
        case ASCII_p: ev_type = SND_SEQ_EVENT_PGMCHANGE;
                      break;
        case ASCII_b: ev_type = SND_SEQ_EVENT_PITCHBEND;
                      break;
        case ASCII_t: ev_type = SND_SEQ_EVENT_CHANPRESS;
                      break;
        case ASCII_a: ev_type = SND_SEQ_EVENT_KEYPRESS;
                      break;
    }
    ph_debug_post("initial midi NOTE:, arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d",ev_type,chan,param,val);

    if(ev_type != 0){
        if(chan >= 0)
            MIDIbuf(ev_type, chan, param, val, x);
        else {
            while(n_instances--)
                MIDIbuf(ev_type, n_instances, param, val, x);
        }
    }
    free(msg_type);
}

static char *ph_tilde_send_configure(ph_tilde *x, char *key, 
        char *value, int instance){

    char *debug;

    debug =   x->descriptor->configure(
            x->instanceHandles[instance], 
            key, value);
    /* FIX:OSC */
    /* if(x->instances[instance].uiTarget != NULL && x->is_DSSI) {
            lo_send(x->instances[instance].uiTarget, 
                x->instances[instance].ui_osc_configure_path,
                "ss", key, value);
        }
                */
    query_programs(x, instance);

    return debug;
}

static void ph_show(ph_tilde *x, t_int instance, t_int toggle)
{
            /* FIX:OSC */
/*
    if(x->instances[instance].uiTarget){
        if (x->instances[instance].ui_hidden && toggle) {
             lo_send(x->instances[instance].uiTarget, 
                    x->instances[instance].ui_osc_show_path, ""); 
            x->instances[instance].ui_hidden = 0;
        }
        else if (!x->instances[instance].ui_hidden && !toggle) {
                    x->instances[instance].ui_osc_hide_path, ""); 
            x->instances[instance].ui_hidden = 1;
        }
    }
    else if(toggle){
        x->instances[instance].ui_show = 1;
        ph_tilde_load_gui(x, instance);

    }
    */
}

static t_int ph_tilde_configure_buffer(ph_tilde *x, char *key, 
        char *value, t_int instance){

    ph_configure_pair *current, *p;
    t_int add_node;
    add_node = 0;	
    current = x->configure_buffer_head;

    while(current){
        if(!strcmp(current->key, key) && 
                current->instance == instance)
            break;
        current = current->next;
    }
    if(current)
        free(current->value);
    else {
        current = (ph_configure_pair *)malloc(sizeof
                (ph_configure_pair));
        current->next = x->configure_buffer_head;
        x->configure_buffer_head = current;
        current->key = strdup(key);
        current->instance = instance;
    }
    current->value = strdup(value);

    p = x->configure_buffer_head;

    /*FIX: eventually give ability to query this buffer (to outlet?) */
    while(p){
        ph_debug_post("key: %s", p->key);
        ph_debug_post("val: %s", p->value);
        ph_debug_post("instance: %d", p->instance);
        p = p->next;
    }

    return 0;
}

static t_int ph_tilde_configure_buffer_free(ph_tilde *x)
{
    ph_configure_pair *curr, *prev;
    prev = curr = NULL;

    for(curr = x->configure_buffer_head; curr != NULL; curr = curr->next){
        if(prev != NULL)
            free(prev);
        free(curr->key);
        free(curr->value);
        prev = curr;
    }
    free(curr);

    return 0;
}

static t_int ph_tilde_reset(ph_tilde *x, t_float instance_f)
{

    t_int instance = (t_int)instance_f - 1;
    if (instance == -1){
        for(instance = 0; instance < (int)x->n_instances; instance++) 			{
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
    return 0;
}

static void ph_tilde_search_plugin_callback (
        const char* full_filename,
        void* plugin_handle,
        DSSI_Descriptor_Function descriptor_function,
        void* user_data,
        int is_DSSI)
{
    DSSI_Descriptor* descriptor = NULL;
    unsigned plug_index = 0;

    char** out_lib_name = (char**)(((void**)user_data)[0]);
    char* name = (char*)(((void**)user_data)[1]);

    /* Stop searching when a first matching plugin is found */
    if (*out_lib_name == NULL)
    {
        ph_debug_post("pluginhost~: searching plugin \"%s\"...", full_filename);

        for(plug_index = 0;(is_DSSI ? 
                    (descriptor = 
                     (DSSI_Descriptor *)descriptor_function(plug_index)) : 
                    ((DSSI_Descriptor *)(descriptor = 
                        ladspa_to_dssi((LADSPA_Descriptor *)
                            descriptor_function(plug_index)))->LADSPA_Plugin)) 
                != NULL; plug_index++){
            ph_debug_post("pluginhost~: label \"%s\"", descriptor->LADSPA_Plugin->Label);

            if (strcasecmp (name, descriptor->LADSPA_Plugin->Label) 
                    == 0)
            {
                *out_lib_name = strdup (full_filename);
                ph_debug_post("pluginhost~: found plugin \"%s\" in library \"%s\"",
                        name, full_filename);

                /*	if(!is_DSSI){
                        free((DSSI_Descriptor *)descriptor);
                        descriptor = NULL;
                        }*/
                break;
            }
            /*	    if (descriptor != NULL){
                    free((DSSI_Descriptor *)descriptor);
                    descriptor = NULL;
                    }*/
        }
    }
}

static const char* plugin_tilde_search_plugin_by_label (ph_tilde *x,
        const char *name)
{
    char* lib_name = NULL;
    void* user_data[2];

    user_data[0] = (void*)(&lib_name);
    user_data[1] = (void*)name;
    ph_debug_post("search plugin by label: '%s'\n", name);


    lib_name = NULL;
    LADSPAPluginSearch (ph_tilde_search_plugin_callback,
            (void*)user_data);

    /* The callback (allocates and) writes lib_name, if it finds the plugin */
    return lib_name;

}

static t_int ph_tilde_dssi_methods(ph_tilde *x, t_symbol *s, int argc, t_atom *argv) 
{
    if (!x->is_DSSI) {
        post(
        "pluginhost~: plugin is not a DSSI plugin, operation not supported");
        return 0;
    }
    char *msg_type;
    char *debug;
    char *filename;
    char *filepath;
    char *key;
    char *value;
    char *temp;
    char mydir[MAXPDSTRING];
    int instance = -1;
    int pathlen;
    int toggle;
    int fd;
    int n_instances = x->n_instances;
    int count;
    int chan;
    int maxpatches;
    unsigned int i;
    t_float val;
    long filelength = 0;
    unsigned char *raw_patch_data = NULL;
    FILE *fp = NULL;
    size_t filename_length, key_size, value_size;
    dx7_patch_t *patchbuf, *firstpatch;
    msg_type = (char *)malloc(TYPE_STRING_SIZE);
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

        if(!strcmp(msg_type, "load") && x->descriptor->configure){
            filename = argv[1].a_w.w_symbol->s_name;
            post("pluginhost~: loading patch: %s for instance %d", filename, instance);

            if(!strcmp(x->descriptor->LADSPA_Plugin->Label, "hexter") || 
                    !strcmp(x->descriptor->LADSPA_Plugin->Label, "hexter6"))		{

                key = malloc(10 * sizeof(char)); /* holds "patchesN" */
                strcpy(key, "patches0");

                /* FIX: duplicates code from load_plugin() */
                fd = canvas_open(x->x_canvas, filename, "",
                        mydir, &filename, MAXPDSTRING, 0);

                if(fd >= 0){
                    filepath = mydir;
                    pathlen = strlen(mydir);
                    temp = &mydir[pathlen];
                    sprintf(temp, "/%s", filename);
                    fp = fopen(filepath, "rb");
                }
                else{
                    post("pluginhost~: unable to get file descriptor");
                }

                /*From dx7_voice_data by Sean Bolton */
                if(fp == NULL){
                    post("pluginhost~: unable to open patch file: %s", filename);
                    return 0;
                }
                if (fseek(fp, 0, SEEK_END) || 
                        (filelength = ftell(fp)) == -1 ||
                        fseek(fp, 0, SEEK_SET)) {
                    post("pluginhost~: couldn't get length of patch file: %s", 
                            filename);
                    fclose(fp);
                    return 0;
                }
                if (filelength == 0) {
                    post("pluginhost~: patch file has zero length");
                    fclose(fp);
                    return 0;
                } else if (filelength > 16384) {
                    post("pluginhost~: patch file is too large");
                    fclose(fp);
                    return 0;
                }
                if (!(raw_patch_data = (unsigned char *)
                            malloc(filelength))) 		     			 {
                    post(
                            "pluginhost~: couldn't allocate memory for raw patch file");
                    fclose(fp);
                    return 0;
                }
                if (fread(raw_patch_data, 1, filelength, fp) 
                        != (size_t)filelength) {
                    post("pluginhost~: short read on patch file: %s", filename);
                    free(raw_patch_data);
                    fclose(fp);
                    return 0;
                }
                fclose(fp);
                ph_debug_post("Patch file length is %ul", filelength);

                /* figure out what kind of file it is */
                filename_length = strlen(filename);
                if (filename_length > 4 &&
                        !strcmp(filename + filename_length - 4, ".dx7") &&
                        filelength % DX7_VOICE_SIZE_PACKED == 0) {  
                    /* It's a raw DX7 patch bank */

                    ph_debug_post("Raw DX7 format patch bank passed");

                    count = filelength / DX7_VOICE_SIZE_PACKED;
                    if (count > maxpatches)
                        count = maxpatches;
                    memcpy(firstpatch, raw_patch_data, count * 
                            DX7_VOICE_SIZE_PACKED);

                } else if (filelength > 6 &&
                        raw_patch_data[0] == 0xf0 &&
                        raw_patch_data[1] == 0x43 &&
                        /*This was used to fix some problem with Galaxy exports - possibly dump in worng format. It is not needed, but it did work, so in future, we may be able to support more formats not just DX7 */
                        /*   ((raw_patch_data[2] & 0xf0) == 0x00 || 
                             raw_patch_data[2] == 0x7e) &&*/
                        (raw_patch_data[2] & 0xf0) == 0x00 && 
                        raw_patch_data[3] == 0x09 &&
                        (raw_patch_data[4] == 0x10 || 
                         raw_patch_data[4] == 0x20) &&  
                        /* 0x10 is actual, 0x20 matches typo in manual */
                        raw_patch_data[5] == 0x00) {  
                    /* It's a DX7 sys-ex 32 voice dump */

                    ph_debug_post("SYSEX header check passed");


                    if (filelength != DX7_DUMP_SIZE_BULK ||
                            raw_patch_data[DX7_DUMP_SIZE_BULK - 1] != 0xf7) {
                        post("pluginhost~: badly formatted DX7 32 voice dump!");
                        count = 0;

#ifdef CHECKSUM_PATCH_FILES_ON_LOAD
                    } else if (dx7_bulk_dump_checksum(&raw_patch_data[6],
                                DX7_VOICE_SIZE_PACKED * 32) !=
                            raw_patch_data[DX7_DUMP_SIZE_BULK - 2]) {

                        post("pluginhost~: DX7 32 voice dump with bad checksum!");
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
                    post("pluginhost~: unknown patch bank file format!");
                    count = 0;

                }

                free(raw_patch_data);

                if(count == 32)
                    value = encode_7in6((uint8_t *)&patchbuf[0].data[0], 
                            count * DX7_VOICE_SIZE_PACKED);

            }
            else if(!strcmp(x->descriptor->LADSPA_Plugin->Label, 
                        "FluidSynth-DSSI")){
                key = malloc(6 * sizeof(char));
                strcpy(key, "load");
                value = filename;
            }
            else{
                post("pluginhost~: %s patches are not supported", 
                        x->descriptor->LADSPA_Plugin->Label);
            }

        }

        if(!strcmp(msg_type, "dir") && x->descriptor->configure){
            pathlen = strlen(argv[1].a_w.w_symbol->s_name) + 2;
            x->project_dir = malloc((pathlen) * sizeof(char));
            atom_string(&argv[1], x->project_dir, pathlen);
            post("pluginhost~: project directory for instance %d has been set to: %s", instance, x->project_dir);
            key = DSSI_PROJECT_DIRECTORY_KEY;
            value = x->project_dir;
        }

        else if(!strcmp(msg_type, "dir"))
            post("pluginhost~: %s %s: operation not supported", msg_type, 
                    argv[1].a_w.w_symbol->s_name);

        if(!strcmp(msg_type, "show") || !strcmp(msg_type, "hide")){
            instance = (int)atom_getfloatarg(1, argc, argv) - 1;
            if(!strcmp(msg_type, "show"))
                toggle = 1;
            else
                toggle = 0;

            if(instance == -1){
                while(n_instances--)
                    ph_show(x, n_instances, toggle);
            }
            else
                ph_show(x, instance, toggle);
        }

        if(!strcmp(msg_type, "remap")) {
            /* remap channel to instance */
            for(i = 0; i < x->n_instances && i < 128; i++){
                chan = (int)atom_getfloatarg(1 + i, argc, argv);
                post("pluginhost~: remapped MIDI channel %d to %d", 1+i, chan);
                x->channelMap[i+1] = chan;
            }
        }

    }

    /*Use this to send arbitrary configure message to plugin */
    else if(!strcmp(msg_type, "configure")){
        key = 
            (char *)malloc(key_size = (strlen(argv[1].a_w.w_symbol->s_name) + 2) * sizeof(char)); 
        atom_string(&argv[1], key, key_size);
        if(argc >= 3){	
            if (argv[2].a_type == A_FLOAT){
                val = atom_getfloatarg(2, argc, argv);
                value = (char *)malloc(TYPE_STRING_SIZE * 
                        sizeof(char));
                sprintf(value, "%.2f", val);
            }
            else if(argv[2].a_type == A_SYMBOL){
                value = 
                    (char *)malloc(value_size = 
                            (strlen(argv[2].a_w.w_symbol->s_name) + 2) * 
                            sizeof(char)); 
                atom_string(&argv[2], value, value_size);
            }		

        }	

        if(argc == 4 && argv[3].a_type == A_FLOAT)
            instance = atom_getfloatarg(3, argc, argv) - 1;
        else if (n_instances)
            instance = -1;
    }

    if(key != NULL && value != NULL){
        if(instance == -1){
            while(n_instances--){
                debug =	ph_tilde_send_configure(
                        x, key, value, n_instances);
                ph_tilde_configure_buffer(x, key, value, n_instances);
            }
        }
        /*FIX: Put some error checking in here to make sure instance is valid*/
        else{

            debug =	ph_tilde_send_configure(x, key, value, instance);
            ph_tilde_configure_buffer(x, key, value, instance);
        }
    }
    ph_debug_post("The plugin returned %s", debug);

    free(msg_type);
    free(patchbuf);

    return 0;
}

static void ph_tilde_bang(ph_tilde *x)
{
    t_atom at[3];

    at[0].a_type = A_FLOAT;
    at[1].a_type = A_SYMBOL;
    at[2].a_type = A_SYMBOL;

    if(x->plugin_label != NULL){
        at[0].a_w.w_float = x->n_instances;
        at[1].a_w.w_symbol = gensym ((char *)x->plugin_label); 
    }
    else{
        at[0].a_w.w_float = 0;
        at[1].a_w.w_symbol = gensym ("plugin"); 
    }	
    at[2].a_w.w_symbol = gensym ("instances"); 
    outlet_anything (x->control_outlet, gensym ("running"), 3, at);
}

static t_int *ph_tilde_perform(t_int *w)
{
    int N = (t_int)(w[2]);
    ph_tilde *x = (ph_tilde *)(w[1]);
    t_float **inputs = (t_float **)(&w[3]);
    t_float **outputs = (t_float **)(&w[3] + x->plugin_ins);
    unsigned int i;
    unsigned int instance;
    int n, timediff, framediff;
    /*See comment for ph_tilde_plug_plugin */
    if(x->dsp){
        x->dsp_loop = 1;

        for(i = 0; i < x->plugin_ins; i++)
            memcpy(x->plugin_InputBuffers[i], inputs[i], N * 
                    sizeof(LADSPA_Data));

        for (i = 0; i < x->n_instances; i++)
            x->instanceEventCounts[i] = 0;

        for (;x->bufReadIndex != x->bufWriteIndex; x->bufReadIndex = 
                (x->bufReadIndex + 1) % EVENT_BUFSIZE) {

            instance = x->midiEventBuf[x->bufReadIndex].data.note.channel;

            if(instance > x->n_instances){
                post(
            "pluginhost~: %s: discarding spurious MIDI data, for instance %d", 
                        x->descriptor->LADSPA_Plugin->Label, 
                        instance);
                ph_debug_post("n_instances = %d", x->n_instances);

                continue;
            }

            if (x->instanceEventCounts[instance] == EVENT_BUFSIZE){
                post("pluginhost~: MIDI overflow on channel %d", instance);
                continue;
            }

            timediff = (t_int)(clock_gettimesince(x->time_ref) * 1000) - 
                x->midiEventBuf[x->bufReadIndex].time.time.tv_nsec;
            framediff = (t_int)((t_float)timediff * .000001 / x->sr_inv); 

            if (framediff >= N || framediff < 0) 
                x->midiEventBuf[x->bufReadIndex].time.tick = 0;
            else
                x->midiEventBuf[x->bufReadIndex].time.tick = 
                    N - framediff - 1;

            x->instanceEventBuffers[instance]
                [x->instanceEventCounts[instance]] = 
                x->midiEventBuf[x->bufReadIndex];
            ph_debug_post("%s, note received on channel %d", 
                    x->descriptor->LADSPA_Plugin->Label, 
                    x->instanceEventBuffers[instance]
                    [x->instanceEventCounts[instance]].data.note.channel);

            x->instanceEventCounts[instance]++; 

            ph_debug_post("Instance event count for instance %d of %d: %d\n",
                    instance + 1, x->n_instances, x->instanceEventCounts[instance]);


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
            else if (x->instanceHandles[i] && 
                    x->descriptor->run_synth){
                x->descriptor->run_synth(x->instanceHandles[i], 
                        (unsigned long)N, x->instanceEventBuffers[i],
                        x->instanceEventCounts[i]); 
                i++;
            }
            else if (x->instanceHandles[i] && 
                    x->descriptor->LADSPA_Plugin->run){
                x->descriptor->LADSPA_Plugin->run
                    (x->instanceHandles[i], N);
                i++;
            }
        }


        for(i = 0; i < x->plugin_outs; i++)
            memcpy(outputs[i], (t_float *)x->plugin_OutputBuffers[i], N * 
                    sizeof(LADSPA_Data));

        /*
           for(i = 0; i < x->plugin_outs; i++)
           memcpy(x->outlets[i], (t_outlet *)x->plugin_OutputBuffers[i], N * 
           sizeof(LADSPA_Data));*/
        x->dsp_loop = 0;
    } 
    return w + (x->plugin_ins + x->plugin_outs + 3);
}

static void ph_tilde_dsp(ph_tilde *x, t_signal **sp)
{
    if(!x->n_instances){
        return;
    }


    t_int *dsp_vector, i, N, M;

    M = x->plugin_ins + x->plugin_outs + 2;

    dsp_vector = (t_int *) getbytes(M * sizeof(t_int));

    dsp_vector[0] = (t_int)x;
    dsp_vector[1] = (t_int)sp[0]->s_n;

    for(i = 2; i < M; i++)
        dsp_vector[i] = (t_int)sp[i - 1]->s_vec;

    dsp_addv(ph_tilde_perform, M, dsp_vector);

}

static void ph_tilde_quit_plugin(ph_tilde *x)
{

    int i;
    unsigned int instance;
    for(instance = 0; instance < x->n_instances; instance++) {
        /* FIX:OSC */
        /* if(x->instances[instance].uiTarget && x->is_DSSI){
             lo_send(x->instances[instance].uiTarget, 
                    x->instances[instance].ui_osc_quit_path, "");
            lo_address_free(x->instances[instance].uiTarget); 
            x->instances[instance].uiTarget = NULL; */
        
        ph_tilde_deactivate_plugin(x, (t_float)instance);
        if (x->descriptor->LADSPA_Plugin &&
                x->descriptor->LADSPA_Plugin->cleanup) {
            x->descriptor->LADSPA_Plugin->cleanup
                (x->instanceHandles[instance]);
        }
    }
}

static void ph_tilde_free_plugin(ph_tilde *x)
{

    int instance;
    unsigned int i;
    if(x->plugin_label != NULL)
        free((char *)x->plugin_label);
    if(x->plugin_handle != NULL){
        instance = x->n_instances;
        free((LADSPA_Handle)x->instanceHandles);
        free(x->plugin_ControlInPortNumbers); 
        free((t_float *)x->plugin_InputBuffers);
        free(x->instanceEventCounts);
        free(x->plugin_ControlDataInput);
        free(x->plugin_ControlDataOutput);

        while(instance--){

            /* FIX:OSC */
            /*
            if(x->instances[instance].gui_pid){
                ph_debug_post("Killing GUI process PID = %d", x->instances[instance].gui_pid);

                kill(x->instances[instance].gui_pid, SIGINT);
            } */
            if (x->instances[instance].pluginPrograms) {
                for (i = 0; i < 
                        x->instances[instance].plugin_ProgramCount; i++)
                    free((void *)
                            x->instances[instance].pluginPrograms[i].Name);
                free((char *)x->instances[instance].pluginPrograms);
                x->instances[instance].pluginPrograms = NULL;
                x->instances[instance].plugin_ProgramCount = 0;
            }
            free(x->instanceEventBuffers[instance]);
            if(x->is_DSSI){
                free(x->instances[instance].ui_osc_control_path);
                free(x->instances[instance].ui_osc_configure_path);
                free(x->instances[instance].ui_osc_program_path);
                free(x->instances[instance].ui_osc_show_path);
                free(x->instances[instance].ui_osc_hide_path);
                free(x->instances[instance].ui_osc_quit_path);
                free(x->instances[instance].osc_url_path);
            }
            free(x->instances[instance].plugin_PortControlInNumbers);
            if(x->plugin_outs)
                free(x->plugin_OutputBuffers[instance]);
        }
        if(x->is_DSSI){	
            if(x->project_dir != NULL)
                free(x->project_dir);
            free(x->osc_url_base);
            ph_tilde_configure_buffer_free(x);
        }
        free((snd_seq_event_t *)x->instanceEventBuffers);
        free(x->instances);
        free((t_float *)x->plugin_OutputBuffers);

        if(x->plugin_ins){
            for(i = 0; i < x->plugin_ins; i++)
                inlet_free((t_inlet *)x->inlets[i]);
            freebytes(x->inlets, x->plugin_ins * sizeof(t_inlet *));
        }

        if(x->plugin_outs){
            for(i = 0; i < x->plugin_outs; i++)
                outlet_free((t_outlet *)x->outlets[i]);
            freebytes(x->outlets, x->plugin_outs * sizeof(t_outlet *));
        }
        if(x->control_outlet)
            outlet_free(x->control_outlet);
        if(x->plugin_basename)
            free(x->plugin_basename);
        if(x->port_info)
            free(x->port_info);
    }
}

static void ph_tilde_init_plugin(ph_tilde *x)
{

    x->project_dir = NULL;
    x->configure_buffer_head = NULL;
    x->outlets = NULL;
    x->inlets = NULL;
    x->control_outlet = NULL;
    x->plugin_handle = NULL;
    x->plugin_full_path = NULL;
    x->plugin_label = NULL;
    x->plugin_basename = NULL;
    x->osc_url_base = NULL;
    x->plugin_ControlDataInput = x->plugin_ControlDataOutput = NULL;
    x->plugin_InputBuffers = x->plugin_OutputBuffers = NULL;
    x->plugin_ControlInPortNumbers = NULL;
    x->port_info = NULL;
    x->descriptor = NULL;
    x->instanceEventCounts = NULL;
    x->instances = NULL;
    x->instanceHandles = NULL;
    x->is_DSSI = 0;
    x->n_instances = 0;
    x->dsp = 0;
    x->dsp_loop = 0;
    x->plugin_ins = x->plugin_outs = 
        x->plugin_controlIns = x->plugin_controlOuts = 0;
    x->ports_in = x->ports_out = x->ports_controlIn = x->ports_controlOut = 0;
    x->bufWriteIndex = x->bufReadIndex = 0;

}

static void *ph_tilde_load_plugin(ph_tilde *x, t_int argc, t_atom *argv)
{
    char *plugin_basename = NULL,
         *plugin_full_path = NULL,
         *tmpstr,
         *plugin_label,
         plugin_dir[MAXPDSTRING];

    ph_debug_post("argc = %d", argc);

    unsigned int i;
    int stop;
    int fd;
    size_t pathlen;

    stop = 0;

    if (!argc){
        post("pluginhost~: no arguments given, please supply a path");
        return x;
    }

    char *argstr = strdup(argv[0].a_w.w_symbol->s_name);

    if(strstr(argstr, ":") != NULL){
        tmpstr = strtok(argstr, ":");
        plugin_full_path = strdup(tmpstr);
        plugin_label = strtok(NULL, ":");
        // first part of the string is empty, i.e. ':mystring'
        if (plugin_label == NULL) {
            x->plugin_label = plugin_full_path;
            plugin_full_path = NULL;
        } else {
            x->plugin_label = strdup(plugin_label);
        }
    } else { 
        x->plugin_label = strdup(argstr);
        tmpstr = (char *)plugin_tilde_search_plugin_by_label(x, x->plugin_label);
        if(tmpstr) {
            plugin_full_path = strdup(tmpstr);
        }
    }

    free(argstr);
    ph_debug_post("plugin path = %s", plugin_full_path);
    ph_debug_post("plugin name = %s", x->plugin_label);

    if(plugin_full_path == NULL){
        post("pluginhost~: can't get path to plugin");
        return x;
    }

    x->plugin_full_path = (char *)plugin_full_path;

    /* search for it in the 'canvas' path, which
     * includes the Pd search dirs and any 'extra' paths set with
     * [declare] */
    fd = canvas_open(x->x_canvas, plugin_full_path, "",
            plugin_dir, &plugin_basename, MAXPDSTRING, 0);

    if (fd >= 0) {
        ph_debug_post("plugin directory is %s, filename is %s", 
                plugin_dir, plugin_basename);

        x->plugin_basename = strdup(plugin_basename);
        pathlen = strlen(plugin_dir);
        tmpstr = &plugin_dir[pathlen];
        sprintf(tmpstr, "/%s", plugin_basename);
        tmpstr = plugin_dir;
        x->plugin_handle = loadLADSPAPluginLibrary(tmpstr);
    } else {
        /* try to load as is: this will work if plugin_full_path is an
         * absolute path, or the name of a library that is in DSSI_PATH
         * or LADSPA_PATH environment variables */
        x->plugin_handle = loadLADSPAPluginLibrary(plugin_full_path);
    }

    if (x->plugin_handle == NULL) {
        error("pluginhost~: can't find plugin in Pd paths, " 
                "try using [declare] to specify the path.");
        return x;
    }

    tmpstr = strdup(plugin_full_path);
    /* Don't bother working out the plugin name if we used canvas_open() 
     * to get the path */
    if(plugin_basename == NULL){
        if(!strstr(tmpstr, ".so")){
            post("pluginhost~: invalid plugin path, must end in .so");
            return x;
        }
        plugin_basename = strtok((char *)tmpstr, "/");
        while(strstr(plugin_basename, ".so") == NULL) {
            plugin_basename = strtok(NULL, "/");
        }
        x->plugin_basename = strdup(plugin_basename);
        ph_debug_post("plugin basename = %s", x->plugin_basename);
    }
    free(tmpstr);
    if(x->desc_func = (DSSI_Descriptor_Function)dlsym(x->plugin_handle,			"dssi_descriptor")){
        x->is_DSSI = true;
        x->descriptor = (DSSI_Descriptor *)x->desc_func(0);
    }
    else if(x->desc_func = 
            (DSSI_Descriptor_Function)dlsym(x->plugin_handle,						"ladspa_descriptor")){
        x->is_DSSI = false;
        x->descriptor = ladspa_to_dssi((LADSPA_Descriptor *)x->desc_func(0));
    }

    if(argc >= 2) {
        x->n_instances = (t_int)argv[1].a_w.w_float;
    } else {
        x->n_instances = 1;
    }

    ph_debug_post("n_instances = %d", x->n_instances);

    x->instances = (ph_instance *)malloc(sizeof(ph_instance) * 
            x->n_instances);

    if(!x->descriptor){
        post("pluginhost~: error: couldn't get plugin descriptor");
        return x;
    }

    ph_debug_post("%s loaded successfully!", 
            x->descriptor->LADSPA_Plugin->Label);

    x->port_info = (ph_port_info *)malloc
        (x->descriptor->LADSPA_Plugin->PortCount * 
         sizeof(ph_port_info));

    ph_tilde_port_info(x);
    ph_tilde_assign_ports(x);

    for(i = 0; i < x->n_instances; i++){
        x->instanceHandles[i] = 
            x->descriptor->LADSPA_Plugin->
            instantiate(x->descriptor->LADSPA_Plugin, x->sr);
        if (!x->instanceHandles[i]){
            post("pluginhost~: instantiation of instance %d failed", i);
            stop = 1;
            break;
        }
    }

    if(!stop){
        for(i = 0;i < x->n_instances; i++)
            ph_tilde_init_instance(x, i);
        for(i = 0;i < x->n_instances; i++)
            ph_tilde_connect_ports(x, i); 
        for(i = 0;i < x->n_instances; i++)
            ph_tilde_activate_plugin(x, i);

        if(x->is_DSSI){
            for(i = 0;i < x->n_instances; i++)
                ph_tilde_osc_setup(x, i);
#if LOADGUI
            for(i = 0;i < x->n_instances; i++)
                ph_tilde_load_gui(x, i);
#endif

            for(i = 0;i < x->n_instances; i++)
                ph_tilde_init_programs(x, i);

            for(i = 0; i < x->n_instances && i < 128; i++){
                x->channelMap[i] = i;
            }
        }
    }

    x->control_outlet =
        outlet_new (&x->x_obj, gensym("control"));

    if(x->plugin_outs){
        x->outlets = (t_outlet **)getbytes(x->plugin_outs * sizeof(t_outlet *)); 
        for(i = 0;i < x->plugin_outs; i++)
            x->outlets[i] = outlet_new(&x->x_obj, &s_signal);
    }
    else {
        post("pluginhost~: error: plugin has no outputs");
    }
    if(x->plugin_ins){
        x->inlets = (t_inlet **)getbytes(x->plugin_ins * sizeof(t_inlet *)); 
        for(i = 0;i < x->plugin_ins; i++) {
            x->inlets[i] = inlet_new(&x->x_obj, &x->x_obj.ob_pd, 
                    &s_signal, &s_signal);
        }
    }
    else {
        post("pluginhost~: error: plugin has no inputs");
    }

    x->dsp = true;
    post("pluginhost~: %d instances of %s, ready.", x->n_instances, 
            x->plugin_label);

    return (void *)x;
}


/* This method is currently buggy. PD's inlet/outlet handling seems buggy if you try to create ins/outs on the fly. Needs further investigation ...*/
static void ph_tilde_plug_plugin(ph_tilde *x, t_symbol *s, int argc, t_atom *argv)
{

    x->dsp = 0;
    ph_tilde_quit_plugin(x);
    while(1){
        if(!x->dsp_loop){
            ph_tilde_free_plugin(x);
            break;
        }
    }
    ph_tilde_init_plugin(x);
    ph_tilde_load_plugin(x, argc, argv);
}

static void *ph_tilde_new(t_symbol *s, t_int argc, t_atom *argv)
{

    ph_tilde *x = (ph_tilde *)pd_new(ph_tilde_class);
    post("\n========================================\npluginhost~: DSSI/LADSPA host - version %.2f\n========================================\n", VERSION);

    ph_tilde_init_plugin(x);

    x->sr       = (t_int)sys_getsr();
    x->sr_inv   = 1 / (t_float)x->sr;
    x->time_ref = (t_int)clock_getlogicaltime;
    x->blksize  = sys_getblksize();
    x->dsp      = 0;
    x->x_canvas = canvas_getcurrent();

    return ph_tilde_load_plugin(x, argc, argv);

}

static void ph_tilde_free(ph_tilde *x)
{

    ph_debug_post("Calling %s", __FUNCTION__);

    ph_tilde_quit_plugin(x);
    ph_tilde_free_plugin(x);

}

static void ph_tilde_sigchld_handler(int sig)
{
    wait(NULL);
}

void pluginhost_tilde_setup(void)
{

    ph_tilde_class = class_new(gensym("pluginhost~"), (t_newmethod)ph_tilde_new,
            (t_method)ph_tilde_free, sizeof(ph_tilde), 0, A_GIMME, 0);
    class_addlist(ph_tilde_class, ph_tilde_list);
    class_addbang(ph_tilde_class, ph_tilde_bang);
    class_addmethod(ph_tilde_class, 
            (t_method)ph_tilde_dsp, gensym("dsp"), 0);
    class_addmethod(ph_tilde_class, (t_method)ph_tilde_dssi_methods, 
            gensym("dssi"), A_GIMME, 0);
    class_addmethod (ph_tilde_class,(t_method)ph_tilde_control, 
            gensym ("control"),A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod (ph_tilde_class,(t_method)ph_tilde_info,
            gensym ("info"),0);
    class_addmethod (ph_tilde_class,(t_method)ph_tilde_list_plugins,
            gensym ("listplugins"),0);
    class_addmethod (ph_tilde_class,(t_method)ph_tilde_reset,
            gensym ("reset"), A_DEFFLOAT, 0);
    class_addmethod (ph_tilde_class,(t_method)ph_tilde_plug_plugin,
            gensym ("plug"),A_GIMME,0);
    /*    class_addmethod (ph_tilde_class,(t_method)ph_tilde_activate_plugin,
          gensym ("activate"),A_DEFFLOAT - 1,0);
          class_addmethod (ph_tilde_class,(t_method)ph_tilde_deactivate_plugin,
          gensym ("deactivate"),A_DEFFLOAT - 1,0);*/
    class_sethelpsymbol(ph_tilde_class, gensym("pluginhost~-help"));
    CLASS_MAINSIGNALIN(ph_tilde_class, ph_tilde, f);
    signal(SIGCHLD, ph_tilde_sigchld_handler);
}
/* FIX:OSC */
/*
static int osc_message_handler(const char *path, const char *types, 
        lo_arg **argv,int argc, void *data, void *user_data)
{
    ph_debug_post("osc_message_handler active");

    int i, instance = 0;
    const char *method;
    char chantemp[2];
    ph_tilde *x = (ph_tilde *)(user_data);

    if (strncmp(path, "/dssi/", 6)){
        ph_debug_post("calling osc_debug_handler"); 

        return osc_debug_handler(path, types, argv, argc, data, x);
    }
    for (i = 0; i < x->n_instances; i++) {
        if (!strncmp(path + 6, x->instances[i].osc_url_path,
                    strlen(x->instances[i].osc_url_path))) {
            instance = i;
            break;
        }
    }
    for(i = 0; i < argc; i++){
        ph_debug_post("got osc request %c from instance %d, path: %s", 
                types[i],instance,path);
    }


    if (!x->instances[instance].osc_url_path){
        ph_debug_post("calling osc_debug_handler"); 

        return osc_debug_handler(path, types, argv, argc, data, x);
    }
    method = path + 6 + strlen(x->instances[instance].osc_url_path);
    if (*method != '/' || *(method + 1) == 0){
        ph_debug_post("calling osc_debug_handler"); 

        return osc_debug_handler(path, types, argv, argc, data, x);
    }
    method++;

    if (!strcmp(method, "configure") && argc == 2 && !strcmp(types, "ss")) {

        ph_debug_post("calling osc_configure_handler");

        return osc_configure_handler(x, argv, instance);

    } else if (!strcmp(method, "control") && argc == 2 && !strcmp(types, "if")) {
        ph_debug_post("calling osc_control_handler");

        return osc_control_handler(x, argv, instance);
    }

    else if (!strcmp(method, "midi") && argc == 1 && !strcmp(types, "m")) {

        ph_debug_post("calling osc_midi_handler");

        return osc_midi_handler(x, argv, instance);

    } else if (!strcmp(method, "program") && argc == 2 && !strcmp(types, "ii")){
        ph_debug_post("calling osc_program_handler"); 

        return osc_program_handler(x, argv, instance);

    } else if (!strcmp(method, "update") && argc == 1 && !strcmp(types, "s")){
        ph_debug_post("calling osc_update_handler"); 

        return osc_update_handler(x, argv, instance);

    } else if (!strcmp(method, "exiting") && argc == 0) {

        return osc_exiting_handler(x, argv, instance);
    }

    return osc_debug_handler(path, types, argv, argc, data, x);
}
*/
static void ph_debug_post(const char *fmt, ...)
{
#if DEBUG
    va_list args;
    size_t fmt_length;
    unsigned int currpos;
    char newfmt[DEBUG_STRING_SIZE];
    char result[DEBUG_STRING_SIZE];

    fmt_length = strlen(fmt);

    sprintf(newfmt, "%s: ", MY_NAME);
    strncat(newfmt, fmt, fmt_length);
    currpos = strlen(MY_NAME) + 2 + fmt_length;
    newfmt[currpos] = '\0';

    va_start(args, fmt);
    vsprintf(result, newfmt, args);
    va_end(args);

    post(result);
#endif
}

