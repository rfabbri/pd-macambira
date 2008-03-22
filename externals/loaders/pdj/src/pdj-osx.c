#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#include <mach-o/ldsyms.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "pdj.h"

/**
 * === USING AWT WITH OS X
 *
 * Unlike Linux or Windows, you cannot just simply fire-up a AWT form on OS X. This is 
 * because the event GUI mecanism has these limitation :
 * 
 * --> A CFRunLoopRun must be park in the main thread.
 * --> Java must be run in a secondary thread.
 *
 * Since this prerequisite need a pure-data patch, we will write our own pd scheduler.
 * This scheduler will simple fire-up another thread that will run the real pd 
 * scheduler (m_mainloop) and park the main thread with a CFRunLoopRun.
 * 
 * To be able to use the pdj scheduler, you need to apply a patch to pd. (Yes there
 * is a bug with the -schedlib option). The patch is available in the directory
 * src/pd_patch/osx_extsched_fix.patch. This patch has been made on a miller's 41.2
 * pd version.
 *
 * Once the patch is applied and compiled, you must configure your pure-data environement
 * to add the option : -schedlib [fullpath of the pdj external without the extension]. Use
 * the menu Pd -> Preference -> Startup to do this. Don't forget to click on [Save All
 * Settings].
 * 
 * Be carfull when you configure this switch since it can crash PD on startup. If you do
 * have the problem; you will have to delete all pd-preference by deleting file:
 * ~/Library/Preferences/org.puredata.pd.plist
 * 
 * If the scheduler is loaded, you should have this pd message :
 *    'pdj: using pdj scheduler for Java AWT.'
 */

int rc_pd = 0xFF;
CFRunLoopRef cfrunloop;


/* setting the environment varible APP_NAME_<pid> to the applications name */
/* sets it for the application menu */
void setAppName(const char * name) {
    char a[32];
    pid_t id = getpid();
    sprintf(a,"APP_NAME_%ld",(long)id);
    setenv(a, name, 1);
}


/** 
 * The main pd thread, will become a secondary thread to AWT.
 */
void *pdj_pdloop(void *notused) {
    post("pdj: using pdj scheduler for Java AWT.");
    rc_pd = 0;        

    // this will tell the java init code to initialize AWT before anything
    setenv("PDJ_USE_AWT", "true", 1);

    // we create the JVM now
    init_jvm();

    /* open audio and MIDI */
    sys_reopen_midi();
    sys_reopen_audio();

    rc_pd = m_mainloop();
    
    exit(rc_pd);
}


/* call back for dummy source used to make sure the CFRunLoop doesn't exit right away */
/* This callback is called when the source has fired. */
void sourceCallBack (  void *info  ) {
}


/**
 * This function is called when pdj is considered a scheduler on pd
 */
int pd_extern_sched(void *notused) {    
    CFRunLoopSourceContext sourceContext;

    /* Start the thread that runs the pure-data main thread and the VM. */
    pthread_t vmthread;    

	setAppName("pdj");

	/* create a new pthread copying the stack size of the primordial pthread */ 
    struct rlimit limit;
    size_t stack_size = 0;
    int rc = getrlimit(RLIMIT_STACK, &limit);
    if (rc == 0) {
        if (limit.rlim_cur != 0LL) {
            stack_size = (size_t)limit.rlim_cur;
        }
    }
    
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (stack_size > 0) {
        pthread_attr_setstacksize(&thread_attr, stack_size);
    }
	
    /* Start the thread that we will start the pd main thread */
    pthread_create(&vmthread, &thread_attr, pdj_pdloop, NULL);
   	pthread_attr_destroy(&thread_attr);
   	    
    /* Create a a sourceContext to be used by our source that makes */
    /* sure the CFRunLoop doesn't exit right away */
    sourceContext.version = 0;
    sourceContext.info = NULL;
    sourceContext.retain = NULL;
    sourceContext.release = NULL;
    sourceContext.copyDescription = NULL;
    sourceContext.equal = NULL;
    sourceContext.hash = NULL;
    sourceContext.schedule = NULL;
    sourceContext.cancel = NULL;
    sourceContext.perform = &sourceCallBack;
    
    /* Create the Source from the sourceContext */
    CFRunLoopSourceRef sourceRef = CFRunLoopSourceCreate(NULL, 0, &sourceContext);
    
    /* Use the constant kCFRunLoopCommonModes to add the source to the set of objects */ 
    /* monitored by all the common modes */
    CFRunLoopAddSource (CFRunLoopGetCurrent(), sourceRef, kCFRunLoopCommonModes); 
    
    /* Park this thread in the runloop for Java */
    CFRunLoopRun();
    
	return rc_pd;
}


int getuglylibpath(char *path) {
	char fullpath[MAXPDSTRING], *pfullpath;
    int cnt = _dyld_image_count();
	char *imagename = NULL;
	FILE *fd;
    int i;    

    // we get the bundle header, that contains the dyld header too...
    struct mach_header* header = (struct mach_header*) &_mh_bundle_header;
    
    for(i=1;i<cnt;i++) {
        if (_dyld_get_image_header(i) == header)
            imagename = (char *) _dyld_get_image_name(i);
    }

	if ( imagename != NULL ) {		
        strncpy(path, imagename, MAXPDSTRING);
	    // remove the pdj.pd_fat text
        path[strlen(imagename) - 10] = 0;
        return 0;
	} 
	
    error("unable to find pdj directory, looking into the user PD path");
	
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

    if ( rc_pd == 0xFF ) {
        post("pdj: warning: Java is initialized from main thread. AWT can lock pure-data environment. See pdj/pd scheduler to cover this problem.");
    }

    return (JNI_CreateJavaVM_func *) &JNI_CreateJavaVM;
}
