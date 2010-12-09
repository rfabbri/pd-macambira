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

#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>     /* for uint8_t      */
#include <stdlib.h>     /* for exit()       */
#include <sys/types.h>  /* for fork()       */
#include <signal.h>     /* for kill()       */
#include <sys/wait.h>   /* for wait()       */
#include <dirent.h>     /* for readdir()    */

#include "m_pd.h"
#include "dssi.h"

#define DX7_VOICE_SIZE_PACKED 	128 /*From hexter_types.h by Sean Bolton */
#define DX7_DUMP_SIZE_BULK 	4096+8

#define VERSION           0.99
#define MY_NAME           "pluginhost~"
#define EVENT_BUFSIZE     1024
#define OSC_BASE_MAX      1024
#define TYPE_STRING_SIZE  20
#define DIR_STRING_SIZE   1024
#define DEBUG_STRING_SIZE 1024
#define ASCII_n           110
#define ASCII_p           112
#define ASCII_c           99
#define ASCII_b           98
#define ASCII_t           116
#define ASCII_a           97

#define LOADGUI 0 /* FIX: deprecate this */
#ifdef DEBUG
#define CHECKSUM_PATCH_FILES_ON_LOAD 1
#endif

#define MIN(a,b) ((a)<(b)?(a):(b))

/*From dx7_voice.h by Sean Bolton */

typedef struct _dx7_patch_t {
    uint8_t data[128];
} dx7_patch_t;

typedef struct _ph_instance {

    unsigned int     plugin_ProgramCount;
    char            *ui_osc_control_path;
    char            *ui_osc_configure_path;
    char            *ui_osc_program_path;
    char            *ui_osc_show_path;
    char            *ui_osc_hide_path;
    char            *ui_osc_quit_path;
    char            *osc_url_path;
    long             currentBank;
    long             currentProgram;
    int              uiNeedsProgramUpdate;
    int              pendingProgramChange;
    int              pendingBankLSB;
    int              pendingBankMSB;
    int              ui_hidden;
    int              ui_show;

    int *plugin_PortControlInNumbers; /*not sure if this should go here?*/
    DSSI_Program_Descriptor *pluginPrograms;

} ph_instance;

typedef struct ph_configure_pair {

    struct ph_configure_pair *next;
    char   *value;
    char   *key;
    int    instance;

} ph_configure_pair;

//typedef struct ph_configure_pair t_ph_configure_pair;

typedef struct _port_info {

    t_atom lower_bound;
    t_atom upper_bound;
    t_atom data_type;
    t_atom p_default;
    t_atom type;
    t_atom name;

} ph_port_info;

typedef struct _ph_tilde {

    t_object x_obj; /* gah, this has to be firs in the struct, WTF? */

    int sr;
    int blksize;
    int time_ref;
    int ports_in;
    int ports_out;
    int ports_controlIn;
    int ports_controlOut;
    int bufWriteIndex;
    int bufReadIndex;

    bool is_DSSI;
    bool dsp;
    bool dsp_loop;

    char *plugin_basename;
    char *plugin_label;
    char *plugin_full_path;
    char *project_dir;
    void *plugin_handle;
    char *osc_url_base;

    float f;
    float sr_inv;
    float **plugin_InputBuffers;
    float **plugin_OutputBuffers;
    float *plugin_ControlDataInput;
    float *plugin_ControlDataOutput;

    unsigned int n_instances;
    unsigned int plugin_ins;
    unsigned int plugin_outs;
    unsigned int plugin_controlIns;
    unsigned int plugin_controlOuts;
    unsigned long *instanceEventCounts;
    unsigned long *plugin_ControlInPortNumbers;
    unsigned char channelMap[128];

    DSSI_Descriptor_Function desc_func;
    DSSI_Descriptor *descriptor;
    LADSPA_Handle *instanceHandles;

    t_inlet  **inlets;
    t_outlet **outlets;
    t_outlet *control_outlet;
    t_canvas *x_canvas;

    ph_port_info *port_info;
    ph_instance *instances;
    ph_configure_pair *configure_buffer_head;

    snd_seq_event_t **instanceEventBuffers;
    snd_seq_event_t midiEventBuf[EVENT_BUFSIZE];

} ph_tilde;

static char *ph_tilde_send_configure(ph_tilde *x, char *key, char *value,
        int instance);
static void MIDIbuf(int type, unsigned int chan, int param, int val,
        ph_tilde *x);
static void ph_debug_post(const char *fmt, ...);
static LADSPA_Data get_port_default(ph_tilde *x, int port);
