package com.cycling74.max;

/**
 * Used to delay the execution of a block of code. Java implementation 
 * of a pdclock.
 * <p><blockquote><pre>
 * 
 * import com.cycling74.max.*;
 * 
 * class clocktest extends MaxObject implements Executable {
 *     MaxClock clock;
 *     float value; 
 * 
 *     public clocktest() {
 *         clock = new MaxClock(this); 
 *     }
 *     
 *     public void inlet(float f) {
 *         value = f;
 *         // ask to call execute after 250ms
 *         clock.delay(250);
 *     }
 * 
 *     // this is called after 250ms
 *     public void execute() {
 *         outlet(0, value)
 *     }
 * }
 * </pre></blockquote></p>
 */
public class MaxClock {
	private Executable exec;
	private long _clock_ptr;
	
	/**
	 * Creates a pdclock without an executable.
	 */
	public MaxClock() {
		create_clock();
	}

	/**
	 * Creates a pdclock with an executable <code>e</code>.
	 * @param e the executable to execute when the clock will be triggerd.
	 */
	public MaxClock(Executable e) {
		create_clock();
		exec = e;
	}

	/**
	 * Creates a pdclock with a specific method on a object. 
	 * @param o the object that holds the method
	 * @param methodName the name of the method to execute when the clock
	 * will be triggerd.
	 */
	public MaxClock(Object o, String methodName) {
		create_clock();
		exec = new Callback(o, methodName);
	}

	/**
	 * Returns the Executable for this clock.
	 * @return the Executable for this clock
	 */
	public Executable getExecutable() {
		return exec;
	}
	
	/**
	 * Set the Executable for this clock.
	 * @param e the Executable to call for this clock
	 */
	public void setExecutable(Executable e) {
		exec = e;
	}

	/**
	 * The method to override if no Executable is provided.
	 */
	public void tick() {
		exec.execute();
	}
	
	protected void finalize() throws Throwable {
		release();
		super.finalize();
	}
	
	/**
	 * Returns pure-data time in milliseconds.
	 * @return pure-data time in milliseconds
	 */
	public static native double getTime();
	
	/**
	 * Time to wait until next tick.
	 * @param time in miliseconds
	 */
	public native void delay(double time);
	
	/**
	 * Release the clock from pure-data. The clock becomes unless afterwards. 
	 */
	public native void release();
	
	/**
	 * Cancels the last delay call.
	 */
	public native void unset();
	
	private native void create_clock();
}
