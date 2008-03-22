package com.cycling74.max;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;

import com.e1.pdj.*;
import java.util.*;
import com.cycling74.msp.MSPObject;

/**
 * Main object to extend to use with pd. The name of this class will
 * reflect the name of the pdj object when it is instanciated.
 * <p>Here is a basic guideline for using the MaxObject:</p>
 * <p><blockquote><pre>
 * 
 * import com.cycling74.max.*;
 *
 * public class example extends MaxObject {
 * 
 *    // called when arguments are used in object creation
 *    public example(Atom args[]) {
 *        for(int i=0;i&#60args.length;i++) {
 *           post("args:" + args[i].toString());
 *        }
 *    }
 * 
 *    // this method will be called when a float is sended to the object
 *    public inlet(float f) {
 *        post("hello float:" + f);
 *        outlet(0, f);
 *    }
 * 
 *    // this method will be called when symbol callme is sended to the object
 *    public callme(Atom args[]) {
 *        post("hello object:" + args[0]);
 *        outlet(0, args[0]);
 *    }
 * 
 *    // this method will be called when bang is sended to the object
 *    public bang() {
 *        post("hello bang");
 *        outletBang(0);
 *    }
 * }
 * </pre></blockquote></p>
 * <p>Compile this class by adding pdj.jar on the classpath and put the
 * example.class in the <i>classes</i> directory found in your pdj home. 
 * You can also edit directly this class in the <i>classes</i> directory
 * and pdj will try to compile it for you.</p>
 */
public class MaxObject {
    /**
     * Native C pdj object pointer.
     */
	private long _pdobj_ptr;
    
    /**
     * Native C inlet pointers.
     */
	private long _outlets_ptr[];
    
    /**
     * Native C outlets pointers.
     */
	private long _inlets_ptr[];
    
    /**
     * The last inlet that received a message.
     */
	private int _activity_inlet;
	private String name;
	private HashMap attributes = new HashMap();
	private boolean toCreateInfoOutlet = true;
	private MaxPatcher patch = new MaxPatcher();
    
	// useless statics....
	/**
	 * Use this to declare that your object has no inlets.
	 */
	public static final int[] NO_INLETS = {};
	
	/**
	 * Use this to declare that your object has no outlets.
	 */
	public static final int[] NO_OUTLETS = {};
	
	/**
	 * Defined in the original MXJ API; don't know what it is used for...
	 */
	public static final String[] EMPTY_STRING_ARRAY = {};
	
	/** 
	 * Default constructor for MaxObject. You can add an constructor that
	 * supports Atom[] to read object creation arguments. It is not defined 
	 * in this API since it does not to force a 'super()' call in all the 
	 * extended class.
	 */
	protected MaxObject() {
        synchronized (MaxObject.class) {
            _pdobj_ptr = popPdjPointer();
        }
		name = this.getClass().getName();
        patch.patchPath = getPatchPath();
	}

	/**
	 * Will show an error message in the pure-data console.
	 * @param message the string to show
	 */
	public static void error(String message) {
		MaxSystem.error(message);
	}

	/**
	 * Will show an info message in the pure-data console.
	 * @param message the string to show
	 */
	public static void post(String message) {
		MaxSystem.post(message);
	}

	/**
	 * Will show an error message in the pure-data console and crash
	 * pure-data. Do not use if you have no friend left.
	 * @param message the message to show when you make pd crash
	 */
	public static void ouch(String message) {
		MaxSystem.ouch(message);
	}
	
	/**
	 * Show the exeception in the pure-data console.
	 * @param t the exception it self
	 */
	public static void showException(Throwable t) {
		t.printStackTrace(PDJSystem.err);
	}
	
	/**
	 * Show the exception in the pure-data console with a message.
	 * @param message the message that comes with the exception
	 * @param t the exception it self
	 */
	public static void showException(String message, Throwable t) {
		PDJSystem.err.println(message);
		t.printStackTrace(PDJSystem.err);
	}
	
