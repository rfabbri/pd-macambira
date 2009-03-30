/* Gphoto PD External */
/* Copyright Ben Bogart, 2009 */
/* This program is distributed under the params of the GNU Public License */

///////////////////////////////////////////////////////////////////////////////////
/* This file is part of the Gphoto PD External.                                 */
/*                                                                               */
/* Gphoto PD External is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by          */
/* the Free Software Foundation; either version 2 of the License, or             */
/* (at your option) any later version.                                           */
/*                                                                               */
/* The Gphoto PD External is distributed in the hope that they will be useful,  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 */
/* GNU General Public License for more details.                                  */
/*                                                                               */
/* You should have received a copy of the GNU General Public License             */
/* along with the Chaos PD Externals; if not, write to the Free Software         */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     */
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <m_pd.h>
#include <fcntl.h>
#include <pthread.h>
#include <gphoto2/gphoto2-camera.h>

t_class *gphoto_class;

typedef struct gphoto_struct {
	t_object x_obj;
	t_outlet *doneOutlet;
	int busy;
	pthread_attr_t threadAttr; //thread attributes
	int capturing;
} gphoto_struct;

// Struct to store A_GIMME data passed to thread.
typedef struct gphoto_gimme_struct {
	gphoto_struct *gphoto;
	t_symbol *s;
	int argc;
	t_atom *argv;
} gphoto_gimme_struct;

