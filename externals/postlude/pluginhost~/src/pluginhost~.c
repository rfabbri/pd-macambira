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

static t_class *ph_class;

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
/* end hexter code */

/*
 * taken from liblo lo_url_get_path by Steve Harris et al
 */
char *osc_get_valid_path(const char *url)
{
    char *path = malloc(strlen(url));

    if (sscanf(url, "osc://%*[^:]:%*[0-9]%s", path)) {
        return path;
    }
    if (sscanf(url, "osc.%*[^:]://%*[^:]:%*[0-9]%s", path) == 1) {
        return path;
    }
    if (sscanf(url, "osc.unix://%*[^/]%s", path) == 1) {
        return path;
    }
    if (sscanf(url, "osc.%*[^:]://%s", path)) {
        return path;
    }

    /* doesnt look like an OSC URL with port number and path*/
    return NULL;
}
/* end liblo code */

static DSSI_Descriptor *ladspa_to_dssi(LADSPA_Descriptor *ladspaDesc)
{
    DSSI_Descriptor *dssiDesc;
    dssiDesc = (DSSI_Descriptor *)calloc(1, sizeof(DSSI_Descriptor));
    ((DSSI_Descriptor *)dssiDesc)->DSSI_API_Version = 1;
    ((DSSI_Descriptor *)dssiDesc)->LADSPA_Plugin = 
        (LADSPA_Descriptor *)ladspaDesc;
    return (DSSI_Descriptor *)dssiDesc;
}

static void ph_set_port_info(ph *x)
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
                ++x->plugin_control_ins;
            }
            else if (LADSPA_IS_PORT_OUTPUT(pod)){
                ++x->plugin_control_outs;
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
    ph_debug_post("%d inputs, %d outputs, %d control inputs, %d control outs", x->plugin_ins, x->plugin_outs, x->plugin_control_ins, x->plugin_control_outs);

}

static void ph_assign_ports(ph *x)
{
    unsigned int i;

    ph_debug_post("%d instances", x->n_instances);


    x->plugin_ins *= x->n_instances;
    x->plugin_outs *= x->n_instances;
    x->plugin_control_ins *= x->n_instances;
    x->plugin_control_outs *= x->n_instances;

    ph_debug_post("%d plugin outs", x->plugin_outs);


    x->plugin_input_buffers = 
        (float **)malloc(x->plugin_ins * sizeof(float *));
    x->plugin_output_buffers = 
        (float **)malloc(x->plugin_outs * sizeof(float *));
    x->plugin_control_input = 
        (float *)calloc(x->plugin_control_ins, sizeof(float));
    x->plugin_control_output = 
        (float *)calloc(x->plugin_control_outs, sizeof(float));
    for(i = 0; i < x->plugin_ins; i++)
        x->plugin_input_buffers[i] = 
            (float *)calloc(x->blksize, sizeof(float));
    for(i = 0; i < x->plugin_outs; i++)
        x->plugin_output_buffers[i] = 
            (float *)calloc(x->blksize, sizeof(float));
    x->instance_event_buffers = 
        (snd_seq_event_t **)malloc(x->n_instances * sizeof(snd_seq_event_t *));

    x->instance_handles = (LADSPA_Handle *)malloc(x->n_instances *
            sizeof(LADSPA_Handle));
    x->instance_event_counts = (unsigned long *)malloc(x->n_instances *
            sizeof(unsigned long));

    for(i = 0; i < x->n_instances; i++){
        x->instance_event_buffers[i] = (snd_seq_event_t *)malloc(EVENT_BUFSIZE *
                sizeof(snd_seq_event_t));

        x->instances[i].plugin_port_ctlin_numbers = 
            (int *)malloc(x->descriptor->LADSPA_Plugin->PortCount * 
                    sizeof(int));/* hmmm... as we don't support instances of differing plugin types, we probably don't need to do this dynamically*/
    }

    x->plugin_ctlin_port_numbers = 
        (unsigned long *)malloc(sizeof(unsigned long) * x->plugin_control_ins);

    ph_debug_post("Buffers assigned!");


}

static void ph_init_instance(ph *x, unsigned int i)
{

    ph_instance zero = {0};

    x->instances[i] = zero;
    /* ph_instance *instance = &x->instances[i];

    instance->current_bank           = 0;
    instance->current_pgm        = 0;
    instance->plugin_pgms        = NULL;
    instance->ui_osc_control_path   = NULL;
    instance->ui_osc_program_path   = NULL;
    instance->ui_osc_show_path      = NULL;
    instance->ui_osc_hide_path      = NULL;
    instance->ui_osc_quit_path      = NULL;
    instance->ui_osc_configure_path = NULL;
    instance->ui_needs_pgm_update  = 0;
    instance->pending_pgm_change  = -1;
    instance->plugin_pgm_count   = 0;
    instance->pending_bank_msb        = -1;
    instance->pending_bank_lsb        = -1;
    instance->ui_hidden             = 1;
    instance->ui_show               = 0;
*/
    ph_debug_post("Instance %d initialized!", i);

}

static void ph_connect_ports(ph *x, unsigned int i)
{

    unsigned int n;
    ph_instance *instance;

    instance = &x->instances[i];

    for(n = 0; n < x->descriptor->LADSPA_Plugin->PortCount; n++){
        ph_debug_post("PortCount: %d of %d", n, 
                x->descriptor->LADSPA_Plugin->PortCount);

        LADSPA_PortDescriptor pod =
            x->descriptor->LADSPA_Plugin->PortDescriptors[n];

        instance->plugin_port_ctlin_numbers[n] = -1;

        if (LADSPA_IS_PORT_AUDIO(pod)) {
            if (LADSPA_IS_PORT_INPUT(pod)) {
                x->descriptor->LADSPA_Plugin->connect_port
                    (x->instance_handles[i], n, 
                     x->plugin_input_buffers[x->ports_in++]);
            } 
            else if (LADSPA_IS_PORT_OUTPUT(pod)) {
                x->descriptor->LADSPA_Plugin->connect_port
                    (x->instance_handles[i], n, 
                     x->plugin_output_buffers[x->ports_out++]);
                ph_debug_post("Audio Input port %d connected", x->ports_in);
                post("Audio Output port %d connected", x->ports_out);

            }
        } 
        else if (LADSPA_IS_PORT_CONTROL(pod)) {
            if (LADSPA_IS_PORT_INPUT(pod)) {
                x->plugin_ctlin_port_numbers[x->ports_control_in] = (unsigned long) i;
                instance->plugin_port_ctlin_numbers[n] = x->ports_control_in;
                x->plugin_control_input[x->ports_control_in] = 
                    (t_float) get_port_default(x, n);
                ph_debug_post("default for port %d, control_in, %d is %.2f", n,
                        x->ports_control_in, 
                        x->plugin_control_input[x->ports_control_in]);


                x->descriptor->LADSPA_Plugin->connect_port
                    (x->instance_handles[i], n, 
                     &x->plugin_control_input[x->ports_control_in++]);

            } else if (LADSPA_IS_PORT_OUTPUT(pod)) {
                x->descriptor->LADSPA_Plugin->connect_port
                    (x->instance_handles[i], n, 
                     &x->plugin_control_output[x->ports_control_out++]);
            }
            ph_debug_post("Control Input port %d connected", x->ports_control_in);
            post("Control Output port %d connected", x->ports_control_out);

        }
    }

    ph_debug_post("ports connected!");

}