	/**
	 * Returns the object name.
	 * @return the pdj object name
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * Sets the object name.
	 * @param name pdj object name
	 */
	public void setName(String name) {
		this.name = name;
	}
	
	/**
	 * This method is called when the object is deleted by the user.
	 */
	public void notifyDeleted() {
	}

	/**
	 * Bail will throw an exception upon object instanciation. This is usefull
	 * if you have a missing requirement and you want to cancel object creation.
	 * @param errormsg the error message to show
	 */
	protected static void bail(String errormsg) {
		throw new PDJError(errormsg);
	}
	
	/**
	 * Declare the inlets used by this object.
  	 * @see com.cycling74.max.DataTypes
	 * @param types the type of message that this inlet will use.
	 */
	protected void declareInlets(int[] types) {
		if ( _inlets_ptr != null ) {
			throw new IllegalStateException(name + ": inlets already defined");
		}
		_inlets_ptr = new long[types.length];
		
        int pos = 0;
		for(int i=0; i<types.length; i++) {
            _inlets_ptr[pos++] = newInlet(types[i]);
		}
	}
		
	/**
	 * Declare the outlets used by this object. 
	 * @see com.cycling74.max.DataTypes
	 * @param types the type of message that this inlet will use.
	 */
	protected void declareOutlets(int[] types) {
		if ( _outlets_ptr != null ) {
			throw new IllegalStateException(name + ": outlets already defined");
		}
		_outlets_ptr = new long[types.length];
        
        int pos = 0;
		for(int i=0; i<types.length; i++) {
            long ret = newOutlet(types[i]);
            if ( ret != 0 )
                _outlets_ptr[pos++] = ret; 
		}
	}
	
	/**
	 * Used to defined typed input and output for this object. Strings are
	 * used to define the type of inlet/outlet. For example, "ffi" will create
	 * a float/float/integer inlet/outlet. 'f' denotes a float, 'i' a int, 'm'
	 * a message, 'l' a list, 's' for a signal. Anything else will be considered 
     * as a outlet of type anything. <b>PD doesn't type all data yet, so every 
     * inlet/outlet are typed as anything; except for signals.</b> 
	 * @param ins list of inlet to create
	 * @param outs list of outlet to create
	 */
	protected void declareTypedIO(String ins, String outs) {
		if ( _inlets_ptr != null || _outlets_ptr != null ) {
			throw new IllegalStateException(name + ": inlets/outles already defined.");
		}
		
		int[] in_type = new int[ins.length()];
		int[] out_type = new int[outs.length()];
		int i;

		for (i=0;i<in_type.length;i++) {
            if ( ins.charAt(i) == 's' )
                in_type[i] = MSPObject.SIGNAL;
            else
                in_type[i] = DataTypes.ANYTHING;
		}
		declareInlets(in_type);

		for (i=0;i<out_type.length;i++) {
            if ( ins.charAt(i) == 's' )
                in_type[i] = MSPObject.SIGNAL;
            else
                in_type[i] = DataTypes.ANYTHING;
		}
		declareOutlets(out_type);
	}
	
	/**
	 * Quickie method for declaring both inlet and outlet that will use
	 * any type of message (DataTypes.ANYTHING)
	 * @param in the number of inlet to create
	 * @param out the number of outlet to create
	 */
	protected void declareIO(int in, int out) {
		if ( _inlets_ptr != null || _outlets_ptr != null ) {
			throw new IllegalStateException(name + ": inlets/outlets already defined.");
		}
		
		_inlets_ptr = new long[in];
		_outlets_ptr = new long[out];
		
		for(int i=0; i<in; i++) {
			_inlets_ptr[i] = newInlet(DataTypes.ANYTHING);
		}
		
		for(int i=0; i<out; i++) {
			_outlets_ptr[i] = newOutlet(DataTypes.ANYTHING);
		}
	}

