/* plugin~, a Pd tilde object for hosting LADSPA/VST plug-ins
   Copyright (C) 2000 Jarno Seppänen
   $Id: plugin~_ladspa.c,v 1.2 2003-01-23 12:32:04 ggeiger Exp $

   This file is part of plugin~.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#include "config.h"
#if PLUGIN_TILDE_USE_LADSPA

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plugin~.h"
#include "plugin~_ladspa.h"
/* LADSPA header */
#include "ladspa/ladspa.h"
/* LADSPA SDK helper functions loadLADSPAPluginLibrary() etc. */
#include "jutils.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif


const char*
plugin_tilde_ladspa_search_plugin (Pd_Plugin_Tilde* x,
				   const char* name)
{
    char* lib_name = NULL;
    void* user_data[2];

    user_data[0] = (void*)(&lib_name);
    user_data[1] = (void*)name;

    lib_name = NULL;
    LADSPAPluginSearch (plugin_tilde_ladspa_search_plugin_callback,
			(void*)user_data);

    /* The callback (allocates and) writes lib_name, if it finds the plugin */
    return lib_name;
}

static void
plugin_tilde_ladspa_search_plugin_callback (const char* full_filename,
					    void* plugin_handle,
					    LADSPA_Descriptor_Function descriptor_function,
					    void* user_data)
{
    const LADSPA_Descriptor* descriptor = NULL;
    unsigned plug_index = 0;

    char** out_lib_name = (char**)(((void**)user_data)[0]);
    char* name = (char*)(((void**)user_data)[1]);

    /* Stop searching when a first matching plugin is found */
    if (*out_lib_name == NULL)
    {
#if PLUGIN_TILDE_DEBUG
	error ("DEBUG plugin~: searching library \"%s\"...",
	       full_filename);
#endif

	for (plug_index = 0;
	     (descriptor = descriptor_function (plug_index)) != NULL;
	     plug_index++)
	{
#if PLUGIN_TILDE_DEBUG
	    error ("DEBUG plugin~: label \"%s\"",
		   descriptor->Label);
#endif

	    if (strcasecmp (name, descriptor->Label) == 0)
	    {
		/* found a matching plugin */
		*out_lib_name = strdup (full_filename);

#if PLUGIN_TILDE_DEBUG
		error ("DEBUG plugin~: found plugin \"%s\" in library \"%s\"",
		       name, full_filename);
#endif

		/* don't need to look any further */
		break;
	    }
	}
    }
}

int
plugin_tilde_ladspa_open_plugin (Pd_Plugin_Tilde* x,
				 const char* name,
				 const char* lib_name,
				 unsigned long sample_rate)
{
    unsigned port_index;

    /* precondition(s) */
    assert (x != NULL);
    assert (lib_name != NULL);
    assert (name != NULL);
    assert (sample_rate != 0);

    /* Initialize object struct */
    x->plugin.ladspa.type = NULL;
    x->plugin.ladspa.instance = NULL;
    x->plugin.ladspa.control_input_values = NULL;
    x->plugin.ladspa.control_output_values = NULL;
    x->plugin.ladspa.control_input_ports = NULL;
    x->plugin.ladspa.control_output_ports = NULL;
    x->plugin.ladspa.prev_control_output_values = NULL;
    x->plugin.ladspa.prev_control_output_values_invalid = 1;
    x->plugin.ladspa.outofplace_audio_outputs = NULL;
    x->plugin.ladspa.actual_audio_outputs = NULL;
    x->plugin.ladspa.num_samples = 0;
    x->plugin.ladspa.sample_rate = sample_rate;

    /* Attempt to load the plugin. */
    x->plugin_library = loadLADSPAPluginLibrary (lib_name);
    if (x->plugin_library == NULL)
    {
	/* error */
	error ("plugin~: Unable to load LADSPA plugin library \"%s\"",
	       lib_name);
	return 1;
    }
    x->plugin.ladspa.type = findLADSPAPluginDescriptor (x->plugin_library,
						 lib_name,
						 name);
    if (x->plugin.ladspa.type == NULL)
    {
	error ("plugin~: Unable to find LADSPA plugin \"%s\" within library \"%s\"",
	       name, lib_name);
	return 1;
    }

    /* Construct the plugin. */
    x->plugin.ladspa.instance
	= x->plugin.ladspa.type->instantiate (x->plugin.ladspa.type,
					      sample_rate);
    if (x->plugin.ladspa.instance == NULL)
    {
	/* error */
	error ("plugin~: Unable to instantiate LADSPA plugin \"%s\"",
	       x->plugin.ladspa.type->Name);
	return 1;
    }

#if PLUGIN_TILDE_DEBUG
    error ("DEBUG plugin~: constructed plugin \"%s\" successfully", x->plugin.ladspa.type->Name);
#endif

    /* Find out the number of inputs and outputs needed. */
    plugin_tilde_ladspa_count_ports (x);

    /* Allocate memory for control values */
    if (plugin_tilde_ladspa_alloc_control_memory (x)) {
	error ("plugin~: out of memory");
	return 1; /* error */
    }

    /* Connect control ports with buffers */
    plugin_tilde_ladspa_connect_control_ports (x);

    /* Activate the plugin. */
    if (x->plugin.ladspa.type->activate != NULL)
    {
	x->plugin.ladspa.type->activate (x->plugin.ladspa.instance);
    }

    /* success */
    return 0;
}