static void ph_activate_plugin(ph *x, unsigned int i)
{

    if(x->descriptor->LADSPA_Plugin->activate){
        ph_debug_post("trying to activate instance: %d", i);

        x->descriptor->LADSPA_Plugin->activate(x->instance_handles[i]);
    }
    ph_debug_post("plugin activated!");

}

static void ph_deactivate_plugin(ph *x, unsigned int instance)
{

    if(x->descriptor->LADSPA_Plugin->deactivate) {
        x->descriptor->LADSPA_Plugin->deactivate(x->instance_handles[instance]);
    }
    ph_debug_post("plugin deactivated!");

}

static void ph_cleanup_plugin(ph *x, unsigned int instance)
{
    if (x->descriptor->LADSPA_Plugin &&
            x->descriptor->LADSPA_Plugin->cleanup) {
        x->descriptor->LADSPA_Plugin->cleanup
            (x->instance_handles[instance]);
    }
}

/* TODO:OSC */
/*
static void osc_error(int num, const char *msg, const char *where)
{
    post("pluginhost~: osc error %d in path %s: %s\n",num, where, msg);
}
*/
static void query_programs(ph *x, unsigned int i)
{
    unsigned int n;
    ph_instance *instance = &x->instances[i];
    ph_debug_post("querying programs");

    /* free old lot */
    if (instance->plugin_pgms) {
        for (n = 0; n < instance->plugin_pgm_count; n++) {
            free((void *)instance->plugin_pgms[n].Name);
        }
        free(instance->plugin_pgms);
        instance->plugin_pgms = NULL;
        instance->plugin_pgm_count = 0;
    }

    instance->pending_bank_lsb = -1;
    instance->pending_bank_msb = -1;
    instance->pending_pgm_change = -1;

    if (x->descriptor->get_program &&
            x->descriptor->select_program) {

        /* Count the plugins first */
        /*TODO ?? */
        for (n = 0; x->descriptor->
                get_program(x->instance_handles[i], n); ++n);

        if (n > 0) {
            instance->plugin_pgm_count = n;
            instance->plugin_pgms = malloc(n * sizeof(DSSI_Program_Descriptor));
            while (n > 0) {
                const DSSI_Program_Descriptor *descriptor;
                --n;
                descriptor = x->descriptor->get_program(
                        x->instance_handles[i], n);
                instance->plugin_pgms[n].Bank = descriptor->Bank;
                instance->plugin_pgms[n].Program = descriptor->Program;
                instance->plugin_pgms[n].Name = strdup(descriptor->Name);
                ph_debug_post("program %d is MIDI bank %lu program %lu,"
                        " named '%s'",i,
                        instance->plugin_pgms[n].Bank,
                        instance->plugin_pgms[n].Program,
                        instance->plugin_pgms[n].Name);
            }
        } else {
            assert(instance->plugin_pgm_count == 0);
        }
    }
}

static LADSPA_Data get_port_default(ph *x, int port)
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

static unsigned ph_get_param_num (ph *x, const char *str)
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
    else if (num >= 1 && num <= (long)x->plugin_control_ins) {
        /* string ok and within range */
        return (unsigned)num;
    }
    else {
        /* number out of range */
        return 0;
    }
}

static void ph_set_control_input_by_index (ph *x,
        unsigned int ctrl_input_index, float value, unsigned int i)
{
    long port, portno;
    t_int argc = 3;
    t_atom argv[argc];
    ph_instance *instance;

    if (ctrl_input_index >= x->plugin_control_ins) {
        post("pluginhost~: control port number %d is out of range [1, %d]",
                ctrl_input_index + 1, x->plugin_control_ins);
        return;
    }

    ph_debug_post("ctrl input number = %d", ctrl_input_index);

    port = x->plugin_ctlin_port_numbers[ctrl_input_index];

    instance = &x->instances[i];

    /* TODO - temporary hack */
    if(x->is_dssi) {
        portno = instance->plugin_port_ctlin_numbers[ctrl_input_index + 1];
    } else {
        portno = instance->plugin_port_ctlin_numbers[ctrl_input_index];
    }

    ph_debug_post("Global ctrl input number = %d", ctrl_input_index);
    ph_debug_post("Global ctrl input value = %.2f", value);

    /* set the appropriate control port value */
    x->plugin_control_input[portno] = value;

    /* Update the UI if there is one */
    if(!x->is_dssi){
        return;
    }

    if(instance->ui_osc_control_path == NULL){
        ph_debug_post("pluginhost~: unable to send to NULL control path");
        return;
    }

    SETSYMBOL(argv, gensym(instance->ui_osc_control_path));
    SETFLOAT(argv+1, port);
    SETFLOAT(argv+2, value);
    ph_instance_send_osc(x->message_out, instance, argc, argv);

}

static void ph_set_control_input_by_name (ph *x,
        const char* name,
        float value,
        unsigned int i)
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
    for (port_index = 0; port_index < x->descriptor->LADSPA_Plugin->PortCount; 
            port_index++)
    {
        LADSPA_PortDescriptor port_type;
        port_type = x->descriptor->LADSPA_Plugin->PortDescriptors[port_index];
        if (LADSPA_IS_PORT_CONTROL (port_type)
                && LADSPA_IS_PORT_INPUT (port_type))
        {
            const char* port_name = NULL;
            unsigned cmp_length = 0;
            port_name = x->descriptor->LADSPA_Plugin->PortNames[port_index];
            cmp_length = MIN (strlen(name), strlen(port_name));
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
        post("pluginhost~: plugin doesn't have a control input port"
                " named \"%s\"", name);
        return;
    }

    ph_set_control_input_by_index (x, ctrl_input_index, value, i);

}

static void ph_control (ph *x, t_symbol* ctrl_name, t_float ctrl_value, 
        int instance)
/* Change the value of a named control port of the plug-in */
{
    unsigned param = 0;
    int i = instance - 1;
    unsigned int n = x->n_instances;

    if (i > (int)x->n_instances || i < -1){
        post("pluginhost~: control: invalid instance number %d", i);
        return;
    }

    ph_debug_post("Received LADSPA control data for instance %d", i);

    if (ctrl_name->s_name == NULL || strlen (ctrl_name->s_name) == 0) {
        post("pluginhost~: control messages must have a name and a value");
        return;
    }
    param = ph_get_param_num(x, ctrl_name->s_name);
    if (param) {
        if(i >= 0) {
            ph_set_control_input_by_index (x, param - 1, ctrl_value, i);
        } else if (i == -1) {
            while(n--) { 
                ph_set_control_input_by_index (x, param - 1, ctrl_value, n);
            }
        }
    } else if (i >= 0) {
        ph_set_control_input_by_name (x, ctrl_name->s_name, 
                ctrl_value, i);
    } else if (i == -1) {
        while(n--) {
            ph_set_control_input_by_name (x, ctrl_name->s_name, ctrl_value, n);
        }
    }
}

