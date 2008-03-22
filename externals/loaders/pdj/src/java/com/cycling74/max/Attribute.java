package com.cycling74.max;

import java.lang.reflect.*;
import com.e1.pdj.PDJError;

class Attribute {
    boolean readOnly = false;
    
    // the method setter and getter if specified
	Method setter;
	Method getter;
    
    // the field to update if no setter or getter is specified
	Field  field;
    
    // attribute type
	char type;
    
    // the owner of the field or method
	Object obj;
    
    // tells if the arguments are a array
	boolean isArray;
    
	Attribute(Object obj, String name, String setter_name, String getter_name) {
        char typeCheck = 0;
        
		try {
			this.obj = obj;
			if ( getter == null ) {
				field = obj.getClass().getDeclaredField(name);
				field.setAccessible(true);
				type = mapType(field.getType());
                if ( type == '-' ) {
                    throw new PDJError("Field: " + name + " is a unsupported type");
                }
                isArray = Character.isLowerCase(type);
			} else {
				getter = obj.getClass().getDeclaredMethod(getter_name, null);
                type = mapType(getter.getReturnType());
                if ( type == '-' ) {
                    throw new PDJError("Method: " + getter_name + " returns a unsupported type");
                }
                isArray = Character.isLowerCase(type);
			}

			if ( setter == null ) {
				field = obj.getClass().getDeclaredField(name);
				field.setAccessible(true);
				typeCheck = mapType(field.getType());
			} else {
                Method methods[] = obj.getClass().getMethods();
                
                for(int i=0;i<methods.length;i++) {
                    if ( methods[i].getName().equals(setter_name) ) {
                        Class c[] = methods[i].getParameterTypes();
                        if ( c.length > 1 ) 
                            throw new PDJError("Method: " + setter_name + " has too much parameters to be a setter");
                        typeCheck = mapType(c[0]);
                        setter = methods[i];
                        break;
                    }
                }
                if ( typeCheck == 0 ) {
                    throw new NoSuchMethodException(setter_name);
                }
			}
		} catch ( Exception e ) {
			throw new PDJError(e);
		}

        if ( typeCheck != type ) {
            throw new PDJError("Object type for setter and getter is not the same");
        }
        
	}
	
	private char mapType(Class clz) {
		if ( clz == Integer.TYPE )
			return 'i';
        if ( clz == int[].class ) 
            return 'I';
		if ( clz == Float.TYPE )
			return 'f';
        if ( clz == float[].class ) 
            return 'F';
		if ( clz == Double.TYPE )
			return 'd';
        if ( clz == double[].class ) 
            return 'D';
		if ( clz == Boolean.TYPE )
			return 'z';
        if ( clz == Boolean[].class ) 
            return 'Z';
		if ( clz == Byte.TYPE )
			return 'b';
        if ( clz == byte[].class ) 
            return 'B';
		if ( clz == Character.TYPE ) 
			return 'c';
        if ( clz == char[].class )
            return 'C';
		if ( clz == Short.TYPE )
			return 's';
        if ( clz == short[].class )
            return 'S';
		if ( clz == String.class ) 
			return 'g';
        if ( clz == String[].class )
            return 'G';
        if ( clz == Atom.class )
            return 'a';
        if ( clz == Atom[].class ) 
            return 'A';
		return '-';
	}
	
