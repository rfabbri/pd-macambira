package com.cycling74.max;

/**
 * Defines an executable object. An object that implements a
 * Executable interface can later be used with a MaxClock or a
 * MaxSystem.deferLow for example since it defines the 
 * execute method. 
 */
public interface Executable {
	/**
	 * The method to execute.
	 */
	public void execute();
}
