/*
 *   Pure Data Packet module for cellular automata
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */



#include "pdp_ca.h"
#include <dlfcn.h>
#include <stdio.h>

t_class *pdp_ca_class;        // a cellular automaton processor: single input - single output
//t_class *pdp_ca2_class;     //                                 double input - single output
t_class *pdp_ca2image_class;   // converter from ca -> grey/yv12
t_class *pdp_image2ca_class;   // converter from grey/yv12 -> ca


// *********************** CA CLASS STUFF *********************



#define PDP_CA_STACKSIZE 256

typedef struct pdp_ca_data_struct
{
    unsigned int env[2*4];
    unsigned int reg[2*4];
    unsigned int stack[2*PDP_CA_STACKSIZE];
    short int random_seed[4];
} t_pdp_ca_data;

typedef struct pdp_ca_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    int x_queue_id;

    /* double buffering data packets */
    int x_packet0;
    int x_packet1;

    /* some data on the ca_routine */
    void (*x_ca_routine)(void);
    void *x_ca_libhandle;
    char *x_ca_rulenames;
    int x_ca_nbrules;
    char ** x_ca_rulename;
    
    /* nb of iterations */
    int x_iterations;



    /* aligned vector data */
    t_pdp_ca_data *x_data;

    /* output packet type */
    t_symbol *x_packet_type;

} t_pdp_ca;



/* process from packet0 -> packet1 */
static void pdp_ca_process_ca(t_pdp_ca *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    unsigned int  *data0   = (unsigned int *)pdp_packet_data  (x->x_packet0);
    unsigned int  *data1   = (unsigned int *)pdp_packet_data  (x->x_packet1);


    int width  = pdp_type_ca_info(header0)->width;
    int height = pdp_type_ca_info(header0)->height;
    int i,j;

    /* load TOS in middle of buffer to limit the effect of stack errors */
    unsigned int *tos = &x->x_data->stack[2*(PDP_CA_STACKSIZE/2)];
    unsigned int *env = &x->x_data->env[0];
    unsigned int *reg = &x->x_data->reg[0];
    void *ca_routine = x->x_ca_routine;
    unsigned int rtos;

    int offset = pdp_type_ca_info(header0)->offset;
    int xoffset = offset % width;
    int yoffset = offset / width;

    /* double word width: number of unsigned ints per row  */
    int dwwidth = width >> 5;

    unsigned long long result = 0;

    /* exit if there isn't a valid routine */
    if(!ca_routine) return;

    //post("pdp_ca: PRE offset: %d, xoffset: %d, yoffset: %d", offset, xoffset, yoffset);

    /* calculate new offset: lines shift up, rows shift left by 16 cells */
    xoffset = (xoffset + width - 16) % width;
    yoffset = (yoffset + height - 1) % height;

    offset =  yoffset * width + xoffset;

    //post("pdp_ca: PST offset: %d, xoffset: %d, yoffset: %d", offset, xoffset, yoffset);


    pdp_type_ca_info(header1)->offset = offset;


    for(j=0; j<dwwidth*(height - 2); j+=(dwwidth<<1)){
	for(i=0; i < (dwwidth-1) ; i+=1){
	    env[0] = data0[i + j];
	    env[1] = data0[i + j + 1];
	    env[2] = data0[i + j + dwwidth];
	    env[3] = data0[i + j + dwwidth + 1];
	    env[4] = data0[i + j + (dwwidth<<1)];
	    env[5] = data0[i + j + (dwwidth<<1) + 1];
	    env[6] = data0[i + j + (dwwidth<<1) + dwwidth];
	    env[7] = data0[i + j + (dwwidth<<1) + dwwidth + 1];
	    result = scaf_feeder(tos, reg, ca_routine, env);
	    data1[i + j] = result & 0xffffffff;
	    data1[i + j + dwwidth] = result >> 32;
	}
	// i == dwwidth-1

	env[0] = data0[i + j];
	env[1] = data0[j];
	env[2] = data0[i + j + dwwidth];
	env[3] = data0[j + dwwidth];
	env[4] = data0[i + j + (dwwidth<<1)];
	env[5] = data0[j + (dwwidth<<1)];
	env[6] = data0[i + j + (dwwidth<<1) + dwwidth];
	env[7] = data0[j + (dwwidth<<1) + dwwidth];
	result = scaf_feeder(tos, reg, ca_routine, env);
	data1[i + j] = result & 0xffffffff;
	data1[i + j + dwwidth] = result >> 32;
    }

    // j == dwwidth*(height - 2)
    for(i=0; i < (dwwidth-1) ; i+=1){
	env[0] = data0[i + j];
	env[1] = data0[i + j + 1];
	env[2] = data0[i + j + dwwidth];
	env[3] = data0[i + j + dwwidth + 1];
	env[4] = data0[i];
	env[5] = data0[i + 1];
	env[6] = data0[i + dwwidth];
	env[7] = data0[i + dwwidth + 1];
	result = scaf_feeder(tos, reg, ca_routine, env);
	data1[i + j] = result & 0xffffffff;
	data1[i + j + dwwidth] = result >> 32;
    }
    // j == dwwidth*(height - 2)
    // i == dwwidth-1
    env[0] = data0[i + j];
    env[1] = data0[j];
    env[2] = data0[i + j + dwwidth];
    env[3] = data0[j + dwwidth];
    env[4] = data0[i];
    env[5] = data0[0];
    env[6] = data0[i + dwwidth];
    env[7] = data0[dwwidth];
    result = scaf_feeder(tos, reg, ca_routine, env);
    data1[i + j] = result & 0xffffffff;
    data1[i + j + dwwidth] = result >> 32;



    /* check data stack pointer */
    rtos = (unsigned int)tos;

    if (env[0] != rtos){
	if (env[0] > rtos) post("pdp_ca: ERROR: stack underflow detected in ca routine");
	if (env[0] < rtos) post("pdp_ca: ERROR: ca routine returned more than one item");
	x->x_ca_routine = 0;
	post("pdp_ca: rule disabled");
	
    }

    return;
}


