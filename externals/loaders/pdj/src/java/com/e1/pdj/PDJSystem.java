package com.e1.pdj;

import com.cycling74.max.MaxSystem;

import java.awt.Toolkit;
import java.io.*;
import java.awt.GraphicsEnvironment;

/**
 * Startup class for pdj.
 */
public class PDJSystem {
	private static int loaded = 0;

	public static PrintStream err;

	public static PrintStream out;

	/**
	  * Called by the pdj external when the JVM is initializing.
	  */
	public static void _init_system() {
		if ( loaded == 1 )
			return;
		javainit();
		initIO();
	}

	static void resolvRtJar() {
		char ps = File.separatorChar;
		String systemCpJar = System.getProperty("pdj.JAVA_HOME");
		if ( systemCpJar == null ) {
			systemCpJar = System.getProperty("JAVA_HOME");
			if ( systemCpJar == null ) {
				systemCpJar = System.getenv("JAVA_HOME");
			}
		}
        
		System.setProperty("pdj.JAVA_HOME", systemCpJar);
		GenericCompiler.rtJar = systemCpJar + ps + "jre" + ps + "lib" + ps + "rt.jar" + File.pathSeparator;
	}


	static void javainit() {
		// this is a hack to be sure that statics of MaxSystem are loaded
		// before everything
		Class cls = MaxSystem.class;
		
		String osname = System.getProperty("os.name");
		if ( osname.indexOf("OS X") != -1 ) {
		    try {
		        if (System.getenv("PDJ_USE_AWT") != null) {
		            GraphicsEnvironment.getLocalGraphicsEnvironment().getScreenDevices();
		            Toolkit.getDefaultToolkit();
		        }
		    } catch (Error e) {
		        // on java 1.4, this will throw an error, we simply ignore AWT with 1.4
		    }
            GenericCompiler.rtJar = "/System/Library/Frameworks/JavaVM.framework/Classes/classes.jar:";
            loaded = 1;
            return;
        }
		
		loaded = 1;
		resolvRtJar();
	}

	static boolean redirectIO() {
		String prop = System.getProperty("pdj.redirect-pdio");

		if ( prop == null )
			return true;

		if ( prop.charAt(0) == '0' )
			return false;

		if ( prop.equals("false") )
			return false;

		return true;
	}

	static void initIO() {
		if ( redirectIO() ) {
			if ( System.getProperty("os.name").indexOf("Windows") == -1 ) {
				out = new PrintStream(new ConsoleStream(), true);
				err = new PrintStream(new ConsoleStream(), true);
			} else {
				out = new PrintStream(new ConsoleStreamWin32(), true);
				err = new PrintStream(new ConsoleStreamWin32(), true);
			}
			System.setOut(out);
			System.setErr(err);
		} else {
			out = System.out;
			err = System.err;
		}
	}
    
    public static boolean isSystemPropertyTrue(String name) {
        String value = System.getProperty(name);
        
        if ( value == null ) 
            return false;
        
        if ( value.toLowerCase().equals("true") ) 
            return true;
        
        if ( value.equals("1") )
            return true;
        
        return false;
    }
}
