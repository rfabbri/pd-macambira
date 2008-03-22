package com.cycling74.msp;

import java.lang.reflect.Method;

/**
 * Process signals with a single method. Like MSPObject except that
 * the performer method will always be "perform" (and you override it).
 */
public abstract class MSPPerformer extends MSPObject implements MSPPerformable {

    /**
     * Called by PD with the dsp message is sended. This method will then
     * call dspsetup and map the method "perform" has the performer method. 
     * You don't have to override this method, use dspsetup if initialization
     * needs to be done before the signals starts processing.
     */
    public Method dsp(MSPSignal[] in, MSPSignal[] out) {
        dspsetup(in, out);
        return getPerformMethod("perform");
    }
    
    /**
     * Initialize the dsp state of the MSPPerformable. Override this if 
     * you have to initalize something before the signal starts 
     * processing.
     */
    public void dspsetup(MSPSignal[] in, MSPSignal[] out) {
    }

    /**
     * Process signal inlets/outlets. You must override this since it is
     * the main processing method.
     */
    public abstract void perform(MSPSignal[] in, MSPSignal[] out);

}