	/**
	 * Creates a attribute with default setter and getter. The name used is
	 * actually a java field in the current class. Later, you can send 'a 10' 
	 * and pdj will set 10 to your java field named 'a'. You can also 
	 * use 'get a' that will return its value to the info outlet. The info
	 * outlet is the last outlet of your object.
	 * @param name name of the java field in this class to map.
	 */
	protected void declareAttribute(String name) {
        Attribute attr = new Attribute(this, name, null, null);
        attributes.put(name, attr);
	}

    /**
     * Creates a attribute with default getter no setter. The name used is
     * actually a java field in the current class. 
     * @param name name of the java field in this class to map.
     */
    protected void declareReadOnlyAttribute(String name) {
        Attribute attr = new Attribute(this, name, null, null);
        attr.readOnly = true;
        attributes.put(name, attr);
    }
    
    
	void declareAttribute(String name, String getter, String setter) {
		Attribute attr = new Attribute(this, name, getter, setter);
		attributes.put(name, attr);
	}
    
    void declareReadOnlyAttribute(String name, String getter) {
        Attribute attr = new Attribute(this, name, getter, null);
        attr.readOnly = true;
        attributes.put(name, attr);
    }

	/**
	 * Tells the constructor to create a info outlet. The info outlet is used
	 * by attributes when the get method is used. The return value of the 'get'
	 * will be return to this outlet. The info outlet is always the last 
	 * outlet of the object. By default, the info outlet is always created.
	 * @param flag false to prevent the creation of the info outlet
	 */
	protected void createInfoOutlet(boolean flag) {
		toCreateInfoOutlet = flag;
	}
	
	/**
	 * Returns the type the inlet at index 'idx'. <i>Not fully implemented,
	 * returns always DataTypes.ANYTHING with pdj</i>
	 * @param idx the outlet position
	 * @return the DataType of the inlet
	 */
	public int getInletType(int idx) {
		return DataTypes.ANYTHING;
	}
	
	/**
	 * Returns the type of the outlet at index 'idx'. <i>Not fully implemented,
	 * returns always DataTypes.ANYTHING with pdj</i>
	 * @param idx the outlet position
	 * @return the DataType of the outlet
	 */
	public int getOutletType(int idx) {
		return DataTypes.ANYTHING;
	}
	
	/**
	 * Returns the number of inlets declared.
	 * @return the number of inlets used by this object
	 */
	public int getNumInlets() {
		return _inlets_ptr.length;
	}

	/**
	 * Returns the number of outlets declared.
	 * @return the number of outlets used by this object
	 */
	public int getNumOutlets() {
		return _outlets_ptr.length;
	}
	
	/**
	 * Called by PD when the current patch issue a loadbang message.
	 */
	protected void loadbang() {
	}

	/**
	 * Returns the index of the inlet that has just received a message.
	 * @return the index of the inlet
	 */
	protected int getInlet() {
		return _activity_inlet;
	}
	
    /**
     * Returns the index of the info outlet
     */
    public int getInfoIdx() {
        if ( !toCreateInfoOutlet ) {
            return -1;
        }
        return _outlets_ptr.length-1;
    }
    
	/**
	 * Sends a bang to outlet x.
	 * @param outlet the outlet number to use
	 * @return always true, since PD API returns void
	 */
	public final boolean outletBang(int outlet) {
		doOutletBang(_outlets_ptr[outlet]);
		return true;
	}
	