static void ph_info (ph *x)
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

        outlet_anything (x->message_out, gensym ("port"), 7, argv);
    }
}

static void ph_ladspa_description(ph *x, t_atom *at, 
        DSSI_Descriptor *psDescriptor){
    at[0].a_w.w_symbol = 
        gensym ((char*)psDescriptor->LADSPA_Plugin->Name); 
    outlet_anything (x->message_out, gensym ("name"), 1, at);
    at[0].a_w.w_symbol = 
        gensym ((char*)psDescriptor->LADSPA_Plugin->Label); 
    outlet_anything (x->message_out, gensym ("label"), 1, at);
    at[0].a_type = A_FLOAT;
    at[0].a_w.w_float = psDescriptor->LADSPA_Plugin->UniqueID; 
    outlet_anything (x->message_out, gensym ("id"), 1, at);
    at[0].a_type = A_SYMBOL;
    at[0].a_w.w_symbol =
        gensym ((char*)psDescriptor->LADSPA_Plugin->Maker);
    outlet_anything (x->message_out, gensym ("maker"), 1, at);
}	

static void ph_ladspa_describe(const char * pcFullFilename, 
        void * pvPluginHandle,
        DSSI_Descriptor_Function fDescriptorFunction, 
        void* user_data,
        int is_dssi) {

    ph *x = (((void**)user_data)[0]);
    t_atom at[1];
    DSSI_Descriptor *psDescriptor;
    long lIndex;

    at[0].a_type = A_SYMBOL;
    at[0].a_w.w_symbol = gensym ((char*)pcFullFilename); 
    outlet_anything (x->message_out, gensym ("library"), 1, at);

    if(is_dssi){
        ph_debug_post("DSSI plugin found by listinfo");

        for (lIndex = 0;
                (psDescriptor = (DSSI_Descriptor *)
                 fDescriptorFunction(lIndex)) != NULL; lIndex++) 
            ph_ladspa_description(x, &at[0], psDescriptor);
    }

    else if(!is_dssi)
        lIndex = 0;
    do{
        psDescriptor = ladspa_to_dssi((LADSPA_Descriptor *)fDescriptorFunction(lIndex++));
        if(psDescriptor->LADSPA_Plugin != NULL){
            ph_ladspa_description(x, &at[0], psDescriptor);
            free((DSSI_Descriptor *)psDescriptor);
        }
        else
            break;
    } while(1);
}

static void ph_list_plugins (ph *x)
{
    void* user_data[1];
    user_data[0] = x;
    LADSPAPluginSearch(ph_ladspa_describe,(void*)user_data);
}

static void ph_osc_debug_handler(const char *path)
{

    ph_debug_post("got unhandled OSC message:\npath: <%s>\n", path);

}

static void ph_get_current_pgm(ph *x, unsigned int i)
{
    t_int argc = 3;
    t_atom argv[argc];
    ph_instance *instance;
    unsigned int pgm;

    instance = &x->instances[i];
    pgm      = instance->current_pgm;

    SETFLOAT(argv, i);
    SETFLOAT(argv+1, instance->plugin_pgms[pgm].Program);
    SETSYMBOL(argv+2, gensym(instance->plugin_pgms[pgm].Name));
    outlet_anything(x->message_out, gensym ("program"), argc, argv);

}

static void ph_program_change(ph *x, unsigned int i)
{
    /* jack-dssi-host queues program changes by using  pending program change variables. In the audio callback, if a program change is received via MIDI it over writes the pending value (if any) set by the GUI. If unset, or processed the value will default back to -1. The following call to select_program is then made. I don't think it eventually needs to be done this way - i.e. do we need 'pending'? */ 
    ph_instance *instance;
    t_int argc = 3;
    t_atom argv[argc];

    instance = &x->instances[i];

    ph_debug_post("executing program change");

    if (instance->pending_pgm_change >= 0){
        if (instance->pending_bank_lsb >= 0) {
            if (instance->pending_bank_msb >= 0) {
                instance->current_bank =
                    instance->pending_bank_lsb + 128 * instance->pending_bank_msb;
            } else {
                instance->current_bank = instance->pending_bank_lsb + 
                    128 * (instance->current_bank / 128);
            }
        } else if (instance->pending_bank_msb >= 0) {
            instance->current_bank = 
                (instance->current_bank % 128) + 128 * instance->pending_bank_msb;
        }

        instance->current_pgm = instance->pending_pgm_change;

        if (x->descriptor->select_program) {
            x->descriptor->select_program(x->instance_handles[i],
                    instance->current_bank, instance->current_pgm);
        }
        if (instance->ui_needs_pgm_update){
            ph_debug_post("Updating GUI program");

            /* TODO - this is a hack to make text ui work*/
            if(x->is_dssi){
                SETSYMBOL(argv, gensym(instance->ui_osc_program_path));
                SETFLOAT(argv+1, instance->current_bank);
                SETFLOAT(argv+2, instance->current_pgm);
                ph_instance_send_osc(x->message_out, instance, argc, argv);
            }

        }
        instance->ui_needs_pgm_update = 0;
        instance->pending_pgm_change = -1;
        instance->pending_bank_msb = -1;
        instance->pending_bank_lsb = -1;
    }
    ph_get_current_pgm(x, i);
}

static void ph_osc_program_handler(ph *x, t_atom *argv, unsigned int i)
{
    unsigned long bank;
    unsigned long program; 
    unsigned int n;
    bool found;
    ph_instance *instance;

    bank     = atom_getfloat(&argv[0]);
    program  = atom_getfloat(&argv[1]);
    instance = &x->instances[i];
    found    = false;

    ph_debug_post("%d programs", instance->plugin_pgm_count);

    for (n = 0; n < instance->plugin_pgm_count; ++n) {
        if (instance->plugin_pgms[n].Bank == bank &&
                instance->plugin_pgms[n].Program == program) {
            ph_debug_post("OSC: setting bank %u, program %u, name %s\n",
                    bank, program, instance->plugin_pgms[n].Name);
            found = true;
            break;
        }
    }

    if (!found) {
        post("%s: OSC:  UI requested unknown program: bank %ul, program %ul: "
                "sending to plugin anyway (plugin should ignore it)\n", 
                MY_NAME, bank, program);
    }

    instance->pending_bank_msb   = bank / 128;
    instance->pending_bank_lsb   = bank % 128;
    instance->pending_pgm_change = program;

    ph_debug_post("bank = %d, program = %d, BankMSB = %d BankLSB = %d", 
            bank, program, instance->pending_bank_msb,
            instance->pending_bank_lsb);

    ph_program_change(x, i);

}