void
plugin_tilde_ladspa_close_plugin (Pd_Plugin_Tilde* x)
{
    /* precondition(s) */
    assert (x != NULL);

    if (x->plugin.ladspa.instance != NULL)
    {
	/* Deactivate the plugin. */
	if (x->plugin.ladspa.type->deactivate != NULL)
	{
	    x->plugin.ladspa.type->deactivate (x->plugin.ladspa.instance);
	}

	/* Destruct the plugin. */
	x->plugin.ladspa.type->cleanup (x->plugin.ladspa.instance);
	x->plugin.ladspa.instance = NULL;
    }

    /* Free the control value memory */
    plugin_tilde_ladspa_free_control_memory (x);

    if (x->plugin_library != NULL)
    {
	unloadLADSPAPluginLibrary (x->plugin_library);
	x->plugin_library = NULL;
	x->plugin.ladspa.type = NULL;
    }

    /* Free the out-of-place memory */
    plugin_tilde_ladspa_free_outofplace_memory (x);
}

void
plugin_tilde_ladspa_apply_plugin (Pd_Plugin_Tilde* x)
{
    unsigned i;

    /* Run the plugin on Pd's buffers */
    x->plugin.ladspa.type->run (x->plugin.ladspa.instance,
				x->plugin.ladspa.num_samples);

    /* Copy out-of-place buffers to Pd buffers if used */
    if (x->plugin.ladspa.outofplace_audio_outputs != NULL)
    {
	for (i = 0; i < x->num_audio_outputs; i++)
	{
	    unsigned j;
	    for (j = 0; j < (unsigned)x->plugin.ladspa.num_samples; j++)
	    {
		x->plugin.ladspa.actual_audio_outputs[i][j]
		    = x->plugin.ladspa.outofplace_audio_outputs[i][j];
	    }
	}
    }

    /* Compare control output values to previous and send control
       messages, if necessary */
    for (i = 0; i < x->num_control_outputs; i++)
    {
	/* Check whether the prev values have been initialized; if
	   not, send a control message for each of the control outputs */
	if ((x->plugin.ladspa.control_output_values[i]
	     != x->plugin.ladspa.prev_control_output_values[i])
	    || x->plugin.ladspa.prev_control_output_values_invalid)
	{
	    /* Emit a control message */
	    plugin_tilde_emit_control_output (x,
					      x->plugin.ladspa.type->PortNames[x->plugin.ladspa.control_output_ports[i]],
					      x->plugin.ladspa.control_output_values[i],
					      i);
	    /* Update the corresponding control monitoring value */
	    x->plugin.ladspa.prev_control_output_values[i] = x->plugin.ladspa.control_output_values[i];
	}
    }
    x->plugin.ladspa.prev_control_output_values_invalid = 0;
}