static void pdp_ca_swappackets(t_pdp_ca *x)
{
   /* swap packets */
   int packet = x->x_packet1;
   x->x_packet1 = x->x_packet0;
   x->x_packet0 = packet;
}





/* tick advance CA one timestep */
static void pdp_ca_bang_thread(t_pdp_ca *x)
{
   int encoding;
   int packet;
   int i;
  
   /* invariant: the two packets are allways valid and compatible 
      so a bang is allways possible. this means that in the pdp an 
      invalid packet needs to be converted to a valid one */

   for(i=0; i < x->x_iterations; i++){
      
       /* process form packet0 -> packet1 and propagate */
       pdp_ca_process_ca(x);

       /* swap */
       pdp_ca_swappackets(x);

   }

}

static void pdp_ca_sendpacket(t_pdp_ca *x)
{
   /* output the packet */
   outlet_pdp(x->x_outlet0, x->x_packet0);
}

static void pdp_ca_bang(t_pdp_ca *x)
{
    /* we don't use input packets for testing dropping here 
       but check the queue_id to see if processing is
       still going on */
    
    if (-1 == x->x_queue_id){
	pdp_queue_add(x, pdp_ca_bang_thread, pdp_ca_sendpacket, &x->x_queue_id);
    }

    else{
	pdp_control_notify_drop(-1);
    }
}


/* this method stores the packet into x->x_packet0 (the packet
   to be processed) if it is valid. x->x_packet1 is not compatible
   it is regenerated so that it is 
   
   in short, when this routine returns both packets are valid
   and compatible.
*/