	private void fieldSetter(Atom[] value) throws Exception {
        Object work;
        int i;
        
		switch ( type ) {
		case 'z' :
			field.setBoolean(obj, value[0].toBoolean());
			break;
        case 'Z' :
            work = field.get(obj);
            for (i=0;i<value.length;i++) 
                Array.setBoolean(work, i, value[i].toBoolean());
            break;
		case 'f' :
			field.setFloat(obj, value[0].toFloat());
			break;
        case 'F' :
            work = field.get(obj);
            for (i=0;i<value.length;i++)
                Array.setFloat(work, i, value[i].toFloat());
            break;
		case 'i' :
			field.setInt(obj, value[0].toInt());
			break;
        case 'I' :
            work = field.get(obj);
            for (i=0;i<value.length;i++)
                Array.setInt(work, i, value[i].toInt());
            break;
		case 'd' :
			field.setDouble(obj, value[0].toDouble());
			break;
        case 'D' :
            work = field.get(obj);
            for (i=0;i<value.length;i++)
                Array.setDouble(work, i, value[i].toDouble());
            break;
		case 'b' :
			field.setByte(obj, value[0].toByte());
			break;
        case 'B' :
            work = field.get(obj);
            for (i=0;i<value.length;i++)
                Array.setByte(work, i, value[i].toByte());
            break;
		case 'c' :
			field.setChar(obj, value[0].toChar());
            break;
        case 'C' :
            work = field.get(obj);
            for (i=0;i<value.length;i++)
                Array.setChar(work, i, value[i].toChar());
            break;
		case 's' :
			field.setShort(obj, value[0].toShort());
			break;
        case 'S' :
            work = field.get(obj);
            for (i=0;i<value.length;i++)
                Array.setShort(work, i, value[i].toShort());
            break;
		case 'g' :
			field.set(obj, value.toString());
			break;
        case 'G' :
            work = field.get(obj);
            for (i=0;i<value.length;i++)
                Array.set(work, i, value[i].toString());
            break;
        case 'a' :
            field.set(obj, value[0]);
            break;
        case 'A' :
            field.set(obj, value);
            break;
		default:
			throw new PDJError("Unable to pass attribute value");
		}
	}
	
	private Atom[] fieldGetter() throws Exception {
        Object array = null;
	    Atom[] ret;
        int i;
        
        if ( isArray ) {
            ret = new Atom[1];
        } else { 
            array = field.get(obj);
            ret = new Atom[Array.getLength(array)];
        }
        
        switch ( type ) {
        case 'z' :
            ret[0] = Atom.newAtom(field.getBoolean(obj));
            break;
        case 'Z' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(Array.getBoolean(array, i));
            break;
        case 'f' :
            ret[0] = Atom.newAtom(field.getFloat(obj));
            break;
        case 'F' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(Array.getFloat(array, i));
            break;
        case 'i' :
            ret[0] = Atom.newAtom(field.getInt(obj));
            break;
        case 'I' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(Array.getInt(array, i));
            break;
        case 'd' :
            ret[0] = Atom.newAtom(field.getDouble(obj));
            break;
        case 'D' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(Array.getDouble(array, i));
            break;
        case 'b' :
            ret[0] = Atom.newAtom(field.getByte(obj));
            break;
        case 'B' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(Array.getByte(array, i));
            break;
        case 'c' :
            ret[0] = Atom.newAtom(field.getChar(obj));
            break;
        case 'C' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(Array.getChar(array, i));
            break;
        case 's' :
            ret[0] = Atom.newAtom(field.getShort(obj));
            break;
        case 'S' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(Array.getShort(array, i));
            break;
        case 'g' :
            ret[0] = Atom.newAtom((String) field.get(obj));
            break;
        case 'G' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom((String) Array.get(array, i));
            break;
        case 'a' :
            ret[0] = (Atom) field.get(obj);
            break;
        case 'A' :
            for (i=0;i<ret.length;i++) 
                ret[i] = (Atom) Array.get(array, i);
            break;
        default:
            throw new PDJError("Unable to pass attribute value");
        }
        
		return ret;
	}
	
