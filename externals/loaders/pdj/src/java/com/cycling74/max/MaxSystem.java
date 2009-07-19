package com.cycling74.max;

import com.e1.pdj.*;

/**
 * MXJ System utilities.
 */
public class MaxSystem {
	static private PriorityQueue low = new PriorityQueue(Thread.MIN_PRIORITY);	
	
	/**
	 * Shows a message to the pd console
	 * @param message the string to show
	 */
	static native public void post(String message); 
	
	/**
	 * Shows a error message to the pd console
	 * @param message the string to show
	 */
	static native public void error(String message);
	
	/**
	 * Shows a message in the pd console and kill PD afterwards.
	 * @param message the string to show
	 */
	static native public void ouch(String message);
	
	/**
	 * Sends a message to a bound object (IEM object or a receiver)
	 * @param name the destination of the message
	 * @param msg the symbol message ("bang", "float", "list")
	 * @param args the array of Atoms
	 * @return true if successfull
	 */
	static native public boolean sendMessageToBoundObject(String name, String msg, Atom[] args);

	/**
	 * Tries to locate file in pure-data search path.
	 * @param filename of the file to search in path
	 * @return the full path of this file
	 */
	static native public String locateFile(String filename);

	/**
	 * Will schedule the executable to a low priority thread
	 * @param fn the executable
	 */
	static synchronized public void deferLow(Executable fn) {
		low.defer(fn);
	}

	/**
	 * Will schedule the executable to a medium priority thread
	 * @param fn the executable
	 */
	static synchronized public void deferMedium(Executable fn) {
		low.defer(fn);
	}
	
	/**
	 * Returns the user classpath.
	 * @return Array of strings of each entries in the user classpath
	 */
	static public String[] getClassPath() {
		return PDJClassLoader.getCurrentClassPath();
	}

	// implemented but not fully supported...
	static public void defer(Executable fn) {
		fn.execute();
	}
	static public void deferFront(Executable fn) {
		fn.execute();
	}
	
	static public String[] getSystemClassPath() {
		return null;
	}
	
	static boolean inMainThread() {
		return false;
	}
	
	static boolean inMaxThread() {
		return false;
	}
	
	static boolean inTimerThread() {
		return false;
	}
	
	static boolean isOsMacOsX() {
        String osname = System.getProperty("os.name");
        if ( osname.indexOf("OS X") != -1 ) {
            return true;
        }
		return false;
	}
	
	static boolean isOsWindows() {
        String osname = System.getProperty("os.name");
        if ( osname.indexOf("Windows") != -1 ) {
            return true;
        }
		return false;
	}
	
	/**
	 * <b>Not supported in PD</b>
	 */
	static void registerCommandAccelerator(char c) {
	}

	/**
	 * <b>Not supported in PD</b>
	 */
	static void registerCommandAccelerators(char c[]) {
	}

	/**
	 * <b>Not supported in PD</b>
	 */
	static void unRegisterCommandAccelerator(char c) {
	}
	/**
	 * <b>Not supported in PD</b>
	 */	
	static void unRegisterCommandAccelerators(char c[]) {
	}
	
	// not compatible Max methods :
	////////////////////////////////////////////////////////
	/**
	 * <b>Not supported in PD</b>
	 */
	static public boolean isStandAlone() {
		return false;
	}
	
	static public short getMaxVersion() {
		return 0;
	}
	
	static public int[] getMaxVersionInts() {
		int ret[] = new int[3];
		
		ret[0] = 0;
		ret[1] = 8;
		ret[2] = 5;
		
		return ret;
	}

	/**
	 * <b>Not supported in PD</b>
	 */
	static public void hideCursor() {
	}

	/**
	 * <b>Not supported in PD</b>
	 */
	static public void showCursor() {		
	}

	/**
	 * <b>Not supported in PD</b>
	 */
	static public void nextWindowIsModal() {
	}
	
	// constants
	public static String MXJ_VERSION = "pdj 0.8.5";
	
	public static final int PATH_STYLE_COLON		= 2;
	public static final int PATH_STYLE_MAX			= 0;
	public static final int PATH_STYLE_NATIVE		= 1;
	public static final int PATH_STYLE_NATIVE_WIN	= 4;
	public static final int PATH_STYLE_SLASH		= 3;
	public static final int PATH_TYPE_ABSOLUTE		= 1;
	public static final int PATH_TYPE_BOOT			= 3;
	public static final int PATH_TYPE_C74			= 4;
	public static final int PATH_TYPE_IGNORE		= 0;
	public static final int PATH_TYPE_RELATIVE		= 2;	
}
