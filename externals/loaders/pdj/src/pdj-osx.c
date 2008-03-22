#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <CoreFoundation/CoreFoundation.h>

#include "pdj.h"

int getuglylibpath(char *path) {
	char fullpath[MAXPDSTRING], *pfullpath;
	FILE *fd;
	
	fd = (FILE *) open_via_path("", "pdj.properties", "", fullpath, &pfullpath, MAXPDSTRING, 0);
	if ( fd != NULL ) {
		close(fd);
        if ( fullpath[0] != 0 ) {
        	if ( pfullpath == ((char *) &fullpath) ) {
        		getcwd(path, MAXPDSTRING);
        	} else {
            	strcpy(path, fullpath);
        	}
            return 0;
        }
    } 
    
    error("unable to find pdj directory, please add it in your pure-data path settings");
    return 1;
}

JNI_CreateJavaVM_func *linkjvm(char *vmtype) {
	char *jvmVersion = pdj_getProperty("pdj.osx.JAVA_JVM_VERSION");
	
	if ( jvmVersion != NULL ) {
		setenv("JAVA_JVM_VERSION", jvmVersion, 1);
	}

    return (JNI_CreateJavaVM_func *) &JNI_CreateJavaVM;
}