static void ph_osc_control_handler(ph *x, t_atom *argv, int i)
{
    int port;
    LADSPA_Data value; 
    ph_instance *instance;

    port     = (int)atom_getfloat(&argv[0]);
    value    = atom_getfloat(&argv[1]);
    instance = &x->instances[i];

    x->plugin_control_input[instance->plugin_port_ctlin_numbers[port]] = value;
    ph_debug_post("OSC: port %d = %f", port, value);

}

static void ph_osc_midi_handler(ph *x, t_atom *argv, unsigned int i)
{
    post("%s: warning: MIDI over OSC currently unsupported", MY_NAME);
}

static void ph_osc_configure_handler(ph *x, t_atom *argv, int i)
{
    const char *key;
    const char *value;
    char *message;

    key   = atom_getsymbol(&argv[0])->s_name;
    value = atom_getsymbol(&argv[1])->s_name;

    ph_debug_post("%s()", __FUNCTION__);

    if (!x->descriptor->configure) {
        return;
    } 

    if (!strncmp(key, DSSI_RESERVED_CONFIGURE_PREFIX,
                strlen(DSSI_RESERVED_CONFIGURE_PREFIX))) {
        post("%s: error: OSC: UI for plugin '' attempted to use reserved "
                "configure key \"%s\", ignoring", MY_NAME, key);
        return;
    }

    message = x->descriptor->configure(x->instance_handles[i], key, value);

    if (message) {
        post("%s: on configure  '%s', plugin '' returned error '%s'", MY_NAME,
                key, message);
        free(message);
    }

    query_programs(x, i);

}

static void ph_osc_exiting_handler(ph *x, t_atom *argv, int i)
{

    ph_instance *instance;

    instance = &x->instances[i];

    free(instance->ui_osc_control_path);
    free(instance->ui_osc_configure_path);
    free(instance->ui_osc_hide_path);
    free(instance->ui_osc_program_path);
    free(instance->ui_osc_show_path); 
    free(instance->ui_osc_quit_path); 
    instance->ui_osc_control_path   = NULL;
    instance->ui_osc_configure_path = NULL;
    instance->ui_osc_hide_path      = NULL;
    instance->ui_osc_program_path   = NULL;
    instance->ui_osc_show_path      = NULL;
    instance->ui_osc_quit_path      = NULL;
    instance->ui_hidden             = true;

}

static void ph_osc_update_handler(ph *x, t_atom *argv, int i)
{
    const char *url; 
    const char *path;
    unsigned int n;
    unsigned int ac = 3;
    t_atom av[ac];
    ph_configure_pair *p;
    ph_instance *instance;

    instance = &x->instances[i];
    url      = atom_getsymbol(&argv[0])->s_name;
    p        = x->configure_buffer_head;

    ph_debug_post("OSC: got update request from <%s>, instance %d",
            url, instance);

    path = osc_get_valid_path(url);

    if(path == NULL) {
        post("%s(): error: invalid url: %s", MY_NAME, url);
        return;
    }

    if (instance->ui_osc_control_path) {
        free(instance->ui_osc_control_path);
    }
    instance->ui_osc_control_path = malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_control_path, "%s/control", path);

    if (instance->ui_osc_configure_path) {
        free(instance->ui_osc_configure_path);
    }
    instance->ui_osc_configure_path = malloc(strlen(path) + 12);
    sprintf(instance->ui_osc_configure_path, "%s/configure", path);

    if (instance->ui_osc_program_path) {
        free(instance->ui_osc_program_path);
    }
    instance->ui_osc_program_path = malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_program_path, "%s/program", path);

    if (instance->ui_osc_quit_path) {
        free(instance->ui_osc_quit_path);
    }
    instance->ui_osc_quit_path = malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_quit_path, "%s/quit", path);

    if (instance->ui_osc_show_path) {
        free(instance->ui_osc_show_path);
    }
    instance->ui_osc_show_path = malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_show_path, "%s/show", path);

    if (instance->ui_osc_hide_path) {
        free(instance->ui_osc_hide_path);
    }
    instance->ui_osc_hide_path = (char *)malloc(strlen(path) + 10);
    sprintf(instance->ui_osc_hide_path, "%s/hide", path);

    free((char *)path);

    while(p){
        if(p->instance == i) {
            ph_send_configure(x, p->key, p->value, i);
        }
        p = p->next;
    }

    /* Send current bank/program */
    if (instance->pending_pgm_change >= 0) {
        ph_program_change(x, i);
    }

    ph_debug_post("pending_pgm_change = %d", instance->pending_pgm_change);

    if (instance->pending_pgm_change < 0) {
        unsigned long bank;
        unsigned long program;
        ac = 3;

        program = instance->current_pgm;
        bank    = instance->current_bank;
        instance->ui_needs_pgm_update = 0;

        SETSYMBOL(av, gensym(instance->ui_osc_program_path));
        SETFLOAT(av+1, bank);
        SETFLOAT(av+2, program);

        ph_instance_send_osc(x->message_out, instance, ac, av);

    }

    /* Send control ports */
    for (n = 0; n < x->plugin_control_ins; n++) {

        ac = 3;

        SETSYMBOL(av, gensym(instance->ui_osc_control_path));
        SETFLOAT(av+1, x->plugin_ctlin_port_numbers[n]);
        SETFLOAT(av+2, x->plugin_control_input[n]);

        ph_instance_send_osc(x->message_out, instance, ac, av);

        ph_debug_post("Port: %d, Default value: %.2f",
                x->plugin_ctlin_port_numbers[n], x->plugin_control_input[n]);

    }

    /* Send 'show' */
    if (instance->ui_show) {

        ac = 2;

        SETSYMBOL(av, gensym(instance->ui_osc_show_path));
        SETSYMBOL(av, gensym(""));

        ph_instance_send_osc(x->message_out, instance, ac, av);

        instance->ui_hidden = false;
        instance->ui_show = false;
    }
}

static void ph_osc_setup(ph *x, unsigned int i)
{
    ph_instance *instance = &x->instances[i];

    if(i == 0){
        x->osc_port = OSC_PORT;
    }
    instance->osc_url_path = malloc(sizeof(char) * 
            (strlen(x->plugin_basename) + 
             strlen(x->descriptor->LADSPA_Plugin->Label) + 
             strlen("chan00") + 3));
    sprintf(instance->osc_url_path, "%s/%s/chan%02d", x->plugin_basename, 
            x->descriptor->LADSPA_Plugin->Label, i); 
    ph_debug_post("OSC Path is: %s", instance->osc_url_path);

}

