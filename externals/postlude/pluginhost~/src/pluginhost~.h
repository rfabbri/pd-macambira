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
#define DX7_BANK_SIZE           32

#define VERSION           0.99
#define MY_NAME           "pluginhost~"
#define EVENT_BUFSIZE     1024
#define OSC_BASE_MAX      1024
#define OSC_ADDR_MAX      8192
#define OSC_PORT          9998
#define DIR_STRING_SIZE   1024
#define DEBUG_STRING_SIZE 1024
#define TYPE_STRING_SIZE  20
#define UI_TARGET_ELEMS   2
#define ASCII_t           116
#define ASCII_p           112
#define ASCII_n           110
#define ASCII_c           99
#define ASCII_b           98
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

    unsigned int     plugin_pgm_count;
    bool             ui_needs_pgm_update;
    char            *ui_osc_control_path;
    char            *ui_osc_configure_path;
    char            *ui_osc_program_path;
    char            *ui_osc_show_path;
    char            *ui_osc_hide_path;
    char            *ui_osc_quit_path;
    char            *osc_url_path;
    long             current_bank;
    long             current_pgm;
    int              pending_pgm_change;
    int              pending_bank_lsb;
    int              pending_bank_msb;
    int              ui_hidden;
    int              ui_show;
    t_atom           ui_target[UI_TARGET_ELEMS]; /* host, port */

    int *plugin_port_ctlin_numbers; /*not sure if this should go here?*/
    DSSI_Program_Descriptor *plugin_pgms;

} ph_instance;

typedef struct ph_configure_pair {

    struct ph_configure_pair *next;
    unsigned int instance;
    char   *value;
    char   *key;

} ph_configure_pair;

typedef struct _port_info {

    t_atom lower_bound;
    t_atom upper_bound;
    t_atom data_type;
    t_atom p_default;
    t_atom type;
    t_atom name;

} ph_port_info;

typedef struct _ph {

    t_object x_obj; /* gah, this has to be first element in the struct, WTF? */

    int sr;
    int blksize;
    int time_ref;
    int ports_in;
    int ports_out;
    int ports_control_in;
    int ports_control_out;
    int buf_write_index;
    int buf_read_index;

    bool is_dssi;
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
    float **plugin_input_buffers;
    float **plugin_output_buffers;
    float *plugin_control_input;
    float *plugin_control_output;

    unsigned int osc_port;
    unsigned int n_instances;
    unsigned int plugin_ins;
    unsigned int plugin_outs;
    unsigned int plugin_control_ins;
    unsigned int plugin_control_outs;
    unsigned long *instance_event_counts;
    unsigned long *plugin_ctlin_port_numbers;
    unsigned char channel_map[128];

    DSSI_Descriptor_Function desc_func;
    DSSI_Descriptor *descriptor;
    LADSPA_Handle *instance_handles;

    t_inlet  **inlets;
    t_outlet **outlets;
    t_outlet *message_out;
    t_canvas *x_canvas;

    ph_port_info *port_info;
    ph_instance *instances;
    ph_configure_pair *configure_buffer_head;

    snd_seq_event_t **instance_event_buffers;
    snd_seq_event_t midi_event_buf[EVENT_BUFSIZE];

} ph;

static char *ph_send_configure(ph *x, const char *key, 
        const char *value, int instance);
static void ph_instance_send_osc(t_outlet *outlet, ph_instance *instance, 
        t_int argc, t_atom *argv);
static void ph_midibuf_add(ph *x, int type, unsigned int chan, int param, int value);
static void ph_debug_post(const char *fmt, ...);
static LADSPA_Data get_port_default(ph *x, int port);
