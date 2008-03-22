#include <stdlib.h>
#include <string.h>
#include "pdj.h"

t_class *pdj_tilde_class;


t_int *pdj_tilde_perform(t_int *w) {
	JNIEnv *env = pdjAttachVM();
	t_pdj_tilde *pdjt = (t_pdj_tilde *) (w[1]);
	int sz = (int) (w[2]);
	int i, work;
	
	/* copy buffer in */
	for (i=0;i<pdjt->ins_count;i++) {
		t_sample *in = (t_sample *)(w[i+3]);
		(*env)->SetFloatArrayRegion(env, pdjt->ins[i], 0, sz, in);
	}
	
	/* call the performer */
	(*env)->CallVoidMethod(env, pdjt->pdj.obj, pdjt->performer,
		pdjt->_used_inputs, pdjt->_used_outputs);

	/* set work to output outlets */
	work = i + 3;
			
	/* if an exception occured, stop the dsp processing for this object */
	if ( (*env)->ExceptionOccurred(env) ) {
		int tmp;
		
		/* insert silence */
		for (tmp=0;tmp<pdjt->outs_count;tmp++) {
			t_sample *out = (t_sample *)(w[work+tmp]);
			memset(out, 0, sizeof(float) * sz);
		}
		
		error("pdj~: exception occured in dsp processing of: %s", pdjt->pdj.jobject_name);
		(*env)->ExceptionDescribe(env);
		
		/* cancels current dsp processing */
		pdjt->performer = pdjCaching.MIDMSPObject_emptyPerformer;
	}
	
	/* copy buffer out */
	for (i=0;i<pdjt->outs_count;i++) {
		t_sample *out = (t_sample *)(w[work+i]);
		(*env)->GetFloatArrayRegion(env, pdjt->outs[i], 0, sz, out);
	}

	pdjDetachVM(env);
	
	return (w+pdjt->argc+1);
}


void pdj_tilde_dsp(t_pdj_tilde *pdjt, t_signal **sp) {
	JNIEnv *env = pdjAttachVM();
	jobject perfReflection = (*env)->CallObjectMethod(env, pdjt->pdj.obj, 
		pdjCaching.MIDMSPObject_dspinit, (jfloat) sp[0]->s_sr, sp[0]->s_n);
	int i;
		
	SHOWEXC;		
	if ( perfReflection == NULL ) {
		post("pdj~: can't bind MSPObject to dsp chain, dsp(MSPSignal[], MSPSignal[]) returned null");
		return;
	}

	pdjt->performer = (*env)->FromReflectedMethod(env, perfReflection);
	JASSERT(pdjt->performer);

	for(i=0;i<pdjt->ins_count;i++) {
		jobject tmp, obj = (*env)->GetObjectArrayElement(env, pdjt->_used_inputs, i);	
		JASSERT(obj);
		if ( pdjt->ins[i] != NULL )
			(*env)->DeleteGlobalRef(env, pdjt->ins[i]);
		tmp = (*env)->GetObjectField(env, obj, pdjCaching.FIDMSPSignal_vec);
		pdjt->ins[i] = (*env)->NewGlobalRef(env, tmp);
		(*env)->DeleteLocalRef(env, tmp);
		JASSERT(pdjt->ins[i]);
	}
	
	for(i=0;i<pdjt->outs_count;i++) {
		jobject tmp, obj = (*env)->GetObjectArrayElement(env, pdjt->_used_outputs, i);
		JASSERT(obj);
		if ( pdjt->outs[i] != NULL )
			(*env)->DeleteGlobalRef(env, pdjt->outs[i]);
		tmp = (*env)->GetObjectField(env, obj, pdjCaching.FIDMSPSignal_vec);
		pdjt->outs[i] = (*env)->NewGlobalRef(env, tmp);
		(*env)->DeleteLocalRef(env, tmp);
		JASSERT(pdjt->outs[i]);		
	}
	pdjDetachVM(env);

	switch(pdjt->argc) {
		case 2:
			// no inlets/outlets == do nothing
			break;
		case 3:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n,
				sp[0]->s_vec);
			break;
		case 4:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n,
				sp[0]->s_vec, sp[1]->s_vec);
			break;
		case 5:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n, 
				sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
			break;
		case 6:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n,
				sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
			break;
		case 7:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n,
				sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, 
				sp[4]->s_vec);
			break;
		case 8:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n, 
				sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, 
				sp[4]->s_vec, sp[5]->s_vec);
			break;
		case 9:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n, 
				sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, 
				sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec);
			break;
		case 10:
			dsp_add(pdj_tilde_perform, pdjt->argc, pdjt, sp[0]->s_n, 
				sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, 
				sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec);
			break;
		default:
			error("pdj~: too much ~ inlets/outlets, go see the source luke");
			break;
	}				
}


void *pdj_tilde_new(t_symbol *s, int argc, t_atom *argv) {
	t_pdj_tilde *pdjt = pdj_new(s, argc, argv);
	JNIEnv *env;
	jobject tmp;

	if ( pdjt == NULL ) 
		return NULL;
		
	env = pdjAttachVM();
	
	/* check if we are really a MSPObject */
	if ( !(*env)->IsInstanceOf(env, pdjt->pdj.obj, pdjCaching.cls_MSPObject) ) {
		error("pdj~: class must be a superclass of MSPObject");
		pdj_free((t_pdj*)pdjt);
		return NULL;
	}

	/* initialize inlets/outlets */	
	tmp = (*env)->GetObjectField(env, pdjt->pdj.obj, pdjCaching.FIDMSPObject_used_inputs);
	JASSERT(tmp);
	pdjt->_used_inputs = (*env)->NewGlobalRef(env, tmp);
	(*env)->DeleteLocalRef(env, tmp);
	
	tmp = (*env)->GetObjectField(env, pdjt->pdj.obj, pdjCaching.FIDMSPObject_used_outputs);
	JASSERT(tmp);
	pdjt->_used_outputs = (*env)->NewGlobalRef(env, tmp);
	(*env)->DeleteLocalRef(env, tmp);
	
	pdjt->ins_count = (*env)->GetArrayLength(env, pdjt->_used_inputs);
	pdjt->outs_count = (*env)->GetArrayLength(env, pdjt->_used_outputs);
	
	pdjDetachVM(env);

	pdjt->ins = malloc(sizeof(jobject) * pdjt->ins_count);
	memset(pdjt->ins, 0, sizeof(jobject) * pdjt->ins_count);
	pdjt->outs = malloc(sizeof(jobject) * pdjt->outs_count);
	memset(pdjt->outs, 0, sizeof(jobject) * pdjt->outs_count);
	
	// pdj object pointer + signal size + nb inlets + nb outlets
	pdjt->argc = 2 + pdjt->ins_count + pdjt->outs_count;
	
	return pdjt;
}


void pdj_tilde_free(t_pdj_tilde *pdjt) {
	JNIEnv *env = pdjAttachVM();
	int i;
	
	(*env)->DeleteGlobalRef(env, pdjt->_used_inputs);
	(*env)->DeleteGlobalRef(env, pdjt->_used_outputs);
	for(i=0;i<pdjt->ins_count;i++) {
		if ( pdjt->ins[i] != NULL )
			(*env)->DeleteGlobalRef(env, pdjt->ins[i]);
	}
	for(i=0;i<pdjt->outs_count;i++) {
		if ( pdjt->outs[i] != NULL ) 
			(*env)->DeleteGlobalRef(env, pdjt->outs[i]);
	}
	pdjDetachVM(env);
	
	free(pdjt->ins);
	free(pdjt->outs);
	
	pdj_free((t_pdj*)pdjt);
}