	/**
	 * Sends a float to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the float value
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, float value) {
		doOutletFloat(_outlets_ptr[outlet], value);
		return true;
	}

	/**
	 * Sends floats to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of float to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, float value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list", Atom.newAtom(value));
		return true;
	}
	
	/**
	 * Sends a symbol to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the symbol
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, String value) {
		doOutletSymbol(_outlets_ptr[outlet], value);
		return true;
	}

	/**
	 * Sends symbols to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of symbol to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, String value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list", Atom.newAtom(value));
		return true;
	}
	
	/**
	 * Sends a byte to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the byte value
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, byte value) {
		return outlet(outlet, (float) value);
	}

	/**
	 * Sends byte to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of byte to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, byte value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list", Atom.newAtom(value));
		return true;
	}
	
	/**
	 * Sends a char to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the char value
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, char value) {
		return outlet(outlet, (float) value);
	}

	/**
	 * Sends char to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of char to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, char value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list", Atom.newAtom(value));
		return true;
	}
	
	/**
	 * Sends a short to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the short value
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, short value) {
		return outlet(outlet, (float) value);
	}
	
	/**
	 * Sends shorts to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of short to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, short value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list" , Atom.newAtom(value));
		return true;
	}
	
	/**
	 * Sends a int to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the int value
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, int value) {
		return outlet(outlet, (float) value);
	}

	/**
	 * Sends ints to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of int to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, int value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list", Atom.newAtom(value));
		return true;
	}
	
	/**
	 * Sends a long to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the long value
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, long value) {
		return outlet(outlet, (float) value);
	}
	
	/**
	 * Sends longs to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of longs to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, long value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list", Atom.newAtom(value));
		return true;
	}
	
	/**
	 * Sends a double to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the double value
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, double value) {
		return outlet(outlet, (float) value);
	}
	
	/**
	 * Sends doubles to outlet x.
	 * @param outlet the outlet number to use
	 * @param value the array of double to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, double value[]) {
		doOutletAnything(_outlets_ptr[outlet], "list" , Atom.newAtom(value));
		return true;
	}

	/**
	 * Sends message with argument to outlet x.
	 * @param outlet the outlet number to use
	 * @param message the message symbol name
	 * @param value the arguments
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, String message, Atom[] value) {
		doOutletAnything(_outlets_ptr[outlet], message, value);
		return true;
	}

	/**
	 * Sends atom value to outlet x. If it is a int/float, it will call
	 * <code>outlet(int, float)</code> otherwise <code>outlet(int, String)</code>
	 * @param outlet the outlet number to use
	 * @param value the atom value to send
	 * @return always true, since PD API returns void
	 */
	public final boolean outlet(int outlet, Atom value) {
		if ( value.isFloat() || value.isInt() ) {
			doOutletFloat(_outlets_ptr[outlet], value.toFloat());
		} else {
			doOutletSymbol(_outlets_ptr[outlet], value.toString());
		}
		return true;
	}
	
	/**
	 * Sends atoms to outlet x. If the array contains only one item, 
	 * <code>outlet(outlet, Atom value)</code> will be called. If the
	 * first element of the array is a float/int, list will be 
	 * appended to the message. Otherwise, the first atom will be the
	 * message and the rest of it the arguments.
	 * @param outlet the outlet number to use
	 * @param value the arguments
	 * @return true or false if atom[] is empty
	 */
	public final boolean outlet(int outlet, Atom[] value) {
		if ( value.length == 0 )
			return false;
		
		if ( value.length == 1 ) {
			outlet(outlet, value[0]);
		} else {
			doOutletAnything(_outlets_ptr[outlet], null, value);
		}
		return true;
	}
	
	// user methods 
	/////////////////////////////////////////////////////////////
	/**
	 * Called by PD when pdj receives a bang from an inlet. Use 
	 * <code>getInlet()</code> to know which inlet has received the message.
	 */
	protected void bang() {
	}

	/**
	 * This will be called if pd sends a float and the float method is not 
	 * overridden. Use <code>getInlet()</code> to know which inlet has 
	 * received the message.
	 * @param i int value
	 */
	protected void inlet(int i) {		
	}
	
	/**
	 * Called by PD when pdj receives a float from an inlet. Use 
	 * <code>getInlet()</code> to know which inlet has received the message.
	 * @param f float value received from the inlet
	 */
	protected void inlet(float f) {
	}
	
	/**
	 * Called by PD when pdj receives a list of atoms. Use 
	 * <code>getInlet()</code> to know which inlet has received the message.
	 * @param args the list
	 */
	protected void list(Atom args[]) {
	}
	
