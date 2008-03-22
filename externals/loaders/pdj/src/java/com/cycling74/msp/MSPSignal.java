package com.cycling74.msp;

/**
 * Signals representation; signals inlets will copy signal data to this 
 * object.
 */
public class MSPSignal {
    
    /**
     * Tells the number of time this signal is connected. This is always
     * 1 in pure-data.
     */
    public short cc = 1;
    
    /**
     * Tells if this signal is connected. This is always true in pure-data.
     */
    public boolean connected = true;
    
    /**
     * Number of sample in vector.
     */
    public int n;
    
    /**
     * The sampling rate.
     */
    public double sr;
    
    /**
     * The current sample data.
     */
    public float[] vec;
    
    MSPSignal() {
    }
    
    public MSPSignal(float vec[], double sr, int n, short cc) {
        this.vec = vec;
        this.sr = sr;
        this.n = n;
        this.cc = cc;
    }

    /**
     * Returns a copy of this signal but with the same sample buffer. 
     * @return the new signal with the same buffer
     */
    public MSPSignal alias() {
        return new MSPSignal(vec, sr, n, cc);
    }
    
    /**
     * Returns the a duplicated signal object. Also copies the array.
     * @return the new signal with the same value
     */
    public MSPSignal dup() {
        return new MSPSignal((float [])vec.clone(), sr, n, cc);
    }
    
    /**
     * Returns the a duplicated signal object, but the sample vector is
     * re-initialized.
     * @return the new signal with empty value (silence)
     */
    public MSPSignal dupclean() {
        return new MSPSignal(new float [vec.length], sr, n, cc);
    }
    
}