void *getConfigDetail(void *threadArgs) {
	int gp_ret;
	const char *textVal;
	const int *toggleVal;
	const float rangeVal;
	const char *label, *info;
	float rangeMax, rangeMin, rangeIncr;
	float floatVal;
	int intVal, i;

	t_symbol *key;

	Camera *camera;
	CameraWidget *config = NULL;
	CameraWidget *child = NULL;
	CameraWidgetType type;

	key = atom_getsymbol( ((gphoto_gimme_struct *)threadArgs)->argv ); // config key

	gp_ret = gp_camera_new (&camera);		
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	// INIT camera (without context)	
	gp_ret = gp_camera_init (camera, NULL); 
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock();}
	if (gp_ret == -105) {
		sys_lock(); 
		error("gphoto: Are you sure the camera is supported, connected and powered on?");
		sys_unlock(); 
		gp_camera_unref(camera);
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		return(NULL);
	}

	gp_ret = gp_camera_get_config (camera, &config, NULL); // get config from camera
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	gp_ret = gp_widget_get_child_by_name (config, key->s_name, &child); // get item from config
	if (gp_ret != 0) {
		sys_lock();
		error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));
		sys_unlock();
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		gp_camera_unref(camera);
		return(NULL);
	}

	gp_ret = gp_widget_get_type (child, &type);
	if (gp_ret != 0) {
		sys_lock; error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));
		error("gphoto: Invalid config key."); sys_unlock();
	} else {
		switch (type) {
			case GP_WIDGET_RADIO:
				gp_ret = gp_widget_get_label (child, &label);
				sys_lock();
				post("gphoto: Label: %s", label);
				post("gphoto: Type: Radio");
				sys_unlock();
				for (i=0; i<gp_widget_count_choices(child); i++) {
					gp_ret = gp_widget_get_choice(child,i,&textVal);
					sys_lock();
					post("gphoto: Choice #%d:\t%s",i , textVal);
					sys_unlock();
				}
				gp_ret = gp_widget_get_value(child, &textVal);
				sys_lock();
				post("gphoto: Current Value: %s", textVal);
				sys_unlock();
				break;
			case GP_WIDGET_TOGGLE:
				gp_ret = gp_widget_get_label (child, &label);
				gp_ret = gp_widget_get_value(child, &intVal);
				sys_lock();
				post("gphoto: Label: %s", label);
				post("gphoto: Type: Toggle");
				post("gphoto: Current Value: %d", intVal);
				sys_unlock();
				break;
			case GP_WIDGET_TEXT:
				gp_ret = gp_widget_get_label (child, &label);
				gp_ret = gp_widget_get_value(child, &textVal);
				sys_lock();				
				post("gphoto: Label: %s", label);
				post("gphoto: Type: Text");
				post("gphoto: Current Value: %d", textVal);
				sys_lock();
				break;
			case GP_WIDGET_RANGE:
				gp_ret = gp_widget_get_label (child, &label);
				gp_ret = gp_widget_get_range(child, &rangeMin, &rangeMax, &rangeIncr);
				gp_ret = gp_widget_get_value(child, &floatVal);
				if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock();}
				sys_lock();
				post("gphoto: Label: %s", label);
				post("gphoto: Type: Range");
				post("gphoto: Minimum Value: %f", rangeMin);
				post("gphoto: Maximum Value: %f", rangeMax);
				post("gphoto: Increment: %f", rangeIncr);
				post("gphoto: Current Value: %f", floatVal);
				sys_lock();
				break;
		}
	}

	// Free memory (not child?)
	gp_widget_unref (config);
	gp_camera_unref (camera);

	// We are done  and other messsages can now be sent.
	((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0;

	// Send bang out 2nd outlet when operation is done.
	sys_lock();
	outlet_bang(((gphoto_gimme_struct *)threadArgs)->gphoto->doneOutlet);
	sys_unlock();	

	return(NULL);	
}

// Wrap getConfigDetail
static void wrapGetConfigDetail(gphoto_struct *gphoto, t_symbol *s, int argc, t_atom *argv) {
	int ret;
	pthread_t thread1;

	if (!gphoto->busy) {

		// instance of structure
		gphoto_gimme_struct *threadArgs = (gphoto_gimme_struct *)malloc(sizeof(gphoto_gimme_struct));

		// packaging arguments into structure
		threadArgs->gphoto = gphoto;
		threadArgs->s = s;
		threadArgs->argc = argc;
		threadArgs->argv = argv;

		// We're busy
		gphoto->busy = 1;

		// Create thread
		ret = pthread_create( &thread1, &gphoto->threadAttr, getConfigDetail, threadArgs);
	} else {
		error("gphoto: ERROR: Already executing a command, try again later.");
	}

	return;
}

// list configuration
void *listConfig(void *threadArgs) {
	int gp_ret, numsections, numchildren, i, j;
	Camera *camera;
	CameraWidget *config = NULL;
	CameraWidget *child = NULL;
	CameraWidget *child2 = NULL;
	CameraWidgetType type;
	char *childName;

	gp_ret = gp_camera_new (&camera);		
	if (gp_ret != 0) {error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); gp_camera_unref(camera); return(NULL);}

	// INIT camera (without context)	
	gp_ret = gp_camera_init (camera, NULL); 
	if (gp_ret != 0) {error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));}
	if (gp_ret == -105) {
		sys_lock(); 
		error("gphoto: Are you sure the camera is supported, connected and powered on?");
		sys_unlock(); 
		gp_camera_unref(camera);
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		return(NULL);
	}

	gp_ret = gp_camera_get_config (camera, &config, NULL); // get config from camera
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	numsections = gp_widget_count_children(config);
	for (i=0; i<numsections; i++) {
		gp_widget_get_child (config, i, &child);
		gp_widget_get_type (child, &type);
	
		if (type == GP_WIDGET_SECTION) {
			//post("gphoto: Config Section: %s\n", childName);
			numchildren = gp_widget_count_children(child);
			for (j=0; j<numchildren; j++) {
				gp_widget_get_child (child, j, &child2);
				gp_widget_get_name (child2, &childName);
				gp_widget_get_type (child2, &type);

				sys_lock();
				outlet_symbol(((gphoto_gimme_struct *)threadArgs)->gphoto->x_obj.ob_outlet, gensym(childName));
				sys_unlock();
			}
		}
	}

	// Free memory (not child?)
	gp_widget_unref (config);
	gp_camera_unref (camera);

	// We are done  and other messsages can now be sent.
	((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0;

	// Send bang out 2nd outlet when operation is done.
	sys_lock();
	outlet_bang(((gphoto_gimme_struct *)threadArgs)->gphoto->doneOutlet);
	sys_unlock();	

	return(NULL);
}

// Wrap listConfig
static void wrapListConfig(gphoto_struct *gphoto) {
	int ret;
	pthread_t thread1;

	if (!gphoto->busy) {

		// instance of structure
		gphoto_gimme_struct *threadArgs = (gphoto_gimme_struct *)malloc(sizeof(gphoto_gimme_struct));

		// packaging arguments into structure
		threadArgs->gphoto = gphoto;

		// We're busy
		gphoto->busy = 1;

		// Create thread
		ret = pthread_create( &thread1, &gphoto->threadAttr, listConfig, threadArgs);
	} else {
		error("gphoto: ERROR: Already executing a command, try again later.");
	}

	return;
}

void *getConfig(void *threadArgs) {
	int gp_ret;
	const char *textVal;
	const int *toggleVal;
	const float rangeVal;
	float value;

	t_symbol *key;

	Camera *camera;
	CameraWidget *config = NULL;
	CameraWidget *child = NULL;
	CameraWidgetType type;

	key = atom_getsymbol( ((gphoto_gimme_struct *)threadArgs)->argv ); // config key

	gp_ret = gp_camera_new (&camera);		
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	// INIT camera (without context)	
	gp_ret = gp_camera_init (camera, NULL); 
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock();}
	if (gp_ret == -105) {
		sys_lock(); 
		error("gphoto: Are you sure the camera is supported, connected and powered on?");
		sys_unlock(); 
		gp_camera_unref(camera);
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		return(NULL);
	}

	gp_ret = gp_camera_get_config (camera, &config, NULL); // get config from camera
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	gp_ret = gp_widget_get_child_by_name (config, key->s_name, &child); // get item from config
	if (gp_ret != 0) {
		sys_lock();
		error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));
		sys_unlock();
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		gp_camera_unref(camera);
		return(NULL);
	}

	gp_ret = gp_widget_get_type (child, &type);
	if (gp_ret != 0) {
		sys_lock; error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));
		error("gphoto: Invalid config key."); sys_unlock();
	} else {
		switch (type) {
			case GP_WIDGET_RADIO:
				gp_ret = gp_widget_get_value(child, &textVal);
				sys_lock();
				outlet_symbol(((gphoto_gimme_struct *)threadArgs)->gphoto->x_obj.ob_outlet, gensym(textVal));
				sys_unlock();
				break;
			case GP_WIDGET_TOGGLE:
				gp_ret = gp_widget_get_value (child, &toggleVal); //  get widget value
				sys_lock();
				outlet_float(((gphoto_gimme_struct *)threadArgs)->gphoto->x_obj.ob_outlet, (int) toggleVal);
				sys_unlock();
				break;
			case GP_WIDGET_TEXT:
				gp_ret = gp_widget_get_value (child, &textVal);
				sys_lock();				
				outlet_symbol(((gphoto_gimme_struct *)threadArgs)->gphoto->x_obj.ob_outlet, gensym(textVal));
				sys_lock();
				break;
			case GP_WIDGET_RANGE:
				gp_ret = gp_widget_get_value (child, &rangeVal);
				if (gp_ret != 0) {error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));}
				sys_lock();
				outlet_float(((gphoto_gimme_struct *)threadArgs)->gphoto->x_obj.ob_outlet, rangeVal);
				sys_lock();
				break;
		}
	}

	// Free memory (not child?)
	gp_widget_unref (config);
	gp_camera_unref (camera);

	// We are done  and other messsages can now be sent.
	((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0;

	// Send bang out 2nd outlet when operation is done.
	sys_lock();
	outlet_bang(((gphoto_gimme_struct *)threadArgs)->gphoto->doneOutlet);
	sys_unlock();	

	return(NULL);	
}

// Wrap getConfig
static void wrapGetConfig(gphoto_struct *gphoto, t_symbol *s, int argc, t_atom *argv) {
	int ret;
	pthread_t thread1;

	if (!gphoto->busy) {

		// instance of structure
		gphoto_gimme_struct *threadArgs = (gphoto_gimme_struct *)malloc(sizeof(gphoto_gimme_struct));

		// packaging arguments into structure
		threadArgs->gphoto = gphoto;
		threadArgs->s = s;
		threadArgs->argc = argc;
		threadArgs->argv = argv;

		// We're busy
		gphoto->busy = 1;

		// Create thread
		ret = pthread_create( &thread1, &gphoto->threadAttr, getConfig, threadArgs);
	} else {
		error("gphoto: ERROR: Already executing a command, try again later.");
	}

	return;
}

void *setConfig(void *threadArgs) {
	int gp_ret, intValue;
	float floatValue;
	t_symbol *key, *textValue;
	char charkey[MAXPDSTRING];
	Camera *camera;
	CameraWidget *config = NULL;
	CameraWidget *child = NULL;
	CameraWidgetType type;

	sys_lock();
	key = atom_getsymbol( ((gphoto_gimme_struct *)threadArgs)->argv ); // config key
	sys_unlock();

	gp_ret = gp_camera_new (&camera);		
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock();}

	// INIT camera (without context)	
	gp_ret = gp_camera_init (camera, NULL); 
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock();}
	if (gp_ret == -105) {
		sys_lock(); 
		error("gphoto: Are you sure the camera is supported, connected and powered on?");
		sys_unlock(); 
		gp_camera_unref(camera);
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		return(NULL);
	}

	gp_ret = gp_camera_get_config (camera, &config, NULL); // get config from camera
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	gp_ret = gp_widget_get_child_by_name (config, key->s_name, &child); // get item from config
	if (gp_ret != 0) {
		sys_lock();
		error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));
		sys_unlock();
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		gp_camera_unref(camera);
		return(NULL);
	}

	gp_ret = gp_widget_get_type (child, &type);
	if (gp_ret != 0) {
		sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret));
		error("gphoto: Invalid config key."); sys_unlock(); 
	} else {
		switch (type) {
			// same method as TOGGLE type
			case GP_WIDGET_RADIO:				
				sys_lock(); 
				textValue = atom_getsymbol( ((gphoto_gimme_struct *)threadArgs)->argv+1 );
				post("argument: %s", textValue->s_name);
				strcpy(charkey,(char *)textValue->s_name);
				post("charkey: %s", charkey);				
				sys_unlock();

				gp_ret = gp_widget_set_value (child, &charkey); //  set widget value
				if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }

				gp_ret = gp_camera_set_config (camera, config, NULL); // set new config
				if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }
				break;
			case GP_WIDGET_TOGGLE:
				sys_lock(); 
				intValue = atom_getint( ((gphoto_gimme_struct *)threadArgs)->argv+1 );
				sys_unlock(); 

				gp_ret = gp_widget_set_value (child, &intValue); //  set widget value
				if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }

				gp_ret = gp_camera_set_config (camera, config, NULL); // set new config
				if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }
				break;

			case GP_WIDGET_RANGE:
				sys_lock(); 
				floatValue = atom_getfloat( ((gphoto_gimme_struct *)threadArgs)->argv+1 );
				sys_unlock(); 

				gp_ret = gp_widget_set_value (child, &floatValue); //  set widget value
				if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }

				gp_ret = gp_camera_set_config (camera, config, NULL); // set new config
				if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }

				break;
		}
	}

	// Free memory
	gp_widget_unref (config);
	gp_camera_unref(camera);
	
	// We are done  and other messsages can now be sent.
	((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0;
	
	// Send bang out 2nd outlet when operation is done.
	sys_lock();
	outlet_bang(((gphoto_gimme_struct *)threadArgs)->gphoto->doneOutlet);
	sys_unlock();

	return(NULL);
}

// Wrap setConfig
// TODO is there a way to have one wrapper for all funcs?
static void wrapSetConfig(gphoto_struct *gphoto, t_symbol *s, int argc, t_atom *argv) {
	int ret;
	pthread_t thread1;

	if (!gphoto->busy) {

		// instance of structure
		gphoto_gimme_struct *threadArgs = (gphoto_gimme_struct *)malloc(sizeof(gphoto_gimme_struct));

		// packaging arguments into structure
		threadArgs->gphoto = gphoto;
		threadArgs->s = s;
		threadArgs->argc = argc;
		threadArgs->argv = argv;

		// We're busy
		gphoto->busy = 1;

		// Create thread
		ret = pthread_create( &thread1, &gphoto->threadAttr, setConfig, threadArgs);
	} else {
		error("gphoto: ERROR: Already executing a command, try again later.");
	}

	return;
}

void *captureImage(void *threadArgs) {
	int gp_ret, fd;
	Camera *camera;
	CameraFile *camerafile;
	CameraFilePath camera_file_path;
	t_symbol *filename;

	sys_lock(); 
	filename = atom_getsymbol( ((gphoto_gimme_struct *)threadArgs)->argv ); // destination filename
	sys_unlock(); 

	gp_ret = gp_camera_new (&camera);	
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	// INIT camera (without context)	
	gp_ret = gp_camera_init (camera, NULL); 
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }
	if (gp_ret == -105) {
		sys_lock(); 
		error("gphoto: Are you sure the camera is supported, connected and powered on?");
		sys_unlock(); 
		gp_camera_unref(camera);
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		return(NULL);
	}

	gp_ret = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, NULL); 
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	fd = open( filename->s_name, O_CREAT | O_WRONLY, 0644); // create file descriptor

	gp_ret = gp_file_new_from_fd(&camerafile, fd); // create gphoto file from descriptor
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	gp_ret = gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name,
		     GP_FILE_TYPE_NORMAL, camerafile, NULL); // get file from camera
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	gp_ret = gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name, NULL);
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	// Free memory
	gp_camera_free(camera);

	// We are done  and other messsages can now be sent.
	((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0;

	// Send bang out 2nd outlet when operation is done.
	sys_lock();
	outlet_bang(((gphoto_gimme_struct *)threadArgs)->gphoto->doneOutlet);
	sys_unlock();

	pthread_exit(NULL);
	//return(NULL); // needed?
}

// Wrap captureImage
// TODO is there a way to have one wrapper for all funcs?
static void wrapCaptureImage(gphoto_struct *gphoto, t_symbol *s, int argc, t_atom *argv) {
	int ret;
	pthread_t thread1;

post("busy state: %d", gphoto->busy);

	if (!gphoto->busy) {

		// instance of structure
		gphoto_gimme_struct *threadArgs = (gphoto_gimme_struct *)malloc(sizeof(gphoto_gimme_struct));

		// packaging arguments into structure
		threadArgs->gphoto = gphoto;
		threadArgs->s = s;
		threadArgs->argc = argc;
		threadArgs->argv = argv;

		// We're busy
		gphoto->busy = 1;

		// Create thread
		ret = pthread_create( &thread1, &gphoto->threadAttr, captureImage, threadArgs);
		post("pthread return: %d", ret);
	} else {
		error("gphoto: ERROR: Already executing a command, try again later.");
	}
}

// Reset internal state to accept new commands.
static void reset(gphoto_struct *gphoto) {
	gphoto->busy = 0;

	return;
}

void *captureImages(void *threadArgs) {
	int gp_ret, fd;
	Camera *camera;
	CameraFile *camerafile;
	CameraFilePath camera_file_path;
	t_symbol *format;
	char filename[MAXPDSTRING];
	int count;
	int sleepTime;

	sys_lock(); 
	format = atom_getsymbol( ((gphoto_gimme_struct *)threadArgs)->argv ); // destination filename
	sleepTime = atom_getint ( ((gphoto_gimme_struct *)threadArgs)->argv+1 ); // loop sleep delay
	sys_unlock();

	// we don't want a delay of 0! (1 ok?)
	if (sleepTime <=0) {
		sleepTime = 1;
		sys_lock();
		error("gphoto: ERROR: The minimum sleep value is 1 second. Sleep set to 1 second.")
		sys_unlock();
	} 

	gp_ret = gp_camera_new (&camera);	
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

	// INIT camera (without context)	
	gp_ret = gp_camera_init (camera, NULL); 
	if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); }
	if (gp_ret == -105) {
		sys_lock(); 
		error("gphoto: Are you sure the camera is supported, connected and powered on?");
		sys_unlock(); 
		gp_camera_unref(camera);
		((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0; // no longer busy if we got an error.
		return(NULL);
	}

	count = 0;
	while (((gphoto_gimme_struct *)threadArgs)->gphoto->capturing) {

		// Create filename from format. This does not check if the format string is suitable.
		sprintf(&filename, format->s_name, count);

		gp_ret = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, NULL); 
		if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

		fd = open( filename, O_CREAT | O_WRONLY, 0644); // create file descriptor

		gp_ret = gp_file_new_from_fd(&camerafile, fd); // create gphoto file from descriptor
		if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

		gp_ret = gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, camerafile, NULL); // get file from camera
		if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

		gp_ret = gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name, NULL);
		if (gp_ret != 0) {sys_lock(); error("gphoto: ERROR: %s\n", gp_result_as_string(gp_ret)); sys_unlock(); gp_camera_unref(camera); return(NULL);}

		close(fd); // close file descriptor

		// Send bang out 2nd outlet for each iteration.
		sys_lock();
		outlet_bang(((gphoto_gimme_struct *)threadArgs)->gphoto->doneOutlet);
		sys_unlock();

		sleep(sleepTime);
		count++;
	}

	// Free memory
	gp_camera_free(camera);

	// We are done  and other messsages can now be sent.
	((gphoto_gimme_struct *)threadArgs)->gphoto->busy = 0;

	pthread_exit(NULL);
}