	/**
	 * Called by PD when pdj receives an un-overriden method. If you need
     * to catch all messages, override this method. Use <code>getInlet()</code> 
     * to know which inlet has received the message.
	 * @param symbol first atom symbol representation
	 * @param args the arguments of the message
	 */
	protected void anything(String symbol, Atom[] args) {
		post("pdj: object '" + name + "' doesn't understand " + symbol);
	}
	
	// compatibility methods
	/////////////////////////////////////////////////////////////
	public final boolean outletBangHigh(int outlet) {
		return outletBang(outlet);
	}
	
	public final boolean outletHigh(int outlet, int value) {
		return outlet(outlet, value);
	}
	
	public final boolean outletHigh(int outlet, float value) {
		return outlet(outlet, value);
	}
	
	public final boolean outletHigh(int outlet, double value) {
		return outlet(outlet, value);
	}
	
	public final boolean outletHigh(int outlet, String value) {
		return outlet(outlet, value);
	}
	
	public final boolean outletHigh(int outlet, String msg, Atom[] args) {
		return outlet(outlet, msg, args);
	}
	
	public final boolean outletHigh(int outlet, Atom[] value) {
		return outlet(outlet, value);
	}
	
	/**
	 * Returns the output stream of PDJ.
	 * @return the PrintStream output stream of PDJ
	 */
	public static com.cycling74.io.PostStream getPostStream() {
		return new com.cycling74.io.PostStream();
	}
	
	/**
	 * Returns the error stream of PDJ.
	 * @return the PrintStream error stream of PDJ
	 */
	public static com.cycling74.io.ErrorStream getErrorStream() {
		return new com.cycling74.io.ErrorStream();
	}	
	
	// unimplementable methods
	/////////////////////////////////////////////////////////////
	/**
	 * <b>NOT USED IN PD.</b>
	 */
	public void viewsource() {
	}

	/**
	 * <b>NOT USED IN PD.</b> Throws <code>UnsupportdOperationException</code></b>
	 */
	public static Object getContext() {
		throw new UnsupportedOperationException();
	}
	
	/**
	 * Returns the object representing the pd patch.
	 */
	public MaxPatcher getParentPatcher() {
	    return patch;
	}
	
	/**
	 * <b>NOT USED IN PD.</b>
	 */
	protected void save() {
	}

	/**
	 * <b>NOT USED IN PD.</b>
	 */
	protected void setInletAssist(String[] messages) {
	}
	
	/**
	 * <b>NOT USED IN PD.</b>
	 */
	protected void setInletAssist(int index, String message) {
	}

	/**
	 * <b>NOT USED IN PD.</b>
	 */
	protected void setOutletAssist(String[] messages) {
	}

	/**
	 * <b>NOT USED IN PD.</b>
	 */
	protected void setOutletAssist(int index, String message) {
	}
	
	/**
	 * <b>NOT USED IN PD.</b>
	 */
	protected void embedMessage(String msg, Atom[] args) {
	}
	
    /**
     * Useless, but in the original API.
     */
    public void gc() {
        System.gc();
    }

    /**
     * Refresh/reinitialize the PDJ classloader.
     */
    public void zap() {
        PDJClassLoader.resetClassloader();
    }
    
	// not from original class
	/////////////////////////////////////////////////////////////
	
	/**
	 * Tries to get the attribute name[0] and send its value to 
	 * the last outlet.
	 * @param name the name of the attribute
	 */
	void get(Atom[] name) throws IllegalAccessException, InvocationTargetException {
		String attrName = name[0].toString();
		if ( attributes.containsKey(attrName) ) {
			Attribute attr = (Attribute) attributes.get(attrName);
			outlet(_outlets_ptr.length-1, attr.get());
		} else {
			error(this.name + ": attribute not defined '" + attrName + "'");
		}
	}
	
	/**
	 * Called by pdj to check if it is possible to set value x.
	 * @param the name of the setter
	 * @param arg [0] the value to set
	 * @return true if the setter has been set
	 */
	private boolean _trySetter(String name, Atom[] arg) {
		if ( !attributes.containsKey(name) )
			return false;
		Attribute attr = (Attribute) attributes.get(name);
		attr.set(arg); 
		return true;
	}
	