void
plugin_tilde_ladspa_print (Pd_Plugin_Tilde* x)
{
    unsigned port_index;
    unsigned i;

    printf ("%s: \"%s\"; control %d in/%d out; audio %d in/%d out\n"
	    "Loaded from library \"%s\".\n",
	    x->plugin.ladspa.type->Label,
	    x->plugin.ladspa.type->Name,
	    x->num_control_inputs,
	    x->num_control_outputs,
	    x->num_audio_inputs,
	    x->num_audio_outputs,
	    x->plugin_library_filename);

    for (i = 1; i <= 4; i++)
    {
	unsigned count;
	int print_controls = (i == 1 || i == 2);
	int print_audios = (i == 3 || i == 4);
	int print_inputs = (i == 1 || i == 3);
	int print_outputs = (i == 2 || i == 4);
	if (print_controls && print_inputs)
	{
	    printf ("Control input(s):\n");
	}
	else if (print_controls && print_outputs)
	{
	    printf ("Control output(s):\n");
	}
	else if (print_audios && print_inputs)
	{
	    printf ("Audio input(s):\n");
	}
	else if (print_audios && print_outputs)
	{
	    printf ("Audio output(s):\n");
	}
	count = 1;
	for (port_index = 0; port_index < x->plugin.ladspa.type->PortCount; port_index++)
	{
	    LADSPA_PortDescriptor port_type;
	    port_type = x->plugin.ladspa.type->PortDescriptors[port_index];

	    if ((print_controls && LADSPA_IS_PORT_CONTROL (port_type)
		 || print_audios && LADSPA_IS_PORT_AUDIO (port_type))
		&&
		(print_inputs && LADSPA_IS_PORT_INPUT (port_type)
		 || print_outputs && LADSPA_IS_PORT_OUTPUT (port_type)))
	    {
		printf (" #%d \"%s\"\n",
			count,
			x->plugin.ladspa.type->PortNames[port_index]);
		/* the port hints could also be printed... */
		count++;
	    }
	}
    }
}

void
plugin_tilde_ladspa_reset (Pd_Plugin_Tilde* x)
{
    /* precondition(s) */
    assert (x != NULL);
    assert (x->plugin.ladspa.type != NULL);
    assert (x->plugin.ladspa.instance != NULL);

    if (x->plugin.ladspa.type->activate != NULL
	&& x->plugin.ladspa.type->deactivate == NULL)
    {
	error ("plugin~: Warning: Plug-in defines activate() method but no deactivate() method");
    }

    /* reset plug-in by first deactivating and then re-activating it */
    if (x->plugin.ladspa.type->deactivate != NULL)
    {
	x->plugin.ladspa.type->deactivate (x->plugin.ladspa.instance);
    }
    if (x->plugin.ladspa.type->activate != NULL)
    {
	x->plugin.ladspa.type->activate (x->plugin.ladspa.instance);
    }
}

void
plugin_tilde_ladspa_connect_audio (Pd_Plugin_Tilde* x,
				   float** audio_inputs,
				   float** audio_outputs,
				   unsigned long num_samples)
{
    unsigned port_index = 0;
    unsigned input_count = 0;
    unsigned output_count = 0;

    /* Allocate out-of-place memory if needed */
    if (plugin_tilde_ladspa_alloc_outofplace_memory (x, num_samples)) {
	error ("plugin~: out of memory");
	return;
    }

    if (x->plugin.ladspa.outofplace_audio_outputs != NULL) {
	x->plugin.ladspa.actual_audio_outputs = audio_outputs;
	audio_outputs = x->plugin.ladspa.outofplace_audio_outputs;
    }

    input_count = 0;
    output_count = 0;
    for (port_index = 0; port_index < x->plugin.ladspa.type->PortCount; port_index++)
    {
	LADSPA_PortDescriptor port_type;
	port_type = x->plugin.ladspa.type->PortDescriptors[port_index];
	if (LADSPA_IS_PORT_AUDIO (port_type))
	{
	    if (LADSPA_IS_PORT_INPUT (port_type))
	    {
		x->plugin.ladspa.type->connect_port (x->plugin.ladspa.instance,
						     port_index,
						     (LADSPA_Data*)audio_inputs[input_count]);
		input_count++;
	    }
	    else if (LADSPA_IS_PORT_OUTPUT (port_type))
	    {
		x->plugin.ladspa.type->connect_port (x->plugin.ladspa.instance,
						     port_index,
						     (LADSPA_Data*)audio_outputs[output_count]);
		output_count++;
	    }
	}
    }

    x->plugin.ladspa.num_samples = num_samples;
}

