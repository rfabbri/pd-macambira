#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include "pdj.h"

// test if this system is amd64
#ifdef __LP64__
	#define ARCH "amd64"
	#define MAPPOS 73
#else
	#define ARCH "i386"
	#define MAPPOS 49
#endif

/*
 * This is why it is called getuglylibpath... getting the info from /proc...
 * if you have a better idea; well I would like to be informed
 */
int getuglylibpath(char *path) {
    char buffer[BUFFER_SIZE];
    FILE *f;

    sprintf(buffer, "/proc/%d/maps", getpid());
    f = fopen(buffer, "r");
    if ( f == NULL ) {
        perror("pdj: unable to map :");
        strcpy(path, ".");
        return 1;
    }

    while(!feof(f)) {
        fgets(buffer, BUFFER_SIZE-1, f);
        if ( strstr(buffer, "pdj.pd_linux") != NULL ) {
            buffer[strlen(buffer) - 14] = 0;
            strcpy(path, buffer + MAPPOS);
            fclose(f);
            return 0;
        }
    }

    /* not found, check in the current dir :( */
    post("pdj: humm... pdj path library not found, setting current path");
    strcpy(path, ".");
    fclose(f);
    return 1;
}


JNI_CreateJavaVM_func *linkjvm(char *vm_type) {
	JNI_CreateJavaVM_func *func;
	char work[BUFFER_SIZE];
	char *javahome = pdj_getProperty("pdj.JAVA_HOME");
	void *libVM;
	
	if ( javahome == NULL ) {
		javahome = getenv("JAVA_HOME");
	} else {
		sprintf(work, "%s/jre/lib/" ARCH "/%s/libjvm.so", javahome, vm_type);
		libVM = dlopen(work, RTLD_LAZY);
		
		if ( libVM == NULL ) {
			post("pdj: unable to use the JVM specified at pdj.JAVA_HOME");
			javahome = getenv("JAVA_HOME");		
		}
	}
	
	if ( javahome == NULL ) {
		post("pdj: using JVM from the LD_LIBRARY_PATH");
		libVM = dlopen("libjava.so", RTLD_LAZY);	
	} else {
		post("pdj: using JVM %s", javahome);
		/* using LD_LIBRARY_PATH + putenv doesn't work, load std jvm libs 
		 * with absolute path. order is important.
		 */	
		sprintf(work, "%s/jre/lib/" ARCH "/%s/libjvm.so", javahome, vm_type);
		dlopen(work, RTLD_LAZY);
				
		sprintf(work, "%s/jre/lib/" ARCH "/libverify.so", javahome);
		dlopen(work, RTLD_LAZY);
			
		sprintf(work, "%s/jre/lib/" ARCH "/libjava.so", javahome);
		dlopen(work, RTLD_LAZY);
		
		sprintf(work, "%s/jre/lib/" ARCH "/libmlib_image.so", javahome);
		dlopen(work, RTLD_LAZY);

		/* ELF should support dynamic LD_LIBRARY_PATH :( :( :( */
		sprintf(work, "%s/jre/lib/" ARCH "/libjava.so", javahome);
		libVM = dlopen(work, RTLD_LAZY);		
	}		
	
	if ( libVM == 0 ) {
		error("pdj: %s", dlerror());
		return NULL;	
	}
	
	func = dlsym(libVM, "JNI_CreateJavaVM");
	if ( func == 0 ) {
		error("pdj: %s", dlerror());
		return NULL;
	}
	return func;
}