    private Atom[] methodGetter() throws Exception {
        Object array = null;
        Atom[] ret;
        int i;
        
        if ( isArray ) {
            ret = new Atom[1];
        } else { 
            array = field.get(obj);
            ret = new Atom[Array.getLength(array)];
        }
        
        switch ( type ) {
        case 'z' :
            ret[0] = Atom.newAtom(((Boolean)getter.invoke(obj, null)).booleanValue());
            break;
        case 'Z' :
            boolean tmpz[] = (boolean[]) getter.invoke(obj, null);
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(tmpz[i]);
            break;
        case 'f' :
            ret[0] = Atom.newAtom(((Float)getter.invoke(obj, null)).floatValue());
            break;
        case 'F' :
            float tmpf[] = (float[]) getter.invoke(obj, null);
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(tmpf[i]);
            break;
        case 'i' :
            ret[0] = Atom.newAtom(((Integer)getter.invoke(obj, null)).intValue());
            break;
        case 'I' :
            int tmpi[] = (int[]) getter.invoke(obj, null);
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(tmpi[i]);
            break;
        case 'd' :
            ret[0] = Atom.newAtom(((Double)getter.invoke(obj, null)).doubleValue());
            break;
        case 'D' :
            double tmpd[] = (double[]) getter.invoke(obj, null);
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(tmpd[i]);
            break;
        case 'b' :
            ret[0] = Atom.newAtom(((Byte)getter.invoke(obj, null)).byteValue());
            break;
        case 'B' :
            byte tmpb[] = (byte[]) getter.invoke(obj, null);
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(tmpb[i]);
            break;
        case 'c' :
            ret[0] = Atom.newAtom(((Character)getter.invoke(obj, null)).charValue());
            break;
        case 'C' :
            char tmpc[] = (char[]) getter.invoke(obj, null);
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(tmpc[i]);
            break;
        case 's' :
            ret[0] = Atom.newAtom(((Short)getter.invoke(obj, null)).shortValue());
            break;
        case 'S' :
            short tmps[] = (short[]) getter.invoke(obj, null);
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom(tmps[i]);
            break;
        case 'g' :
            ret[0] = Atom.newAtom((String) getter.invoke(obj, null));
            break;
        case 'G' :
            for (i=0;i<ret.length;i++) 
                ret[i] = Atom.newAtom((String) Array.get(array, i));
            break;
        case 'a' :
            ret[0] = (Atom) getter.invoke(obj, null);
            break;
        case 'A' :
            ret = (Atom []) getter.invoke(obj, null);
            break;
        default:
            throw new PDJError("Unable to pass attribute value");
        }
        
        return ret;
    }
    
    private Object setterCast(Atom values[]) throws Exception {
        int i;
        
        switch ( type ) {
        case 'z' :
            return new Boolean(values[0].toBoolean());
        case 'Z' :
            boolean tmpz[] = new boolean[values.length];
            for (i=0;i<values.length; i++) 
                tmpz[i] = values[i].toBoolean();
            return tmpz;
        case 'f' :
            return new Float(values[0].toFloat());
        case 'F' :
            float tmpf[] = new float[values.length];
            for (i=0;i<values.length; i++) 
                tmpf[i] = values[i].toFloat();
            return tmpf;
        case 'i' :
            return new Integer(values[0].toInt());
        case 'I' :
            int tmpi[] = new int[values.length];
            for (i=0;i<values.length; i++) 
                tmpi[i] = values[i].toInt();
            return tmpi;
        case 'd' :
            return new Double(values[0].toDouble());
        case 'D' :
            double tmpd[] = new double[values.length];
            for (i=0;i<values.length; i++) 
                tmpd[i] = values[i].toDouble();
            return tmpd;
        case 'b' :
            return new Byte(values[0].toByte());
        case 'B' :
            byte tmpb[] = new byte[values.length];
            for (i=0;i<values.length; i++) 
                tmpb[i] = values[i].toByte();
            return tmpb;
        case 'c' :
            return new Character(values[0].toChar());
        case 'C' :
            char tmpc[] = new char[values.length];
            for (i=0;i<values.length; i++) 
                tmpc[i] = values[i].toChar();
            return tmpc;
        case 's' :
            return new Short(values[0].toShort());
        case 'S' :
            short tmps[] = new short[values.length];
            for (i=0;i<values.length; i++) 
                tmps[i] = values[i].toShort();
            return tmps;
        case 'g' :
            return values[0].toString();
        case 'G' :
            String tmpg[] = new String[values.length];
            for (i=0;i<values.length;i++) 
                tmpg[i] = values[0].toString();
            return tmpg;
        case 'a' :
            return values[0];
        case 'A' :
            return values;
        default:
            throw new PDJError("Unable to pass attribute value");
        }
    }
    
	Atom[] get() {
		try {
			if ( getter == null )
				return fieldGetter();
			return methodGetter();
		} catch ( Exception e ){
			throw new PDJError(e);
		}
	}
	
	void set(Atom values[]) {
        if ( readOnly )
            throw new PDJError("Field is readonly");
        
		try {
			if ( setter == null )
				fieldSetter(values);
			else
				setter.invoke(obj, new Object[] { setterCast(values) });
		} catch (Exception e) {
			throw new PDJError(e);
		}
	}

}