static void pdp_ca_copy_rw_if_valid(t_pdp_ca *x, int packet)
{
    t_pdp *header  = pdp_packet_header(packet);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);


    int grabpacket;
    int convertedpacket;

    /* check if header is valid */
    if (!header) return;

    if (PDP_CA != header->type) return;
    if (PDP_CA_STANDARD != pdp_type_ca_info(header)->encoding) return;


    /* packet is a ca, register it */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = pdp_packet_copy_rw(packet);


    /* make sure we have the right header */
    header = pdp_packet_header(x->x_packet0);


    /* make sure that the other packet is compatible */
    if ((pdp_type_ca_info(header1)->width != pdp_type_ca_info(header)->width) ||
	 (pdp_type_ca_info(header1)->height != pdp_type_ca_info(header)->height)) {

	/* if not, throw away and clone the new one */
	pdp_packet_mark_unused(x->x_packet1);
	x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
    }


};

/* hot packet inlet */
static void pdp_ca_input_0(t_pdp_ca *x, t_symbol *s, t_floatarg f)
{

    if (s == gensym("register_rw")){
	pdp_ca_copy_rw_if_valid(x, (int)f);
    }
    else if (s == gensym("process")){
	pdp_ca_bang(x);
    }


}

/* cold packet inlet */
static void pdp_ca_input_1(t_pdp_ca *x, t_symbol *s, t_floatarg f)
{

    if (s == gensym("register_rw"))
    {
	pdp_ca_copy_rw_if_valid(x, (int)f);
    }

}


static void pdp_ca_rule_string(t_pdp_ca *x, char *c)
{
    char tmp[256];
    void (*prev_routine)(void);

    /* save previous routine ptr */
    prev_routine = x->x_ca_routine;

    /* check if we can find string */
    sprintf(tmp, "rule_%s", c);
    if (!(x->x_ca_routine = dlsym(x->x_ca_libhandle, tmp))){
	post("pdp_ca: can't fine ca rule %s (symbol: %s)", c, tmp);
	x->x_ca_routine = x->x_ca_routine;
	return;
    }

    /* all seems ok */
    //post("pdp_ca: using ca rule %s", c);
   
}
static void pdp_ca_rule(t_pdp_ca *x, t_symbol *s)
{
    /* make sure lib is loaded */
    if (!x->x_ca_libhandle) return;
    
    /* set rule by name */
    pdp_ca_rule_string(x, s->s_name);
}

static void pdp_ca_rule_index(t_pdp_ca *x, t_float f)
{
    int i = (int)f;

    /* make sure lib is loaded */
    if (!x->x_ca_libhandle) return;

    /* check index */
    if (i<0) return;
    if (i>=x->x_ca_nbrules) return;

    /* set rule by index */
    pdp_ca_rule_string(x, x->x_ca_rulename[i]);
    
}


static void pdp_ca_close(t_pdp_ca *x)
{
    if (x->x_ca_libhandle){
	dlclose(x->x_ca_libhandle);
	x->x_ca_libhandle = 0;
	x->x_ca_routine = 0;
	if (x->x_ca_rulename){
	    free (x->x_ca_rulename);
	    x->x_ca_rulename = 0;
	}
    

    }
}


static void pdp_ca_printrules(t_pdp_ca *x)
{
    int i;

    if (!(x->x_ca_libhandle)) return;
    post("pdp_ca: found %d rules: ", x->x_ca_nbrules);
    for(i=0;i<x->x_ca_nbrules; i++) post("%3d: %s ", i, x->x_ca_rulename[i]);


}

