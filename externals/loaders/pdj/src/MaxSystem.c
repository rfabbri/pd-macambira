#include "native_classes.h"
#include "type_handler.h"
#include "pdj.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef MSW
	#include <io.h>
#else
	#include <unistd.h>
#endif

/* from m_imp.h... todo: ask Miller ??? */
EXTERN void outlet_setstacklim(void);


/**
 * Using a { or } in a post or error will lock PD, we substitute
 * them with a ( and ).
 */
static char* removePdAcc(char *str, jboolean isCopy) {
	char *work;
	
	if ( isCopy == JNI_TRUE ) {
		work = str;
		while(*work != 0) {
			switch(*work) {
				case '{' :
					*work = '(';
					break;
				case '}' :
					*work = ')';
					break;
			}
			work++;
		}
		return str;
	}
		
	work = malloc(strlen(str)+1);
	while(*str != 0) {
		switch(*str) {
			case '{' :
				*work = '(';
				break;
			case '}' :
				*work = ')';
				break;
			default:
				*work = *str;
		}
		work++;
		str++;
	}
	
	*work = 0;
	return work;
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxSystem_error
	(JNIEnv *env, jclass cls, jstring message) {
	jboolean isCopy;
	
	char *msg = (*env)->GetStringUTFChars(env, message, &isCopy);
	msg = removePdAcc(msg, isCopy);
	
	if ( REDIRECT_PD_IO ) {
		error(msg);
	} else {
		fprintf(stderr, msg);
		fprintf(stderr, "\n");
	}

	if ( isCopy == JNI_FALSE )
		free(msg);
	
	(*env)->ReleaseStringUTFChars(env, message, msg);
}
  

JNIEXPORT void JNICALL Java_com_cycling74_max_MaxSystem_post
	(JNIEnv *env, jclass cls, jstring message) {
	jboolean isCopy;
	
	char *msg = (*env)->GetStringUTFChars(env, message, &isCopy);
	msg = removePdAcc(msg, isCopy);
	
	if ( REDIRECT_PD_IO ) {
		post(msg);
	} else {
		fprintf(stdout, msg);
		fprintf(stdout, "\n");
	}
	
	if ( isCopy == JNI_FALSE )
		free(msg);
	
	(*env)->ReleaseStringUTFChars(env, message, msg);
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxSystem_ouch
	(JNIEnv *env, jclass cls, jstring message) {
	jboolean isCopy;
	
	char *msg = (*env)->GetStringUTFChars(env, message, &isCopy);
	fprintf(stderr, msg);
	fprintf(stderr, "\n");
	msg = removePdAcc(msg, isCopy);
	bug(msg);
	
	if ( isCopy == JNI_FALSE )
		free(msg);
		
	(*env)->ReleaseStringUTFChars(env, message, msg);
}


JNIEXPORT jboolean JNICALL Java_com_cycling74_max_MaxSystem_sendMessageToBoundObject
  (JNIEnv *env, jclass cls, jstring jname, jstring jmsg, jobjectArray jatoms) {
  	t_symbol *name = jstring2symbol(env, jname);
  	t_symbol *msg  = jstring2symbol(env, jmsg);
 	t_pd *dest = findPDObject(name);
	t_atom argv[MAX_ATOMS_STACK];
	int argc;
  	
  	if ( dest == NULL ) {
  		post("pdj: unable to get object %s for sendMessageToBoundObject", name->s_name);
  		return 0;
  	}

	/* reset the stack pointer for pd events */
	outlet_setstacklim();
  	
  	if ( msg == &s_bang ) {
  		pd_bang(dest);	
  		return 1;
  	}

	if ( jatoms == NULL ) {
		pd_symbol(dest, msg);
		return 1;
	}

  	jatoms2atoms(env, jatoms, &argc, argv);

  	if ( msg == &s_float ) {
  		pd_float(dest, atom_getfloatarg(0, argc, argv));
  		return 1;
  	}
  	
  	pd_list(dest, msg, argc, argv);
  	return 1;
}


JNIEXPORT jstring JNICALL Java_com_cycling74_max_MaxSystem_locateFile
  (JNIEnv *env, jclass cls, jstring filename) {
	const char *file = (*env)->GetStringUTFChars(env, filename, NULL);
	char fullpath[MAXPDSTRING], *pfullpath;
	FILE *fd;
	
	fd = open_via_path("", file, "", fullpath, &pfullpath, MAXPDSTRING, 0);
	(*env)->ReleaseStringUTFChars(env, filename, file);
	if ( fd != NULL ) {
		close(fd);
		if ( fullpath[0] != 0 ) {
        	if ( pfullpath == &fullpath ) {
        		getcwd(fullpath, MAXPDSTRING);
        	} 
        	return (*env)->NewStringUTF(env, fullpath);
        }
	}

	return NULL;		
}
