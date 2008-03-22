/**
 * This code is very ugly and needs rewrite. PERIOD.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pdj.h"

#define MAX_PROPERTIES 40
char *properties[MAX_PROPERTIES+1][2];

char *pdj_getProperty(char *name) {
	int i;
	
	for(i=0; properties[i][0] != NULL; i++) {
		if ( !strcmp(properties[i][0], name) ) {
			return properties[i][1];
		}
	}
	return NULL;
}


static void load_properties() {
	char propPath[BUFFER_SIZE];
	int  propIdx = 0;
	char *alloc;
	FILE *f;
	
	getuglylibpath(propPath);
	
	properties[0][0] = "pdj.home";
	alloc = malloc(strlen(propPath)+1);
	strcpy(alloc, propPath);
	properties[0][1] = alloc;
	
	strcat(propPath, DIR_SEP "pdj.properties");
	f = fopen(propPath, "r");
	
	if ( f == NULL ) {
		post("pdj: warning: property file not found at %s", propPath);
		return;
	}
	
	while(!feof(f)) {
		char buffer[BUFFER_SIZE];
		char *work, *key, *value;
		
		fgets(buffer, BUFFER_SIZE-1, f);
		
		work = strchr(buffer, '\n');
		if ( work != 0 ) {
			*work = 0;
			
			if ( work == buffer )
				continue;
		}
		
		/* cuts comments */
		work = strchr(buffer, '#');
		if ( work != NULL ) {
			*work = 0;
			
			if ( work == buffer ) 
				continue;
		}
		
		key = strtok(buffer, "=");
		if ( key == NULL ) {
			continue;
		}
		
		value = strtok(NULL, "");
		if ( value == NULL ) {
			value = "";
		}

		if ( propIdx == MAX_PROPERTIES ) {
			error("pdj: maximum defined properties");
			break;
		}
		propIdx++;
		
		alloc = malloc(strlen(key)+1);
		strcpy(alloc, key);
		properties[propIdx][0] = alloc;
		
		alloc = malloc(strlen(value)+1);
		strcpy(alloc, value);
		properties[propIdx][1] = alloc;
	}

	properties[propIdx+1][0] = NULL;
	properties[propIdx+1][1] = NULL;
	fclose(f);
}


