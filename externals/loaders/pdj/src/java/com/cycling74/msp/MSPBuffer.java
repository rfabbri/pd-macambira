package com.cycling74.msp;

/**
 * Used to get or set pd array content. Please note that the channel parameter
 * is added in the API to match Max/MSP MSPBuffer signature.
 */
public class MSPBuffer {
    /**
     * Returns the array content.
     * @param name the array name
     * @return the array contents
     */
    public static float[] peek(String name) {
        return getArray(name, 0, -1);
    }
    
    /**
     * Returns the array content.
     * @param name the array name
     * @param channel <i>not used in pd</i>
     * @return the array contents
     */
    public static float[] peek(String name, int channel) {
        return getArray(name, 0, -1);
    }

    /**
     * Returns the array content.
     * @param name the array name
     * @param channel <i>not used in pd</i>
     * @param start the start index of the array
     * @param length the size of the array to return
     * @return the array contents
     */
    public static float[] peek(String name, int channel, long start, long length) {
        return getArray(name, start, length);
    }

    /**
     * Returns the array content value at a specific position.
     * @param name the array name
     * @param channel <i>not used in pd</i>
     * @param index the start index of the array
     * @return the value stored at index <code>start</code>
     */
    public static float peek(String name, int channel, long index) {
        float ret[] = getArray(name, index, 1);
        if ( ret == null ) 
            return 0;
        return ret[0];
    }
    
    /**
     * Sets array content.
     * @param name the array name
     * @param values the array to set
     */
    public static void poke(String name, float values[]) {
        setArray(name, 0, values);
    }

    /**
     * Sets array content.
     * @param name the array name
     * @param channel <i>not used in pd</i>
     * @param values the array to set
     */
    public static void poke(String name, int channel, float values[]) {
        setArray(name, 0, values);
    }

    /**
     * Sets array content.
     * @param name the array name
     * @param channel <i>not used in pd</i>
     * @param start the start index of the array
     * @param values the array to set
     */
    public static void poke(String name, int channel, long start, float values[]) {
        setArray(name, start, values);
    }

    /**
     * Set a value in a array.
     * @param name the array name
     * @param channel <i>not used in pd</i>
     * @param index the index in the array to set
     * @param value the value to set in the array
     */
    public static void poke(String name, int channel, long index, float value) {
        float content[] = new float[1];
        content[0] = value;
        setArray(name, index, content);
    }
    
    /**
     * Sets the array size.
     * @param name the array name
     * @param numchannel <i>not used in pd</i>
     * @param size the new array size;
     */
    public static native void setSize(String name, int numchannel, long size);
    
	/** 
	 * Returns the array size
	 * @param name the array name
	 * @return the array size or -1 if not found
	 */
    public static native long getSize(String name);
    
    private static native float[] getArray(String name, long from, long size);
    private static native void setArray(String name, long from, float[]content);
    private MSPBuffer() {}

	/**
	 * Return the number of channel for this array. Useless in PD cause there
     * is no channels in a array.
	 * @param name array name.
	 * @return always returns 1 on pd; unless the name is not defined.
	 */
	public static int getChannel(String name) {
		// resolv the name
		if ( getSize(name) != -1 )
			return 1;
		return -1;
	}
}
