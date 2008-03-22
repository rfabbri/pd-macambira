#include <stdlib.h>
#include "native_classes.h"
#include "type_handler.h"
#include "pdj.h"

typedef struct _clockCtnr {
	jobject instance;
	jmethodID tick;
	t_clock *pd_clock;
} t_clockCtnr;


static t_clockCtnr *getClock(JNIEnv *env, jobject obj) {
  	return (t_clockCtnr *) JPOINTER_CAST (*env)->GetLongField(env, obj, pdjCaching.FIDMaxClock_clock_ptr);
}


void clock_callback(t_clockCtnr *clk) {
	JNIEnv *env = pdjAttachVM();
	JASSERT(clk->instance);
	
	(*env)->CallVoidMethod(env, clk->instance , clk->tick, NULL);
	pdjDetachVM(env);
}


JNIEXPORT jdouble JNICALL Java_com_cycling74_max_MaxClock_getTime
  (JNIEnv *env, jclass cls) {
	return sys_getrealtime() * 1000;  	
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxClock_create_1clock
  (JNIEnv *env, jobject obj) {
  	jclass cls = (*env)->GetObjectClass(env, obj);
  	t_clockCtnr *clk;
  	
	clk = malloc(sizeof(t_clockCtnr));  	 	
  	ASSERT(clk);
  	
    clk->pd_clock = clock_new(clk, (t_method) clock_callback);
  	(*env)->SetLongField(env, obj, pdjCaching.FIDMaxClock_clock_ptr, (long) clk);

	clk->instance = (*env)->NewGlobalRef(env, obj);
	JASSERT(clk->instance);
	clk->tick = (*env)->GetMethodID(env, cls, "tick", "()V");
  	JASSERT(clk->tick);
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxClock_delay
  (JNIEnv *env, jobject obj, jdouble value) {
	t_clockCtnr *clk = getClock(env, obj);
	clock_delay(clk->pd_clock, value);
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxClock_unset
  (JNIEnv *env, jobject obj) {
  	t_clockCtnr *clk = getClock(env, obj);
  	clock_unset(clk->pd_clock);
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxClock_release
  (JNIEnv *env, jobject obj) {
  	t_clockCtnr *clk = getClock(env, obj);
  	clock_unset(clk->pd_clock);
  	clock_free(clk->pd_clock);
  	free(clk);
  	
  	(*env)->SetObjectField(env, obj, pdjCaching.FIDMaxClock_clock_ptr, 0);
  	(*env)->DeleteGlobalRef(env, obj);
}