static void copyToJavaSystemProperties(JNIEnv *env) {
	jclass system = (*env)->FindClass(env, "java/lang/System");		
	jmethodID id = (*env)->GetStaticMethodID(env, system, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	int i;
	
	for(i=0; properties[i][0] != NULL; i++) {
		jobject key, value;
		
		key = (*env)->NewStringUTF(env, properties[i][0]);
		JASSERT(key);
		value = (*env)->NewStringUTF(env, properties[i][1]);
		JASSERT(value);

		(*env)->CallStaticObjectMethod(env, system, id, key, value);
	}
}


PdjCaching pdjCaching;
static int initIDCaching(JNIEnv *env) {
	pdjCaching.cls_Atom = (*env)->FindClass(env, "com/cycling74/max/Atom");
	if ( pdjCaching.cls_Atom == NULL ) {
		// if the Atom class is not found... it means that pdj.jar is not on
		// classpath and the installation is broken.
		error("pdj: com.cycling74.max.Atom is not found on classpath ! pdj.jar must in the same directory of the external!");
		return 1;
	}
	pdjCaching.cls_Atom = (*env)->NewGlobalRef(env, pdjCaching.cls_Atom);
	
	pdjCaching.cls_MaxClock = (*env)->FindClass(env, "com/cycling74/max/MaxClock");
	JASSERT(pdjCaching.cls_MaxClock);
	pdjCaching.cls_MaxClock = (*env)->NewGlobalRef(env, pdjCaching.cls_MaxClock);

	pdjCaching.cls_MaxObject = (*env)->FindClass(env, "com/cycling74/max/MaxObject");
	JASSERT(pdjCaching.cls_MaxObject);
	pdjCaching.cls_MaxObject = (*env)->NewGlobalRef(env, pdjCaching.cls_MaxObject);

	pdjCaching.cls_MSPObject = (*env)->FindClass(env, "com/cycling74/msp/MSPObject");
	JASSERT(pdjCaching.cls_MSPObject);
	pdjCaching.cls_MSPObject = (*env)->NewGlobalRef(env, pdjCaching.cls_MSPObject);

	pdjCaching.cls_MSPSignal = (*env)->FindClass(env, "com/cycling74/msp/MSPSignal");
	JASSERT(pdjCaching.cls_MSPSignal);
	pdjCaching.cls_MSPSignal = (*env)->NewGlobalRef(env, pdjCaching.cls_MSPSignal);
	
	pdjCaching.MIDAtom_newAtom_Float = 
		(*env)->GetStaticMethodID(env, pdjCaching.cls_Atom, "newAtom", "(F)Lcom/cycling74/max/Atom;");
	JASSERT(pdjCaching.MIDAtom_newAtom_Float);
	
	pdjCaching.MIDAtom_newAtom_String =
		(*env)->GetStaticMethodID(env, pdjCaching.cls_Atom, "newAtom", "(Ljava/lang/String;)Lcom/cycling74/max/Atom;");
	JASSERT(pdjCaching.MIDAtom_newAtom_String);
	
	pdjCaching.FIDAtom_type = 
		(*env)->GetFieldID(env, pdjCaching.cls_Atom, "type", "I");
	JASSERT(pdjCaching.FIDAtom_type);
	
	pdjCaching.cls_AtomFloat = (*env)->FindClass(env, "com/cycling74/max/AtomFloat");
	JASSERT(pdjCaching.cls_AtomFloat);
	
	pdjCaching.FIDAtomFloat_value =
		(*env)->GetFieldID(env, pdjCaching.cls_AtomFloat, "value", "F");
	JASSERT(pdjCaching.FIDAtomFloat_value);
	
	pdjCaching.cls_AtomString = (*env)->FindClass(env, "com/cycling74/max/AtomString");
	JASSERT(pdjCaching.cls_AtomString);

	pdjCaching.FIDAtomString_value = 
		(*env)->GetFieldID(env, pdjCaching.cls_AtomString, "value", "Ljava/lang/String;");
	JASSERT(pdjCaching.FIDAtomString_value);
	
	pdjCaching.FIDMaxClock_clock_ptr =
		(*env)->GetFieldID(env, pdjCaching.cls_MaxClock, "_clock_ptr", "J");
	JASSERT(pdjCaching.FIDMaxClock_clock_ptr);
	
	pdjCaching.FIDMaxObject_pdobj_ptr =
		(*env)->GetFieldID(env, pdjCaching.cls_MaxObject, "_pdobj_ptr", "J");
	JASSERT(pdjCaching.FIDMaxObject_pdobj_ptr);

	pdjCaching.FIDMaxObject_activity_inlet =
		(*env)->GetFieldID(env, pdjCaching.cls_MaxObject, "_activity_inlet", "I");
	JASSERT(pdjCaching.FIDMaxObject_activity_inlet);
	
	pdjCaching.MIDMaxObject_trySetter =
		(*env)->GetMethodID(env, pdjCaching.cls_MaxObject, "_trySetter", "(Ljava/lang/String;[Lcom/cycling74/max/Atom;)Z");
	JASSERT(pdjCaching.MIDMaxObject_trySetter);
	
	pdjCaching.FIDMSPObject_used_inputs = 
		(*env)->GetFieldID(env, pdjCaching.cls_MSPObject, "_used_inputs", "[Lcom/cycling74/msp/MSPSignal;");
	JASSERT(pdjCaching.FIDMSPObject_used_inputs);
	
	pdjCaching.FIDMSPObject_used_outputs = 
		(*env)->GetFieldID(env, pdjCaching.cls_MSPObject, "_used_outputs", "[Lcom/cycling74/msp/MSPSignal;");
	JASSERT(pdjCaching.FIDMSPObject_used_outputs);

	pdjCaching.MIDMSPObject_dspinit = 
		(*env)->GetMethodID(env, pdjCaching.cls_MSPObject, "_dspinit", "(FI)Ljava/lang/reflect/Method;");
	JASSERT(pdjCaching.MIDMSPObject_dspinit);

	pdjCaching.MIDMSPObject_emptyPerformer = 
		(*env)->GetMethodID(env, pdjCaching.cls_MSPObject, "_emptyPerformer", "([Lcom/cycling74/msp/MSPSignal;[Lcom/cycling74/msp/MSPSignal;)V");
	JASSERT(pdjCaching.MIDMSPObject_emptyPerformer);

	pdjCaching.FIDMSPSignal_vec = 
		(*env)->GetFieldID(env, pdjCaching.cls_MSPSignal, "vec", "[F");
	JASSERT(pdjCaching.FIDMSPSignal_vec);
	
	return 0;
}


static int linkClasses(JNIEnv *env) {
	jclass pdjSystem = (*env)->FindClass(env, "com/e1/pdj/PDJSystem");
	jmethodID id;
	if ( pdjSystem == NULL ) {
		SHOWEXC;
		return 1;
	}	
	
	id = (*env)->GetStaticMethodID(env, pdjSystem, "_init_system", "()V");
	if ( id == NULL ) {
		SHOWEXC;
		return 1;
	}	
	
	(*env)->CallStaticVoidMethod(env, pdjSystem, id);
	if ( (*env)->ExceptionOccurred(env) ) {
		(*env)->ExceptionDescribe(env);
		return 1;
	}
	
	return 0;
}


void buildVMOptions(jint *nb, JavaVMOption *options) {
	static char cp[BUFFER_SIZE], pdj_cp[BUFFER_SIZE];
	char installPath[BUFFER_SIZE];
	char *prop;
	char *token, *work;
	int i;
	*nb = 0;

	getuglylibpath(installPath);
	
	// first; we set the system classpath
	strcpy(cp, "-Djava.class.path=");
	strcat(cp, installPath);
	strcat(cp, DIR_SEP "pdj.jar" PATH_SEP);
	prop = pdj_getProperty("pdj.system-classpath");
	if ( prop != NULL ) 
		strcat(cp, prop);
	options[0].optionString = cp;
	
	prop = pdj_getProperty("pdj.vm_args");
	if ( prop == NULL ) {
		*nb = 1;
		return; 
	}
	
	work = malloc(strlen(prop)+1);
	strcpy(work, prop);
	token = strtok(work, " ");

	for(i=*nb; i<32; i++) {
		*nb += 1;
		
		if ( token == NULL ) {		
			free(work);
			return;
		}

		options[*nb].optionString = malloc(strlen(token)+1);	
		strcpy(options[*nb].optionString, token);
		token = strtok(NULL, " ");
	}
	
	bug("pdj: maximum vm_args properties defined. Go see the source Luke.");
}


int REDIRECT_PD_IO;
static void redirectIoInit(void) {
	char *ret;
	
	ret = pdj_getProperty("pdj.redirect-pdio");
	if ( ret == NULL ) {
		REDIRECT_PD_IO = 1;
		return;
	}
	
	if ( ret[0] == '0' ) {
		REDIRECT_PD_IO = 0;
		return;
	}
	
	if ( strcmp(ret, "false") == 0 ) {
		REDIRECT_PD_IO = 0;
		return;
	}	
	
	REDIRECT_PD_IO = 1;	
}


JNIEnv *init_jvm(void) {
	JNI_CreateJavaVM_func *func;
	JavaVMOption opt[32];
	JavaVMInitArgs vm_args;  
	JNIEnv *jni_env;
	char *vm_type;
	int rc;
	
	load_properties();
	
	buildVMOptions(&(vm_args.nOptions), opt);
	vm_args.options = opt;
    vm_args.version = JNI_VERSION_1_4;
    vm_args.ignoreUnrecognized = JNI_FALSE; 
		
	vm_type = pdj_getProperty("pdj.vm_type");
	if ( vm_type == NULL ) {
		error("pdj: unknown vm_type, using client");
		vm_type = "client";
	}
	
	func = linkjvm(vm_type);	
	if ( func == NULL )
		return NULL;
		
	rc = func(&jni_jvm, &jni_env, &vm_args);
	if ( rc != 0 ) {
		error("pdj: unable to create JVM: JNI_CreateJavaVM = %d", rc);
		return NULL;
	}

	copyToJavaSystemProperties(jni_env);
	if ( initIDCaching(jni_env) != 0) {
		return NULL;
	}
	
	if ( linkClasses(jni_env) != 0 ) {
		return NULL;
	}
	
	redirectIoInit();
		
	return jni_env;
}