// Wrap captureImage
// TODO is there a way to have one wrapper for all funcs?
static void wrapCaptureImages(gphoto_struct *gphoto, t_symbol *s, int argc, t_atom *argv) {
	int ret;
	pthread_t thread1;

	if (strcmp(atom_getsymbol(argv)->s_name, "stop") == 0) {

		gphoto->capturing = 0; // Stop Capturing.
		gphoto->busy = 0; // No longer busy

	} else if (!gphoto->busy) {

		if (argc != 2) {
			error("gphoto: ERROR: usage: captureimages [filename-format] [sleeptime (seconds)]");
		} else {

			gphoto->capturing = 1; // Now capturing.

			// instance of structure
			gphoto_gimme_struct *threadArgs = (gphoto_gimme_struct *)malloc(sizeof(gphoto_gimme_struct));

			// packaging arguments into structure
			threadArgs->gphoto = gphoto;
			threadArgs->s = s;
			threadArgs->argc = argc;
			threadArgs->argv = argv;

			// We're busy
			gphoto->busy = 1;

			// Create thread
			ret = pthread_create( &thread1, &gphoto->threadAttr, captureImages, threadArgs);
		}

	} else {

		error("gphoto: ERROR: Already executing a command, try again later.");
	}

	return;
}

static void *gphoto_new(void) {
	gphoto_struct *gphoto = (gphoto_struct *) pd_new(gphoto_class);
	outlet_new(&gphoto->x_obj, NULL);
	gphoto->doneOutlet = outlet_new(&gphoto->x_obj, &s_bang);

	// Initially the external is not "busy"
	gphoto->busy = 0;

	// When we create a thread, make sure it is deatched.
	pthread_attr_init(&gphoto->threadAttr);
	pthread_attr_setdetachstate(&gphoto->threadAttr, PTHREAD_CREATE_DETACHED);
	
	return (void *)gphoto;
}

static void *gphoto_destroy(gphoto_struct *gphoto) {
	pthread_attr_destroy(&gphoto->threadAttr);

	return(NULL);
}

void gphoto_setup(void) {
	gphoto_class = class_new(gensym("gphoto"), (t_newmethod) gphoto_new, (t_method) gphoto_destroy, sizeof(gphoto_struct), 0, CLASS_DEFAULT, 0);
	class_addmethod(gphoto_class, (t_method) wrapGetConfig, gensym("getconfig"), A_GIMME, 0);
	class_addmethod(gphoto_class, (t_method) wrapGetConfigDetail, gensym("configdetail"), A_GIMME, 0);
	class_addmethod(gphoto_class, (t_method) wrapCaptureImage, gensym("captureimage"), A_GIMME, 0);
	class_addmethod(gphoto_class, (t_method) wrapCaptureImages, gensym("captureimages"), A_GIMME, 0);
	class_addmethod(gphoto_class, (t_method) wrapSetConfig, gensym("setconfig"), A_GIMME, 0);
	class_addmethod(gphoto_class, (t_method) wrapListConfig, gensym("listconfig"), 0);
	class_addmethod(gphoto_class, (t_method) reset, gensym("reset"), 0);
}