static void ph_init_programs(ph *x, unsigned int i)
{
    ph_instance *instance = &x->instances[i];
    ph_debug_post("Setting up program data");
    query_programs(x, i);

    if (x->descriptor->select_program && instance->plugin_pgm_count > 0) {

        /* select program at index 0 */
        unsigned long bank            = instance->plugin_pgms[0].Bank;
        instance->pending_bank_msb    = bank / 128;
        instance->pending_bank_lsb    = bank % 128;
        instance->pending_pgm_change  = instance->plugin_pgms[0].Program;
        instance->ui_needs_pgm_update = 1;
    }
}

/* TODO:OSC */
#if 0
static void ph_load_gui(ph *x, int instance)
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


    /* osc_url_base was of the form:
     * osc.udp://127.0.0.1:9997/dssi
     */
    osc_url = (char *)malloc
        (sizeof(char) * (strlen(x->osc_url_base) + 
                         strlen(instance->osc_url_path) + 2));

    sprintf(osc_url, "%s/%s", x->osc_url_base, 
            instance->osc_url_path);
    post("pluginhost~: instance %d URL: %s",instance, osc_url);
    ph_debug_post("Trying to open GUI!");


    instance->gui_pid = fork();
    if (instance->gui_pid == 0){
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

static void ph_midibuf_add(ph *x, int type, unsigned int chan, int param, int val)
{

    if(chan > x->n_instances - 1){
        post("pluginhost~: note discarded: MIDI data is destined for a channel that doesn't exist");
        return;
    }

    t_int time_ref = x->time_ref;
    t_int mapped;

    mapped = x->channel_map[chan + 1] - 1;

    x->midi_event_buf[x->buf_write_index].time.time.tv_sec = 
        (t_int)(clock_gettimesince(time_ref) * .001); 
    x->midi_event_buf[x->buf_write_index].time.time.tv_nsec = 
        (t_int)(clock_gettimesince(time_ref) * 1000); /*actually usec - we can't store this in nsec! */

    if ((type == SND_SEQ_EVENT_NOTEON && val != 0) || 
            type != SND_SEQ_EVENT_NOTEON) {
        x->midi_event_buf[x->buf_write_index].type = type;
        switch (type) {
            case SND_SEQ_EVENT_NOTEON:
                x->midi_event_buf[x->buf_write_index].data.note.channel = mapped;
                x->midi_event_buf[x->buf_write_index].data.note.note = param;
                x->midi_event_buf[x->buf_write_index].data.note.velocity = val;
                break;
            case SND_SEQ_EVENT_NOTEOFF:
                x->midi_event_buf[x->buf_write_index].data.note.channel = mapped;
                x->midi_event_buf[x->buf_write_index].data.note.note = param;
                x->midi_event_buf[x->buf_write_index].data.note.velocity = val;
                break;
            case SND_SEQ_EVENT_CONTROLLER:
                x->midi_event_buf[x->buf_write_index].data.control.channel = mapped;
                x->midi_event_buf[x->buf_write_index].data.control.param = param;
                x->midi_event_buf[x->buf_write_index].data.control.value = val;
                break;
            case SND_SEQ_EVENT_PITCHBEND:
                x->midi_event_buf[x->buf_write_index].data.control.channel = mapped;
                x->midi_event_buf[x->buf_write_index].data.control.param = 0;
                x->midi_event_buf[x->buf_write_index].data.control.value = val;
                break;
            case SND_SEQ_EVENT_CHANPRESS:
                x->midi_event_buf[x->buf_write_index].data.control.channel = mapped;
                x->midi_event_buf[x->buf_write_index].data.control.param = 0;
                x->midi_event_buf[x->buf_write_index].data.control.value = val;
                break;
            case SND_SEQ_EVENT_KEYPRESS:
                x->midi_event_buf[x->buf_write_index].data.note.channel = mapped;
                x->midi_event_buf[x->buf_write_index].data.note.note = param;
                x->midi_event_buf[x->buf_write_index].data.note.velocity = val;
                break;
            case SND_SEQ_EVENT_PGMCHANGE:
                x->instances[mapped].pending_bank_msb = (param - 1) / 128;
                x->instances[mapped].pending_bank_lsb = (param - 1) % 128;
                x->instances[mapped].pending_pgm_change = val;
                x->instances[mapped].ui_needs_pgm_update = 1; 
                ph_debug_post("pgm chabge received in buffer: MSB: %d, LSB %d, prog: %d",
                        x->instances[mapped].pending_bank_msb, x->instances[mapped].pending_bank_lsb, val);

                ph_program_change(x, mapped);
                break;
        }
    }
    else if (type == SND_SEQ_EVENT_NOTEON && val == 0) {
        x->midi_event_buf[x->buf_write_index].type = SND_SEQ_EVENT_NOTEOFF;
        x->midi_event_buf[x->buf_write_index].data.note.channel = mapped;
        x->midi_event_buf[x->buf_write_index].data.note.note = param;
        x->midi_event_buf[x->buf_write_index].data.note.velocity = val;
    }

    ph_debug_post("MIDI received in buffer: chan %d, param %d, val %d, mapped to %d",
            chan, param, val, mapped);

    x->buf_write_index = (x->buf_write_index + 1) % EVENT_BUFSIZE;
}

static void ph_list(ph *x, t_symbol *s, int argc, t_atom *argv)
{
    char msg_type[TYPE_STRING_SIZE];
    int ev_type = 0;
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

    if(ev_type != 0) {
        if(chan >= 0) {
            ph_midibuf_add(x, ev_type, chan, param, val);
        } else {
            while(n_instances--) {
                ph_midibuf_add(x, ev_type, n_instances, param, val);
            }
        }
    }
}

static char *ph_send_configure(ph *x, const char *key, const char *value, 
        int instance){

    char *debug;

    debug =   x->descriptor->configure(x->instance_handles[instance],
            key, value);
    /* TODO:OSC */
    /* if(instance->ui_target != NULL && x->is_dssi) {
            lo_send(instance->ui_target, 
                instance->ui_osc_configure_path,
                "ss", key, value);
        }
                */
    query_programs(x, instance);

    return debug;
}

static void ph_show(ph *x, unsigned int i, t_int toggle)
{
            /* TODO:OSC */
/*
    if(instance->ui_target){
        if (instance->ui_hidden && toggle) {
             lo_send(instance->ui_target, 
                    instance->ui_osc_show_path, ""); 
            instance->ui_hidden = 0;
        }
        else if (!instance->ui_hidden && !toggle) {
                    instance->ui_osc_hide_path, ""); 
            instance->ui_hidden = 1;
        }
    }
    else if(toggle){
        instance->ui_show = 1;
        ph_load_gui(x, instance);

    }
    */
}

static t_int ph_configure_buffer(ph *x, char *key, 
        char *value, unsigned int i){

    ph_configure_pair *current;
    ph_configure_pair *p;
    ph_instance       *instance;

    instance = &x->instances[i];
    current  = x->configure_buffer_head;

    while(current){
        if(!strcmp(current->key, key) && current->instance == i) {
            break;
        }
        current = current->next;
    }
    if(current) {
        free(current->value);
    } else {
        current                  = malloc(sizeof(ph_configure_pair));
        current->next            = x->configure_buffer_head;
        current->key             = strdup(key);
        current->instance        = i;
        x->configure_buffer_head = current;
    }
    current->value = strdup(value);
    p = x->configure_buffer_head;

    /*TODO: eventually give ability to query this buffer (to outlet?) */
    while(p){
        ph_debug_post("key: %s", p->key);
        ph_debug_post("val: %s", p->value);
        ph_debug_post("instance: %d", p->instance);
        p = p->next;
    }

    return 0;
}

static t_int ph_configure_buffer_free(ph *x)
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

static t_int ph_instance_reset(ph *x, int i)
{
    unsigned int n;
    const LADSPA_Descriptor *ladspa;

    ladspa = x->descriptor->LADSPA_Plugin;

    for(n = 0; n < x->n_instances; n++) {
        if (i == -1 || n == i) {
            if (ladspa->deactivate && ladspa->activate){
                ladspa->deactivate(x->instance_handles[n]);
                ladspa->activate(x->instance_handles[n]);
            }
        }
    }

    return 0;
}

static void ph_search_plugin_callback (
        const char* full_filename,
        void* plugin_handle,
        DSSI_Descriptor_Function descriptor_function,
        void* user_data,
        int is_dssi)
{
    DSSI_Descriptor* descriptor = NULL;
    unsigned plug_index = 0;

    char** out_lib_name = (char**)(((void**)user_data)[0]);
    char* name = (char*)(((void**)user_data)[1]);

    /* Stop searching when a first matching plugin is found */
    if (*out_lib_name == NULL)
    {
        ph_debug_post("pluginhost~: searching plugin \"%s\"...", full_filename);

        for(plug_index = 0;(is_dssi ? 
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

                /*	if(!is_dssi){
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

static const char* plugin_tilde_search_plugin_by_label (ph *x,
        const char *name)
{
    char* lib_name = NULL;
    void* user_data[2];

    user_data[0] = (void*)(&lib_name);
    user_data[1] = (void*)name;
    ph_debug_post("search plugin by label: '%s'\n", name);


    lib_name = NULL;
    LADSPAPluginSearch (ph_search_plugin_callback,
            (void*)user_data);

    /* The callback (allocates and) writes lib_name, if it finds the plugin */
    return lib_name;

}

static t_int ph_dssi_methods(ph *x, t_symbol *s, int argc, t_atom *argv) 
{
    if (!x->is_dssi) {
        post(
        "pluginhost~: plugin is not a DSSI plugin, operation not supported");
        return 0;
    }

    char msg_type[TYPE_STRING_SIZE];
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
    dx7_patch_t patchbuf[DX7_BANK_SIZE];
    dx7_patch_t *firstpatch;
    atom_string(argv, msg_type, TYPE_STRING_SIZE);
    debug = NULL;
    key = NULL;	
    value = NULL;
    maxpatches = 128; 
    firstpatch = &patchbuf[0];
    val = 0;

    /*TODO: Temporary - at the moment we always load the first 32 patches to 0 */
    if(strcmp(msg_type, "configure")){
        instance = (int)atom_getfloatarg(2, argc, argv) - 1;

        if(!strcmp(msg_type, "load") && x->descriptor->configure){
            filename = argv[1].a_w.w_symbol->s_name;
            post("pluginhost~: loading patch: %s for instance %d", filename, instance);

            if(!strcmp(x->descriptor->LADSPA_Plugin->Label, "hexter") || 
                    !strcmp(x->descriptor->LADSPA_Plugin->Label, "hexter6"))		{

                key = malloc(10 * sizeof(char)); /* holds "patchesN" */
                strcpy(key, "patches0");

                /* TODO: duplicates code from load_plugin() */
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
                x->channel_map[i+1] = chan;
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
                debug = ph_send_configure(x, key, value, n_instances);
                ph_configure_buffer(x, key, value, n_instances);
            }
        }
        /*TODO: Put some error checking in here to make sure instance is valid*/
        else{

            debug = ph_send_configure(x, key, value, instance);
            ph_configure_buffer(x, key, value, instance);
        }
    }
    ph_debug_post("The plugin returned %s", debug);

    return 0;
}

static void ph_osc_methods(ph *x, t_symbol *s, int argc, t_atom *argv) 
{

    unsigned int i;
    const char *method;
    char path[OSC_ADDR_MAX];
    ph_instance *instance;

    instance = NULL;

    atom_string(argv, path, TYPE_STRING_SIZE);

    if (strncmp(path, "/dssi/", 6)){
        ph_osc_debug_handler(path);
    }

    for (i = 0; i < x->n_instances; i++) {
        instance = &x->instances[i];
        if (!strncmp(path + 6, instance->osc_url_path,
                    strlen(instance->osc_url_path))) {
            break;
        }
    }

    if(instance == NULL) {
        post("%s: error instance not found");
        return;
    }

    if (!instance->osc_url_path){
        ph_osc_debug_handler(path);
    }

    method = path + 6 + strlen(instance->osc_url_path);

    if (*method != '/' || *(method + 1) == 0){
        ph_osc_debug_handler(path);
    }

    method++;

    switch(argc) {
        case 2:

            if (!strcmp(method, "configure") && 
                    argv[1].a_type == A_SYMBOL &&
                    argv[2].a_type == A_SYMBOL) {
                ph_osc_configure_handler(x, &argv[1], i);
            } else if (!strcmp(method, "control") &&
                    argv[1].a_type == A_FLOAT &&
                    argv[2].a_type == A_FLOAT) {
                ph_osc_control_handler(x, argv, i);
            } else if (!strcmp(method, "program") && 
                    argv[1].a_type == A_FLOAT &&
                    argv[2].a_type == A_FLOAT) {
                ph_osc_program_handler(x, argv, i);
            }
            break;

        case 1:
            if (!strcmp(method, "midi")) {
                ph_osc_midi_handler(x, argv, i);
            } else if (!strcmp(method, "update") &&
                    argv[1].a_type == A_SYMBOL) {
                ph_osc_update_handler(x, argv, i);
            }
            break;
        case 0:
            if (!strcmp(method, "exiting")) {
                ph_osc_exiting_handler(x, argv, i);
            }
            break;
        default:
            ph_osc_debug_handler(path);
            break;
    }
}

static void ph_instance_send_osc(t_outlet *outlet, ph_instance *instance, 
        t_int argc, t_atom *argv)
{

    outlet_anything(outlet, gensym("connect"), UI_TARGET_ELEMS, 
            instance->ui_target);
    outlet_anything(outlet, gensym("send"), argc, argv);
    outlet_anything(outlet, gensym("disconnect"), 0, NULL);

}

static void ph_bang(ph *x)
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
    outlet_anything (x->message_out, gensym ("running"), 3, at);
}

static t_int *ph_perform(t_int *w)
{
    unsigned int instance;
    unsigned int timediff;
    unsigned int framediff;
    unsigned int i;
    unsigned int N;
    t_float **inputs;
    t_float **outputs;
    ph *x;

    x       = (ph *)(w[1]);
    N       = (t_int)(w[2]);
    inputs  = (t_float **)(&w[3]);
    outputs = (t_float **)(&w[3] + x->plugin_ins);

    /*See comment for ph_plug_plugin */
    if(x->dsp){
        x->dsp_loop = true;

        for(i = 0; i < x->plugin_ins; i++)
            memcpy(x->plugin_input_buffers[i], inputs[i], N * 
                    sizeof(LADSPA_Data));

        for (i = 0; i < x->n_instances; i++)
            x->instance_event_counts[i] = 0;

        for (;x->buf_read_index != x->buf_write_index; x->buf_read_index = 
                (x->buf_read_index + 1) % EVENT_BUFSIZE) {

            instance = x->midi_event_buf[x->buf_read_index].data.note.channel;

            if(instance > x->n_instances){
                post(
            "pluginhost~: %s: discarding spurious MIDI data, for instance %d", 
                        x->descriptor->LADSPA_Plugin->Label, 
                        instance);
                ph_debug_post("n_instances = %d", x->n_instances);

                continue;
            }

            if (x->instance_event_counts[instance] == EVENT_BUFSIZE){
                post("pluginhost~: MIDI overflow on channel %d", instance);
                continue;
            }

            timediff = (t_int)(clock_gettimesince(x->time_ref) * 1000) - 
                x->midi_event_buf[x->buf_read_index].time.time.tv_nsec;
            framediff = (t_int)((t_float)timediff * .000001 / x->sr_inv); 

            if (framediff >= N || framediff < 0) 
                x->midi_event_buf[x->buf_read_index].time.tick = 0;
            else
                x->midi_event_buf[x->buf_read_index].time.tick = 
                    N - framediff - 1;

            x->instance_event_buffers[instance]
                [x->instance_event_counts[instance]] = 
                x->midi_event_buf[x->buf_read_index];
            ph_debug_post("%s, note received on channel %d", 
                    x->descriptor->LADSPA_Plugin->Label, 
                    x->instance_event_buffers[instance]
                    [x->instance_event_counts[instance]].data.note.channel);

            x->instance_event_counts[instance]++; 

            ph_debug_post("Instance event count for instance %d of %d: %d\n",
                    instance + 1, x->n_instances, x->instance_event_counts[instance]);


        }

        i = 0;
        while(i < x->n_instances){
            if(x->instance_handles[i] && 
                    x->descriptor->run_multiple_synths){
                x->descriptor->run_multiple_synths
                    (x->n_instances, x->instance_handles, 
                     (unsigned long)N, x->instance_event_buffers,
                     &x->instance_event_counts[0]);
                break; 
            }
            else if (x->instance_handles[i] && 
                    x->descriptor->run_synth){
                x->descriptor->run_synth(x->instance_handles[i], 
                        (unsigned long)N, x->instance_event_buffers[i],
                        x->instance_event_counts[i]); 
                i++;
            }
            else if (x->instance_handles[i] && 
                    x->descriptor->LADSPA_Plugin->run){
                x->descriptor->LADSPA_Plugin->run
                    (x->instance_handles[i], N);
                i++;
            }
        }


        for(i = 0; i < x->plugin_outs; i++)
            memcpy(outputs[i], (t_float *)x->plugin_output_buffers[i], N * 
                    sizeof(LADSPA_Data));

        x->dsp_loop = false;
    } 
    return w + (x->plugin_ins + x->plugin_outs + 3);
}

static void ph_dsp(ph *x, t_signal **sp)
{
    if(!x->n_instances){
        return;
    }


    t_int *dsp_vector, i, M;

    M = x->plugin_ins + x->plugin_outs + 2;

    dsp_vector = (t_int *) getbytes(M * sizeof(t_int));

    dsp_vector[0] = (t_int)x;
    dsp_vector[1] = (t_int)sp[0]->s_n;

    for(i = 2; i < M; i++)
        dsp_vector[i] = (t_int)sp[i - 1]->s_vec;

    dsp_addv(ph_perform, M, dsp_vector);

}

static void ph_quit_plugin(ph *x)
{

    unsigned int i;
    t_atom argv[2];
    t_int argc;
    ph_instance *instance;

    argc = 2;

    for(i = 0; i < x->n_instances; i++) {
        instance = &x->instances[i];
         if(x->is_dssi){
            argc = 2;
            SETSYMBOL(argv, gensym(instance->ui_osc_quit_path));  
            SETSYMBOL(argv+1, gensym(""));
            ph_instance_send_osc(x->message_out, instance, argc, argv);
         }
         ph_deactivate_plugin(x, i);
         ph_cleanup_plugin(x, i);
    }
}

static void ph_free_plugin(ph *x)
{
    unsigned int i;
    unsigned int n;

    if(x->plugin_label != NULL) {
        free((char *)x->plugin_label);
    }

    if(x->plugin_handle == NULL) {
        return;
    }

    free((LADSPA_Handle)x->instance_handles);
    free(x->plugin_ctlin_port_numbers); 
    free((t_float *)x->plugin_input_buffers);
    free(x->instance_event_counts);
    free(x->plugin_control_input);
    free(x->plugin_control_output);

    i = x->n_instances;

    while(i--){
        ph_instance *instance = &x->instances[i];

        /* TODO:OSC */
        /*
           if(instance->gui_pid){
           ph_debug_post("Killing GUI process PID = %d", instance->gui_pid);

           kill(instance->gui_pid, SIGINT);
           } */
        if (instance->plugin_pgms) {
            for (n = 0; n < instance->plugin_pgm_count; n++) {
                free((void *)instance->plugin_pgms[n].Name);
            }
            free(instance->plugin_pgms);
            instance->plugin_pgms = NULL;
            instance->plugin_pgm_count = 0;
        }
        free(x->instance_event_buffers[i]);
        if(x->is_dssi){
            free(instance->ui_osc_control_path);
            free(instance->ui_osc_configure_path);
            free(instance->ui_osc_program_path);
            free(instance->ui_osc_show_path);
            free(instance->ui_osc_hide_path);
            free(instance->ui_osc_quit_path);
            free(instance->osc_url_path);
        }
        free(instance->plugin_port_ctlin_numbers);
        if(x->plugin_outs) {
            free(x->plugin_output_buffers[i]);
        }
    }
    if(x->is_dssi) {
        if(x->project_dir != NULL) {
            free(x->project_dir);
        }
        free(x->osc_url_base);
        ph_configure_buffer_free(x);
    }
    free((snd_seq_event_t *)x->instance_event_buffers);
    free(x->instances);
    free((t_float *)x->plugin_output_buffers);

    if(x->plugin_ins){
        for(n = 0; n < x->plugin_ins; n++) {
            inlet_free((t_inlet *)x->inlets[n]);
        }
        freebytes(x->inlets, x->plugin_ins * sizeof(t_inlet *));
    }

    if(x->plugin_outs){
        for(n = 0; n < x->plugin_outs; n++) {
            outlet_free((t_outlet *)x->outlets[n]);
        }
        freebytes(x->outlets, x->plugin_outs * sizeof(t_outlet *));
    }
    if(x->message_out) {
        outlet_free(x->message_out);
    }
    if(x->plugin_basename) {
        free(x->plugin_basename);
    }
    if(x->port_info) {
        free(x->port_info);
    }
}

static void ph_init_plugin(ph *x)
{

    x->port_info                 = NULL;
    x->descriptor                = NULL;
    x->instance_event_counts     = NULL;
    x->instances                 = NULL;
    x->instance_handles          = NULL;
    x->osc_url_base              = NULL;
    x->configure_buffer_head     = NULL;
    x->project_dir               = NULL;
    x->outlets                   = NULL;
    x->inlets                    = NULL;
    x->message_out               = NULL;
    x->plugin_handle             = NULL;
    x->plugin_full_path          = NULL;
    x->plugin_label              = NULL;
    x->plugin_basename           = NULL;
    x->plugin_control_input      = NULL;
    x->plugin_control_output     = NULL;
    x->plugin_input_buffers      = NULL;
    x->plugin_output_buffers     = NULL;
    x->plugin_ctlin_port_numbers = NULL;
    x->plugin_ins                = 0;
    x->plugin_outs               = 0;
    x->plugin_control_ins        = 0;
    x->plugin_control_outs       = 0;
    x->is_dssi                   = 0;
    x->n_instances               = 0;
    x->dsp                       = 0;
    x->dsp_loop                  = 0;
    x->ports_in                  = 0;
    x->ports_out                 = 0;
    x->ports_control_in          = 0;
    x->ports_control_out         = 0;
    x->buf_write_index           = 0;
    x->buf_read_index            = 0;

}

static void *ph_load_plugin(ph *x, t_int argc, t_atom *argv)
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
    if((x->desc_func = (DSSI_Descriptor_Function)dlsym(x->plugin_handle,
                    "dssi_descriptor"))){
        x->is_dssi = true;
        x->descriptor = (DSSI_Descriptor *)x->desc_func(0);
    }
    else if((x->desc_func = 
            (DSSI_Descriptor_Function)dlsym(x->plugin_handle,
                "ladspa_descriptor"))){
        x->is_dssi = false;
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

    ph_set_port_info(x);
    ph_assign_ports(x);

    for(i = 0; i < x->n_instances; i++){
        x->instance_handles[i] = 
            x->descriptor->LADSPA_Plugin->
            instantiate(x->descriptor->LADSPA_Plugin, x->sr);
        if (!x->instance_handles[i]){
            post("pluginhost~: instantiation of instance %d failed", i);
            stop = 1;
            break;
        }
    }

    if(!stop){
        for(i = 0;i < x->n_instances; i++)
            ph_init_instance(x, i);
        for(i = 0;i < x->n_instances; i++)
            ph_connect_ports(x, i); 
        for(i = 0;i < x->n_instances; i++)
            ph_activate_plugin(x, i);

        if(x->is_dssi){
            for(i = 0;i < x->n_instances; i++)
                ph_osc_setup(x, i);
#if LOADGUI
            for(i = 0;i < x->n_instances; i++)
                ph_load_gui(x, i);
#endif

            for(i = 0;i < x->n_instances; i++)
                ph_init_programs(x, i);

            for(i = 0; i < x->n_instances && i < 128; i++){
                x->channel_map[i] = i;
            }
        }
    }

    x->message_out = outlet_new (&x->x_obj, gensym("control"));

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
static void ph_plug_plugin(ph *x, t_symbol *s, int argc, t_atom *argv)
{

    x->dsp = 0;
    ph_quit_plugin(x);
    while(1){
        if(!x->dsp_loop){
            ph_free_plugin(x);
            break;
        }
    }
    ph_init_plugin(x);
    ph_load_plugin(x, argc, argv);
}

static void *ph_new(t_symbol *s, t_int argc, t_atom *argv)
{

    ph *x = (ph *)pd_new(ph_class);
    post("\n========================================\npluginhost~: DSSI/LADSPA host - version %.2f\n========================================\n", VERSION);

    ph_init_plugin(x);

    x->sr       = (int)sys_getsr();
    x->sr_inv   = 1 / (t_float)x->sr;
    x->time_ref = (t_int)clock_getlogicaltime;
    x->blksize  = sys_getblksize();
    x->dsp      = 0;
    x->x_canvas = canvas_getcurrent();

    return ph_load_plugin(x, argc, argv);

}

static void ph_free(ph *x)
{

    ph_debug_post("Calling %s", __FUNCTION__);

    ph_quit_plugin(x);
    ph_free_plugin(x);

}

static void ph_sigchld_handler(int sig)
{
    wait(NULL);
}


void pluginhost_tilde_setup(void)
{

    ph_class = class_new(gensym("pluginhost~"), (t_newmethod)ph_new,
            (t_method)ph_free, sizeof(ph), 0, A_GIMME, 0);
    class_addlist(ph_class, ph_list);
    class_addbang(ph_class, ph_bang);
    class_addmethod(ph_class, 
            (t_method)ph_dsp, gensym("dsp"), 0);
    class_addmethod(ph_class, (t_method)ph_dssi_methods, 
            gensym("dssi"), A_GIMME, 0);
    class_addmethod (ph_class,(t_method)ph_control, 
            gensym ("control"),A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod (ph_class,(t_method)ph_info,
            gensym ("info"),0);
    class_addmethod (ph_class,(t_method)ph_list_plugins,
            gensym ("listplugins"),0);
    class_addmethod (ph_class,(t_method)ph_instance_reset,
            gensym ("reset"), A_DEFFLOAT, 0);
    class_addmethod (ph_class,(t_method)ph_plug_plugin,
            gensym ("plug"),A_GIMME,0);
    class_addmethod (ph_class, (t_method)ph_osc_methods,
            gensym("osc"), A_GIMME, 0);
    class_sethelpsymbol(ph_class, gensym("pluginhost~-help"));
    CLASS_MAINSIGNALIN(ph_class, ph, f);
    signal(SIGCHLD, ph_sigchld_handler);
}

static void ph_debug_post(const char *fmt, ...)
{
#if DEBUG
    unsigned int currpos;
    char     newfmt[DEBUG_STRING_SIZE];
    char     result[DEBUG_STRING_SIZE];
    size_t   fmt_length;
    va_list  args;

    fmt_length = strlen(fmt);

    sprintf(newfmt, "%s: ", MY_NAME);
    strncat(newfmt, fmt, fmt_length);
    currpos         = strlen(MY_NAME) + 2 + fmt_length;
    newfmt[currpos] = '\0';

    va_start(args, fmt);
    vsprintf(result, newfmt, args);
    va_end(args);

    post(result);
#endif
}

