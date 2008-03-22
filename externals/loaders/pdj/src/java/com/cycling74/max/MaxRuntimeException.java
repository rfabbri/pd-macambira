package com.cycling74.max;

/**
 * API based runtime exception with Max/PDJ.
 */
public class MaxRuntimeException extends RuntimeException {

    public MaxRuntimeException() {
    }
    
    public MaxRuntimeException(String msg) {
        super(msg);
    }

	public MaxRuntimeException(Exception e) {
		super(e);
	}

}