static void pdp_ca_open(t_pdp_ca *x, t_symbol *s)
{

    char *c;
    int words;

    /* close current lib, if one */
    pdp_ca_close(x);

    /* try to open new lib */
    if (!(x->x_ca_libhandle = dlopen(s->s_name, RTLD_NOW))){
	post("pdp_ca: can't open ca library %s, %s", s->s_name, dlerror());
	x->x_ca_libhandle = 0;
    }

    /* scan for valid rules */
    if (!(x->x_ca_rulenames = (char *)dlsym(x->x_ca_libhandle, "rulenames"))){
	post("pdp_ca: ERROR: %s does not contain a name table. closing.", s->s_name);
        pdp_ca_close(x);
	return;
    }

    /* count rules */
    words = 0;
    for(c = (char *)x->x_ca_rulenames; *c;){
	words++;
	while(*c++);
    }
    x->x_ca_nbrules = words;
    x->x_ca_rulename = (char **)malloc(sizeof(char *) * words);

    /* build name array */
    words = 0;
    for(c = (char *)x->x_ca_rulenames; *c;){
	x->x_ca_rulename[words] = c;
	words++;
	while(*c++);
    }

    /* ok, we're done */
    post("pdp_ca: opened rule library %s", s->s_name ,x->x_ca_nbrules);

    /* print rule names */
    //pdp_ca_printrules(x);

  
    
}

/* init the current packet with random noise */
static void pdp_ca_rand(t_pdp_ca *x){

    t_pdp *header = pdp_packet_header(x->x_packet0);
    short int *data = (short int *) pdp_packet_data(x->x_packet0);
    int i;

    int nbshortints = (pdp_type_ca_info(header)->width >> 4) * pdp_type_ca_info(header)->height;

    for(i=0; i<nbshortints; i++) 
	data[i] = random();
    
}


static void pdp_ca_newca(t_pdp_ca *x, t_float width, t_float height)
{
    int w = (int)width;
    int h = (int)height;
    int bytesize;
    t_pdp *header;

    /* ensure with = multiple of 64 */
    w &= 0xffffffc0;

    /* ensure height = multiple of 4 */
    w &= 0xfffffffc;

    w = (w<64) ? 64 : w;
    h = (h<4) ? 4 : h;

    bytesize = (w>>3) * h;

    /* delete old packets */
    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);


    /* create new packets */
    x->x_packet0 = pdp_packet_new(PDP_CA, bytesize);
    header = pdp_packet_header(x->x_packet0);
    pdp_type_ca_info(header)->encoding = PDP_CA_STANDARD;
    pdp_type_ca_info(header)->width = w;
    pdp_type_ca_info(header)->height = h;
    pdp_type_ca_info(header)->offset = 0;
    x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);


    /* fill with a test pattern */
    if(0)
    {
	unsigned int *d;
	int i, s;

	s = (w * h) >> 5;

	/* fill the first packet with 01 */
	d = (unsigned int *)pdp_packet_data(x->x_packet0);
	for (i=0; i<s; i++){
	    d[i] = i;
	}
    
	/* fill the second packet with 10 */
	d = (unsigned int *)pdp_packet_data(x->x_packet1);
	for (i=0; i<s; i++){
	    d[i] = i^-1;
	}
    
    
    
    }


    /* fill with random noise */
    pdp_ca_rand(x);
    
}


static void pdp_ca_iterations(t_pdp_ca *x, t_float f)
{
    int i = (int)f;

    if (i < 0) i = 0;

    x->x_iterations = i;
}

static void pdp_ca_free(t_pdp_ca *x)
{
    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);
    pdp_ca_close(x);
    free(x->x_data);
}



