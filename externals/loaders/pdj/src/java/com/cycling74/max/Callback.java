package com.cycling74.max;

import java.lang.reflect.*;

/**
 * Used to transform a java method into an Executable object. This
 * simplify the job of always implementing the executable 
 * interface on all your objects since you can define it dynamically
 * with this class.
 * <p><blockquote><pre>
 * 
 * class Myclass  {
 *    public void doit() {
 *		do_something_fun()
 *    }
 * }
 * 
 * ...
 * 
 *      Myclass myclass = new Myclass();
 *      Executable e = new Callback(myclass, "doit");
 *      MaxClock clock = new MaxClock(e);
 * 
 * </pre></blockquote></p>
 */
public class Callback implements Executable {
	private Method method;
	private String methodName;
	private Object obj;
	private Object args[];
	
	/**
	 * Will call method <i>methodName</i> with no argument by using execute()
	 * @param obj the object with the method
	 * @param methodName the name of the method
	 */
	public Callback(Object obj, String methodName) {
		this(obj, methodName, null, null);
	}

	/**
	 * Will call method <i>methodName</i> with a int by using execute()
	 * @param obj the object with the method
	 * @param methodName the name of the method
	 * @param i the int value
	 */
	public Callback(Object obj, String methodName, int i) {
		this(obj, methodName, new Object[] { new Integer(i) }, new Class[] { Integer.TYPE });
	}

	/**
	 * Will call method <i>methodName</i> with a float by using execute()
	 * @param obj the object with the method
	 * @param methodName the name of the method
	 * @param f the float value
	 */
	public Callback(Object obj, String methodName, float f) {
		this(obj, methodName, new Object[] { new Float(f) }, new Class[] { Float.TYPE });
	}
	
	/**
	 * Will call method <i>methodName</i> with a Stringt by using execute()
	 * @param obj the object with the method
	 * @param methodName the name of the method
	 * @param str the string value
	 */
	public Callback(Object obj, String methodName, String str) {
		this(obj, methodName, new Object[] { str });
	}
	
	/**
	 * Will call method <i>methodName</i> with a boolean by using execute()
	 * @param obj the object with the method
	 * @param methodName the name of the method
	 * @param flag the boolean value
	 */
	public Callback(Object obj, String methodName, boolean flag) {
		this(obj, methodName, new Object[] { flag ? Boolean.TRUE : Boolean.FALSE });
	}

	/**
	 * Will call method <i>methodName</i> with multiple arguments by using execute()
	 * @param obj the object with the method
	 * @param methodName the name of the method
	 * @param params argument to pass to the method
	 */
	public Callback(Object obj, String methodName, Object params[]) {
		this(obj, methodName, params, buildClasses(params));
	}
	
	/**
	 * Will call method <i>methodName</i> with multiple arguments (typed) by using execute()
	 * @param obj the object with the method
	 * @param methodName the name of the method
	 * @param params argument to pass to the method
	 * @param params_types the type of arguments
	 */
	public Callback(Object obj, String methodName, Object params[], Class params_types[]) {
		try {
			if ( params == null ) {
				method = obj.getClass().getDeclaredMethod(methodName, null);
			} else {
				method = obj.getClass().getDeclaredMethod(methodName, params_types);
			}
			this.obj = obj;
			this.methodName = methodName;
		} catch ( NoSuchMethodException e ) {
			MaxSystem.post("pdj: unable to find method: " + methodName + ", "+ e);
		}
	}
	
	private static Class[] buildClasses(Object params[]) {
		Class clz[] = new Class[params.length];
		
		for(int i=0;i<params.length;i++) {
			clz[i] = params[i].getClass();
		}
		
		return clz;
	}
	
	/**
	 * Execute the method with arguments specified at constructor
	 */
	public void execute() {
		if ( obj == null ) {
			throw new Error("pdj: this Callback has never been initialised");
		}
		
		try {
			method.invoke(obj, args);
		} catch (IllegalArgumentException e) {
		    e.printStackTrace();
			MaxSystem.error("pdj: IllegalArgumentException:" + e);
		} catch (IllegalAccessException e) {
			MaxSystem.error("pdj: IllegalAccessException:" + e);
		} catch (InvocationTargetException e) {
			MaxSystem.error("pdj: InvocationTargetException:" + e);
		}
	}
	
	/**
	 * Returns the argument that will be used when the execute method
	 * will be invoke.
	 * @return the array of arguments
	 */
	public Object[] getArgs() {
		return (Object[]) args[0];
	}
	
	/**
	 * Returns the object used to issue the call
	 * @return the object to use with execute
	 */
	public Object getObject() {
		return obj;
	}
	
	/**
	 * Returns the Method that will be used with the execute call
	 * @return the method given by reflection
	 */
	public Method getMethod() {
		return method;
	}
	
	/**
	 * Returns the method name invoked on object with the execute call
	 * @return the method name
	 */
	public String getMethodName() {
		return methodName;
	}
	
	/**
	 * Sets int argument to method
	 * @param i int value
	 */
	public void setArgs(int i) {
		setSubArgs(new Object[] { new Integer(i) });
	}

	/**
	 * Sets float argument to method
	 * @param f float argument
	 */
	public void setArgs(float f) {
	    setSubArgs(new Object[] { new Float(f) });
	}
	
	/**
	 * Sets String argument to method
	 * @param value int value
	 */
	public void setArgs(String value) {
		setSubArgs(new Object[] { value });
	}
	
	/**
	 * Sets boolean argument to method
	 * @param flag boolean value
	 */
	public void setArgs(boolean flag) {
		setSubArgs(new Object[] { flag ? Boolean.TRUE : Boolean.FALSE });
	}
	
	/**
	 * Set the argument for the execute call
	 * @param args the array object to pass to the method
	 */
	public void setArgs(Object args[]) {
		setSubArgs((Object[]) args.clone());
	}
	
	/**
	 * Fix for <1.5 method.invoke.
	 * @param args
	 */
	private void setSubArgs(Object args[]) {
	    this.args = new Object[1];
	    this.args[0] = args;
	}
}