	/**
	 * Tries to instantiate a MaxObject.
	 * @param name fq java name
	 * @param _pdobj_ptr C pointer to pd object
	 * @param args objects arguments
	 */
	static synchronized MaxObject registerObject(String name, long _pdobj_ptr, Atom[] args_complete) {
		try {
			Class clz = PDJClassLoader.dynamicResolv(name);
			MaxObject obj = null;
			
			// map arguments and attributes
			List largs = new ArrayList();
			List lattr = new ArrayList();
			
			for (int i=1;i<args_complete.length;i++) {
				if ( args_complete[i].toString().startsWith("@") ) {
					lattr.add(args_complete[i].toString().substring(1));
					if ( i+1>=args_complete.length ) {
                        post("pdj: " + name + ": attribute '" + args_complete[i].toString() +
                            "' must have a initial value");
                        return null;
					}
					lattr.add(args_complete[++i]);
				} else {
					largs.add(args_complete[i]);
				}
			}
			
			Atom args[] = new Atom[largs.size()];
			for (int i=0;i<args.length;i++) {
				args[i] = (Atom) largs.get(i);
			}
            
            Class argType[] = new Class[1];
            argType[0] = Atom[].class;
            
            pushPdjPointer(_pdobj_ptr);
            
            // instantiate the object
			if ( args.length > 0 ) {
				try {
					Object argValue[] = new Object[1];
					argValue[0] = args;
					Constructor c = clz.getConstructor(argType);
					obj = (MaxObject) c.newInstance(argValue);
				} catch ( NoSuchMethodException e) {
                    popPdjPointer();
					post("pdj: object " + name + " has no constructor with Atom[] parameters");
					return null;
				}
			} else {
                try {
                    Constructor c = clz.getConstructor(null);
                    obj = (MaxObject) c.newInstance(null);
                } catch (NoSuchMethodException e) {
                    try {
                        Constructor c = clz.getConstructor(argType);
                        obj = (MaxObject) c.newInstance(new Object[0]);
                    } catch ( Exception e1 ) {
                        popPdjPointer();
                        throw e1;
                    }
                }
			}
			
			// next we process attributes from the constructor arguments
			Iterator i = lattr.iterator();
			while( i.hasNext() ) {
				String attrName = (String) i.next();
				obj.declareAttribute(attrName); 
				Attribute attr = (Attribute) obj.attributes.get(attrName);
				attr.set(new Atom[] { (Atom) i.next() });
			}

			obj.name = name;
			obj.postInit();
			return obj;
        } catch (PDJClassLoaderException e ) {
            MaxSystem.post("pdj: " + e.toString());
        } catch (Throwable e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * Called after the constructor has been called. Check if the user 
	 * has override the standard inlets/outlets.
	 */
	private void postInit() {
		if ( _inlets_ptr == null ) {
			declareInlets(new int[] { DataTypes.ANYTHING }); 
		}
		
		if ( _outlets_ptr == null ) {
			declareOutlets(new int[] { DataTypes.ANYTHING });
		}

		// add the last/info outlet
		if ( toCreateInfoOutlet ) {
			long tmp[] = new long[_outlets_ptr.length+1];
			
			System.arraycopy(_outlets_ptr, 0, tmp, 0, _outlets_ptr.length);
			tmp[_outlets_ptr.length] = newOutlet(DataTypes.ANYTHING); 
			
			_outlets_ptr = tmp; 
		}
	}
	
	// native party
	/////////////////////////////////////////////////////////////
	native private long newInlet(int type);
	native private long newOutlet(int type);
	
	native private void doOutletBang(long ptr);
	native private void doOutletFloat(long ptr, float value);
	native private void doOutletSymbol(long ptr, String symbol);
	native private void doOutletAnything(long ptr, String msg, Atom []args);
	
    native private String getPatchPath();
    
	// ugly ugly... but MaxContext vs this; I prefer this....
	native private static void pushPdjPointer(long ptr);
	native private static long popPdjPointer();
}