void *pdp_ca_new(void)
{
    t_pdp_ca *x = (t_pdp_ca *)pd_new(pdp_ca_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("pdp"), gensym("pdp1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("iterations"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;
    
    x->x_data = (t_pdp_ca_data *)malloc(sizeof(t_pdp_ca_data));
    x->x_ca_routine = 0;
    x->x_ca_libhandle = 0;
    x->x_ca_rulename = 0;

    pdp_ca_newca(x, 64, 64);
    pdp_ca_iterations(x, 1);

    x->x_packet_type = gensym("grey");

    return (void *)x;
}


// *********************** CA CONVERTER CLASSES STUFF *********************
// TODO: move this to a separate file later together with other converters (part of system?)

#define PDP_CA2IMAGE 1
#define PDP_IMAGE2CA 2

typedef struct pdp_ca_conv_struct
{
    t_object x_obj;
    t_float x_f;


    int x_threshold;

    int x_packet;

    t_outlet *x_outlet0;

    /* solve identity crisis */
    int x_whoami;

    /* output packet type */
    /* only greyscale for now */
    t_symbol *x_packet_type;

} t_pdp_ca_conv;

/* hot packet inlet */
static void pdp_ca_conv_input_0(t_pdp_ca_conv *x, t_symbol *s, t_floatarg f)
{
    int packet = -1;

    if (s == gensym("register_ro")){
	pdp_packet_mark_unused(x->x_packet);
	x->x_packet = pdp_packet_copy_ro((int)f);
	return;
    }
    else if (s == gensym("process")){
	switch(x->x_whoami){
	case PDP_CA2IMAGE:
	    packet = pdp_type_ca2grey(x->x_packet);
	    break;
	case PDP_IMAGE2CA:
	    packet = pdp_type_grey2ca(x->x_packet, x->x_threshold);
	    break;
	}
	
	/* throw away the original packet */
	pdp_packet_mark_unused(x->x_packet);
	x->x_packet = -1;

	/* unregister the freshly created packet */
	pdp_packet_mark_unused(packet);

	/* output if valid */
	if (-1 != packet) outlet_pdp(x->x_outlet0, packet);
    }


}

void pdp_ca_conv_free(t_pdp_ca_conv *x) 
{
    pdp_packet_mark_unused(x->x_packet);
}


void pdp_image2ca_threshold(t_pdp_ca_conv *x, t_float f)
{
    f *= 0x8000;

    if (f < -0x7fff) f = -0x7fff;
    if (f > 0x7fff) f = 0x7fff;

    x->x_threshold = (short int)f;
}

void *pdp_ca2image_new(void)
{
    t_pdp_ca_conv *x = (t_pdp_ca_conv *)pd_new(pdp_ca2image_class);
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet_type = gensym("grey");
    x->x_packet = -1;
    x->x_whoami = PDP_CA2IMAGE;
    return (void *)x;
}

void *pdp_image2ca_new(void)
{
    t_pdp_ca_conv *x = (t_pdp_ca_conv *)pd_new(pdp_image2ca_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("threshold"));
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet_type = gensym("grey");
    x->x_packet = -1;
    x->x_whoami = PDP_IMAGE2CA;
    x->x_threshold = 0x4000;
    return (void *)x;
}


// *********************** CLASS SETUP FUNCTIONS *********************

#ifdef __cplusplus
extern "C"
{
#endif



void pdp_ca2image_setup(void)
{
    pdp_ca2image_class = class_new(gensym("pdp_ca2image"), (t_newmethod)pdp_ca2image_new,
    	(t_method)pdp_ca_conv_free, sizeof(t_pdp_ca), 0, A_NULL);
    class_addmethod(pdp_ca2image_class, (t_method)pdp_ca_conv_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
}

void pdp_image2ca_setup(void)
{
    pdp_image2ca_class = class_new(gensym("pdp_image2ca"), (t_newmethod)pdp_image2ca_new,
    	(t_method)pdp_ca_conv_free, sizeof(t_pdp_ca), 0, A_NULL);
    class_addmethod(pdp_image2ca_class, (t_method)pdp_ca_conv_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_image2ca_class, (t_method)pdp_image2ca_threshold, gensym("threshold"),  A_FLOAT, A_NULL);
}

void pdp_ca_setup(void)
{


    pdp_ca_class = class_new(gensym("pdp_ca"), (t_newmethod)pdp_ca_new,
    	(t_method)pdp_ca_free, sizeof(t_pdp_ca), 0, A_NULL);

   
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_iterations, gensym("iterations"), A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_printrules, gensym("rules"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_rand, gensym("random"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_newca, gensym("ca"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_close, gensym("close"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_open, gensym("open"),  A_SYMBOL, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_rule, gensym("rule"),  A_SYMBOL, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_rule_index, gensym("ruleindex"),  A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_input_1, gensym("pdp1"), A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
