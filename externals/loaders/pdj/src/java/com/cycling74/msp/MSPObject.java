package com.cycling74.msp;

import java.lang.reflect.Method;

import com.cycling74.max.DataTypes;
import com.cycling74.max.MaxObject;

/**
 * Main object to extend to use dsp with pd. You will need to
 * use the pdj~ external if you want to process signals. Before 
 * the dsp starts processing, the method dsp will be called and it must
 * return the performer method that will process each dsp cycles.
 * 
 * <p>Here is a basic guideline for using the MSPObject:</p>
 * <p><blockquote><pre>
 * import com.cycling74.max.*;
 * import com.cycling74.msp.*;
 * import java.lang.reflection.Method;
 * 
 * public class panner extends MSPObject {
 *   float left = 1, right = 1;
 *   
 *   public panner() {
 *       declareInlets( new int[] { SIGNAL, DataTypes.ANYTHING } );
 *       declareOutlets( new int[] { SIGNAL, SIGNAL } );
 *   }
 *   
 *   // From 0..127
 *   public void inlet(float val) {
 *       if ( val > 64 ) {
 *           right = 1;
 *           left = ((127-val) / 64);
 *       } else {
 *           left = 1;
 *           right = val / 64;
 *       }
 *   }
 *  
 *   public Method dsp(MSPSignal[] ins, MSPSignal[] outs) {
 *       return getPerformMethod("perform");
 *   }
 *
 *   public void perform(MSPSignal[] ins, MSPSignal[] outs) {
 *       for (int i=0;i&lt;ins[0].n;i++) {
 *           outs[0].vec[i] = ins[0].vec[i] * left;
 *           outs[1].vec[i] = ins[0].vec[i] * right;
 *       }
 *   }
 * }
 * </p></blockquote></pre>
 */
public abstract class MSPObject extends MaxObject {
    /**
     * Use this value to indentify a signal intlet/outlet to 
     * the method declareInlet/declareOutlet.
     */
    public final static int SIGNAL = 32;
    
    /**
     * Initialize the dsp state. From the numbers of input/output this
     * method must return a performing method.
     * @param ins input signals
     * @param outs output signals
     * @return the method to execute at each dsp cycle
     */
    public abstract Method dsp(MSPSignal[] ins, MSPSignal[] outs);
    
    /**
     * Quicky method to be used with dsp(MSPSignal[], MSPSignal[]). It
     * will return the method name <code>methodName</code> with signature
     * (MSPSignal[], MSPSignal[]) in the current class. 
     * @param methodName the name of the method in the current class
     * @return the method reflection
     */
    protected Method getPerformMethod(String methodName) {
        try {
            Method m = getClass().getDeclaredMethod(methodName, new Class[] {
                    MSP_SIGNAL_ARRAY_CLZ, MSP_SIGNAL_ARRAY_CLZ });
            return m;
        } catch ( NoSuchMethodException e ) {
            error("pdj~: method: " + methodName + " not found in class in:" + getClass().toString());
        }
        return null;
    }
        
    /**
     * Force to copy of each MSPBuffer when performer is called. Right now,
     * on pdj the MSPBuffer is always copied.
     * @param copyBuffer true if you need to copyBuffer
     */
    protected void setNoInPlace(boolean copyBuffer) {
        //this.copyBuffer = copyBuffer;
    }
    
    /**
     * This method is called when the dsp is start/stop.
     * <b>NOT USED ON PD.</b>
     * @param dspRunning true if the dsp is running
     */
    protected void dspstate(boolean dspRunning) {
    }
    
    
    /**
     * Declare the inlets used by this object. Use MSPObject.SIGNAL
     * to define a signal inlet. <b>Note:</b> any signal inlet won't
     * be able to process any atom messages.
     */
    protected void declareInlets(int[] types) {
        int i, inlets = 0;
        for (i=0;i<types.length;i++) {
            if ( types[i] == SIGNAL )
                inlets++;
        }
        _used_inputs = new MSPSignal[inlets];
        for(i=0;i<_used_inputs.length;i++) {
            _used_inputs[i] = new MSPSignal();
        }
        super.declareInlets(types);
    }
    
    /**
     * Declare the outlets used by this object. Use MSPObject.SIGNAL
     * to define a signal outlet.
     */
    protected void declareOutlets(int[] types) {
        int i, outlets = 0;
        for (i=0;i<types.length;i++) {
            if ( types[i] == SIGNAL )
                outlets++;
        }
        _used_outputs = new MSPSignal[outlets];
        for(i=0;i<_used_outputs.length;i++) {
            _used_outputs[i] = new MSPSignal();
        }
        super.declareOutlets(types);
    }

    public static final Class MSP_SIGNAL_ARRAY_CLZ = (new MSPSignal[0]).getClass();    
    
    private MSPSignal[] _used_inputs = new MSPSignal[0];
    private MSPSignal[] _used_outputs = new MSPSignal[0];
    
    private Method _dspinit(float sr, int vector_size) {
        int i;
        for(i=0;i<_used_inputs.length;i++) {
            _used_inputs[i].n = vector_size;
            _used_inputs[i].vec = new float[vector_size];
            _used_inputs[i].sr = sr;
        }
        for(i=0;i<_used_outputs.length;i++) {
            _used_outputs[i].n = vector_size;
            _used_outputs[i].vec = new float[vector_size];
            _used_outputs[i].sr = sr;
        }
        dspstate(true);
        return dsp(_used_inputs, _used_outputs);
    }
    
    private void _emptyPerformer(MSPSignal[] ins, MSPSignal[] outs) {
    }
}

