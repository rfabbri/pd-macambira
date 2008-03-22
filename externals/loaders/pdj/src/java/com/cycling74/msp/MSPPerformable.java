package com.cycling74.msp;

/**
 * This interface is used to chain Java dsp objects since they don't 
 * have to be MSPObject to process signals.
 */
public interface MSPPerformable {
    /**
     * @see MSPPerformer.dspsetup(MSPSignal in[], MSPSignal out[])
     */
    public void dspsetup(MSPSignal in[], MSPSignal out[]);
    
    /**
     * @see MSPPerformer.perform(MSPSignal in[], MSPSignal out[]);
     */
    public void perform(MSPSignal in[], MSPSignal out[]);
}