void
plugin_tilde_ladspa_set_control_input_by_name (Pd_Plugin_Tilde* x,
				       const char* name,
				       float value)
{
    unsigned port_index = 0;
    unsigned ctrl_input_index = 0;
    int found_port = 0; /* boolean */

    /* precondition(s) */
    assert (x != NULL);

    if (name == NULL || strlen (name) == 0) {
	error ("plugin~: no control port name specified");
	return;
    }

    /* compare control name to LADSPA control input ports' names
       case-insensitively */
    found_port = 0;
    ctrl_input_index = 0;
    for (port_index = 0; port_index < x->plugin.ladspa.type->PortCount; port_index++)
    {
	LADSPA_PortDescriptor port_type;
	port_type = x->plugin.ladspa.type->PortDescriptors[port_index];
	if (LADSPA_IS_PORT_CONTROL (port_type)
	    && LADSPA_IS_PORT_INPUT (port_type))
	{
	    const char* port_name = NULL;
	    unsigned cmp_length = 0;
	    port_name = x->plugin.ladspa.type->PortNames[port_index];
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
	error ("plugin~: plugin doesn't have a control input port named \"%s\"",
	       name);
	return;
    }

    plugin_tilde_ladspa_set_control_input_by_index (x,
						    ctrl_input_index,
						    value);
}

void
plugin_tilde_ladspa_set_control_input_by_index (Pd_Plugin_Tilde* x,
				       unsigned ctrl_input_index,
				       float value)
{
    unsigned port_index = 0;
    unsigned ctrl_input_count = 0;
    int found_port = 0; /* boolean */
    int bounded_from_below = 0;
    int bounded_from_above = 0;
    int bounded = 0;
    float lower_bound = 0;
    float upper_bound = 0;
 
    /* precondition(s) */
    assert (x != NULL);
    /* assert (ctrl_input_index >= 0); causes a warning */
    /* assert (ctrl_input_index < x->num_control_inputs); */
    if (ctrl_input_index >= x->num_control_inputs) {
 	error ("plugin~: control port number %d is out of range [1, %d]",
 	       ctrl_input_index + 1, x->num_control_inputs);
 	return;
    }

    /* bound parameter value */
    /* sigh, need to find the N'th ctrl input port by hand */
    found_port = 0;
    ctrl_input_count = 0;
    for (port_index = 0; port_index < x->plugin.ladspa.type->PortCount; port_index++)
    {
	LADSPA_PortDescriptor port_type;
	port_type = x->plugin.ladspa.type->PortDescriptors[port_index];
	if (LADSPA_IS_PORT_CONTROL (port_type)
	    && LADSPA_IS_PORT_INPUT (port_type))
	{
	    if (ctrl_input_index == ctrl_input_count) {
		found_port = 1;
		break;
	    }
	    ctrl_input_count++;
	}
    }
    if (!found_port) {
	error ("plugin~: plugin doesn't have %ud control input ports",
	       ctrl_input_index + 1);
	return;
    }
    if (x->plugin.ladspa.type->PortRangeHints != NULL) {
	const LADSPA_PortRangeHint* hint
	    = &x->plugin.ladspa.type->PortRangeHints[port_index];
	if (LADSPA_IS_HINT_BOUNDED_BELOW (hint->HintDescriptor)) {
	    bounded_from_below = 1;
	    lower_bound = hint->LowerBound;
	    if (LADSPA_IS_HINT_SAMPLE_RATE (hint->HintDescriptor)) {
		assert (x->plugin.ladspa.sample_rate != 0);
		lower_bound *= (float)x->plugin.ladspa.sample_rate;
	    }
	}
	if (LADSPA_IS_HINT_BOUNDED_ABOVE (hint->HintDescriptor)) {
	    bounded_from_above = 1;
	    upper_bound = hint->UpperBound;
	    if (LADSPA_IS_HINT_SAMPLE_RATE (hint->HintDescriptor)) {
		assert (x->plugin.ladspa.sample_rate != 0);
		upper_bound *= (float)x->plugin.ladspa.sample_rate;
	    }
	}
    }
    bounded = 0;
    if (bounded_from_below && value < lower_bound) {
	value = lower_bound;
	bounded = 1;
    }
    if (bounded_from_above && value > upper_bound) {
	value = upper_bound;
	bounded = 1;
    }
    if (bounded) {
	fputs ("plugin~: warning: parameter limited to within ", stderr);
	if (bounded_from_below) {
	    fprintf (stderr, "[%f, ", lower_bound);
	}
	else {
	    fputs ("(-inf, ", stderr);
	}
	if (bounded_from_above) {
	    fprintf (stderr, "%f]\n", upper_bound);
	}
	else {
	    fputs ("inf)\n", stderr);
	}
    }
    /* set the appropriate control port value */
    x->plugin.ladspa.control_input_values[ctrl_input_index] = value;

#if PLUGIN_TILDE_DEBUG
    error ("DEBUG plugin~: control change control input port #%ud to value %f",
	   ctrl_input_index + 1, value);
#endif
}

static void
plugin_tilde_ladspa_count_ports (Pd_Plugin_Tilde* x)
{
    unsigned i = 0;

    x->num_audio_inputs = 0;
    x->num_audio_outputs = 0;
    x->num_control_inputs = 0;
    x->num_control_outputs = 0;

    for (i = 0; i < x->plugin.ladspa.type->PortCount; i++)
    {
	LADSPA_PortDescriptor port_type;
	port_type = x->plugin.ladspa.type->PortDescriptors[i];

	if (LADSPA_IS_PORT_AUDIO (port_type))
	{
	    if (LADSPA_IS_PORT_INPUT (port_type))
	    {
		x->num_audio_inputs++;
	    }
	    else if (LADSPA_IS_PORT_OUTPUT (port_type))
	    {
		x->num_audio_outputs++;
	    }
	}
	else if (LADSPA_IS_PORT_CONTROL (port_type))
	{
	    if (LADSPA_IS_PORT_INPUT (port_type))
	    {
		x->num_control_inputs++;
	    }
	    else if (LADSPA_IS_PORT_OUTPUT (port_type))
	    {
		x->num_control_outputs++;
	    }
	}
    }

#if PLUGIN_TILDE_DEBUG
    error ("DEBUG plugin~: plugin ports: audio %d/%d ctrl %d/%d",
	   x->num_audio_inputs, x->num_audio_outputs,
	   x->num_control_inputs, x->num_control_outputs);
#endif
}

static void
plugin_tilde_ladspa_connect_control_ports (Pd_Plugin_Tilde* x)
{
    unsigned port_index = 0;
    unsigned input_count = 0;
    unsigned output_count = 0;

    input_count = 0;
    output_count = 0;
    for (port_index = 0; port_index < x->plugin.ladspa.type->PortCount; port_index++)
    {
	LADSPA_PortDescriptor port_type;
	port_type = x->plugin.ladspa.type->PortDescriptors[port_index];

	if (LADSPA_IS_PORT_CONTROL (port_type))
	{
	    if (LADSPA_IS_PORT_INPUT (port_type))
	    {
		x->plugin.ladspa.type->connect_port (x->plugin.ladspa.instance,
					      port_index,
					      &x->plugin.ladspa.control_input_values[input_count]);
		x->plugin.ladspa.control_input_ports[input_count] = port_index;
		input_count++;
	    }
	    else if (LADSPA_IS_PORT_OUTPUT (port_type))
	    {
		x->plugin.ladspa.type->connect_port (x->plugin.ladspa.instance,
					      port_index,
					      &x->plugin.ladspa.control_output_values[output_count]);
		x->plugin.ladspa.control_output_ports[output_count] = port_index;

		output_count++;
	    }
	}
    }
}

static int
plugin_tilde_ladspa_alloc_outofplace_memory (Pd_Plugin_Tilde* x, unsigned long buflen)
{
    assert (x != NULL);

    plugin_tilde_ladspa_free_outofplace_memory (x);

    if (LADSPA_IS_INPLACE_BROKEN (x->plugin.ladspa.type->Properties)
	|| PLUGIN_TILDE_FORCE_OUTOFPLACE)
    {
	unsigned i = 0;

	x->plugin.ladspa.outofplace_audio_outputs = (t_float**)
	    calloc (x->num_audio_outputs, sizeof (t_float*));
	if (x->plugin.ladspa.outofplace_audio_outputs == NULL) {
	    return 1; /* error */
	}

	for (i = 0; i < x->num_audio_outputs; i++)
	{
	    x->plugin.ladspa.outofplace_audio_outputs[i] = (t_float*)
		calloc (buflen, sizeof (t_float));
	    if (x->plugin.ladspa.outofplace_audio_outputs[i] == NULL) {
		/* FIXME free got buffers? */
		return 1; /* error */
	    }
	}
    }
    return 0; /* success */
}

static void
plugin_tilde_ladspa_free_outofplace_memory (Pd_Plugin_Tilde* x)
{
    assert (x != NULL);

    if (x->plugin.ladspa.outofplace_audio_outputs != NULL)
    {
	unsigned i = 0;
	for (i = 0; i < x->num_audio_outputs; i++)
	{
	    free (x->plugin.ladspa.outofplace_audio_outputs[i]);
	}
	free (x->plugin.ladspa.outofplace_audio_outputs);
	x->plugin.ladspa.outofplace_audio_outputs = NULL;
    }
}

static int
plugin_tilde_ladspa_alloc_control_memory (Pd_Plugin_Tilde* x)
{
    x->plugin.ladspa.control_input_values = NULL;
    x->plugin.ladspa.control_input_ports = NULL;
    if (x->num_control_inputs > 0)
    {
	x->plugin.ladspa.control_input_values = (float*)calloc
	    (x->num_control_inputs, sizeof (float));
	x->plugin.ladspa.control_input_ports = (int*)calloc
	    (x->num_control_inputs, sizeof (int));
	if (x->plugin.ladspa.control_input_values == NULL
	    || x->plugin.ladspa.control_input_ports == NULL) {
	    return 1; /* error */
	}
    }
    x->plugin.ladspa.control_output_values = NULL;
    x->plugin.ladspa.control_output_ports = NULL;
    x->plugin.ladspa.prev_control_output_values = NULL;
    if (x->num_control_outputs > 0)
    {
	x->plugin.ladspa.control_output_values = (float*)calloc
	    (x->num_control_outputs, sizeof (float));
	x->plugin.ladspa.control_output_ports = (int*)calloc
	    (x->num_control_outputs, sizeof (int));
	x->plugin.ladspa.prev_control_output_values = (float*)calloc
	    (x->num_control_outputs, sizeof (float));
	if (x->plugin.ladspa.control_output_values == NULL
	    || x->plugin.ladspa.prev_control_output_values == NULL
	    || x->plugin.ladspa.control_output_ports == NULL) {
	    return 1; /* error */
	}
    }
    /* Indicate initial conditions */
    x->plugin.ladspa.prev_control_output_values_invalid = 1;
    return 0; /* success */
}

static void
plugin_tilde_ladspa_free_control_memory (Pd_Plugin_Tilde* x)
{
    if (x->plugin.ladspa.control_input_values != NULL)
    {
	free (x->plugin.ladspa.control_input_values);
	x->plugin.ladspa.control_input_values = NULL;
    }
    if (x->plugin.ladspa.control_output_values != NULL)
    {
	free (x->plugin.ladspa.control_output_values);
	x->plugin.ladspa.control_output_values = NULL;
    }
    if (x->plugin.ladspa.prev_control_output_values != NULL)
    {
	free (x->plugin.ladspa.prev_control_output_values);
	x->plugin.ladspa.prev_control_output_values = NULL;
    }
    if (x->plugin.ladspa.control_input_ports != NULL)
    {
	free (x->plugin.ladspa.control_input_ports);
	x->plugin.ladspa.control_input_ports = NULL;
    }
    if (x->plugin.ladspa.control_output_ports != NULL)
    {
	free (x->plugin.ladspa.control_output_ports);
	x->plugin.ladspa.control_output_ports = NULL;
    }
}

#endif /* PLUGIN_TILDE_USE_LADSPA */

/* EOF */
