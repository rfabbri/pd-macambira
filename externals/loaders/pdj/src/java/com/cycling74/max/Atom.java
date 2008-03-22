package com.cycling74.max;

import java.io.*;
import java.util.*;

/**
 * PD element that is used in message or arguments. It can contains a 
 * float, a int (always map to a float in pd) or a string.
 */
public abstract class Atom implements Comparable, Serializable {
    
    /**
     * Empty array to use with the API when theres is no arguments.
     */
    public static final Atom[] emptyArray = new Atom[] {};
    
	int type;
	
	Atom(int type) {
		this.type = type;
	}

	// Atom factories
	///////////////////////////////////////////////////////////////////
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(int value) {
		return new AtomFloat(value);
	}

	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(int value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}

	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(long value) {
		return new AtomFloat(value);
	}

	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(long value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
		
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(short value) {
		return new AtomFloat(value);
	}

	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(short value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
		
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(byte value) {
		return new AtomFloat(value);
	}

	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(byte value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(char value) {
		return new AtomString(String.valueOf(value));
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(char value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(boolean value) {
		return new AtomFloat( value ? 1:0 );
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(boolean value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(float value) {
		return new AtomFloat(value);
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(float value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(double value) {
		return new AtomFloat((float) value);
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(double value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom newAtom(String value) {
		return new AtomString(value);
	}
	
	/**
	 * Creates a new atom with the specified type.
	 * @param value the value of the atom
	 * @return the new Atom
	 */
	public static Atom[] newAtom(String value[]) {
		Atom[] ret = new Atom[value.length];
		for(int i=0;i<value.length;i++) {
			ret[i] = newAtom(value[i]);
		}
		return ret;
	}
    
	/**
	 * Creates an array of atoms from string tokens. 
	 * @param values the atoms value seperated by string
	 * @return the array of atoms created from the list
	 */
	public static Atom[] parse(String values) {
	    return parse(values, false);
	}

	/**
	 * Creates an array of atoms from strings tokens. 
	 * @param values the atoms value seperated by string
	 * @param skipfirst skip the first token
	 * @return the array of atoms created from the list
	 */
	public static Atom[] parse(String values, boolean skipfirst) {
	    ArrayList list = new ArrayList();
	    StringTokenizer token = new StringTokenizer(values);
	    
	    if ( skipfirst && token.hasMoreTokens() ) {
	        token.nextToken();
	    }
	    
	    while(token.hasMoreTokens()) {
	        list.add(newAtom(token.nextToken()));
	    }
	    
	    Iterator i = list.iterator();
	    Atom[] ret = new Atom[list.size()];
	    for(int idx=0; i.hasNext(); idx++) {
	        ret[idx] = (Atom) i.next();
	    }
	    
	    return ret;
	}
	
	// Atom methods
	///////////////////////////////////////////////////////////////////
	
	/**
	 * Get string value for this Atom. If the Atom does not contains a 
	 * string it will throw an UnsupportedOperationException.
	 */
	public String getString() {
		throw new UnsupportedOperationException("pdj: this atom is not a string");
	}

	/**
	 * Get int value for this Atom. If the Atom does not contains a 
	 * int it will throw an UnsupportedOperationException.
	 */
	public int getInt() {
		throw new UnsupportedOperationException("pdj: this atom is not a int");
	}
	
	/**
	 * Get float value for this Atom. If the Atom does not contains a 
	 * float it will throw an UnsupportedOperationException.
	 */
	public float getFloat() {
		throw new UnsupportedOperationException("pdj: this atom is not a float");
	}
	
	/**
	 * Returns the Object representation of the Atom. If it is an int or a 
	 * float, it will return a Int/Float object.
	 * @return The java.lang.* representation of the object
	 */
	abstract Object toObject();
	
	/**
	 * Returns the double value of this atom.
	 * @return the double value
	 */
	public double toDouble() {
		return 0;
	}
	
	/**
	 * Transform an array of Atom into an array of doubles.
	 * @param values the array of atoms
	 * @return array of doubles
	 */
	public static double[] toDouble(Atom[] values) {
	    double[] ret = new double[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toDouble();
	    }
	    return ret;
	}
	
	/**
	 * Returns the int value of this atom.
	 * @return the int value
	 */
	public int toInt() {
		return 0;
	}

	/**
	 * Transform an array of Atom into an array of ints.
	 * @param values the array of atoms
	 * @return array of ints
	 */
	public static int[] toInt(Atom[] values) {
	    int[] ret = new int[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toInt();
	    }
	    return ret;
	}

	
	/**
	 * Returns the char value of this atom.
	 * @return the char value
	 */
	public char toChar() {
		return 0;
	}
	
	/**
	 * Transform an array of Atom into an array of chars.
	 * @param values the array of atoms
	 * @return array of chars
	 */
	public static char[] toChar(Atom[] values) {
	    char[] ret = new char[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toChar();
	    }
	    return ret;
	}
	
	
	/**
	 * Returns the byte value of this atom.
	 * @return the byte value
	 */
	public byte toByte() {
		return 0;
	}
	
	/**
	 * Transform an array of Atom into an array of bytes.
	 * @param values the array of atoms
	 * @return array of bytes
	 */
	public static byte[] toByte(Atom[] values) {
	    byte[] ret = new byte[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toByte();
	    }
	    return ret;
	}
	
	/**
	 * Returns the long value of this atom.
	 * @return the long value
	 */
	public long toLong() {
		return 0;
	}
	
	/**
	 * Transform an array of Atom into an array of longs.
	 * @param values the array of atoms
	 * @return array of longs
	 */
	public static long[] toLong(Atom[] values) {
	    long[] ret = new long[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toLong();
	    }
	    return ret;
	}
	
	/**
	 * Returns the short value of this atom.
	 * @return the short value
	 */
	public short toShort() {
		return 0;
	}
	
	/**
	 * Transform an array of Atom into an array of shorts.
	 * @param values the array of atoms
	 * @return array of shorts
	 */
	public static short[] toShort(Atom[] values) {
	    short[] ret = new short[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toShort();
	    }
	    return ret;
	}
	
	/**
	 * Returns the float value of this atom.
	 * @return the float value
	 */
	public float toFloat() {
		return 0;
	}
	
	/**
	 * Transform an array of Atom into an array of floats.
	 * @param values the array of atoms
	 * @return array of floats
	 */
	public static float[] toFloat(Atom[] values) {
	    float[] ret = new float[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toFloat();
	    }
	    return ret;
	}
	
	/**
	 * Returns the boolean value of this atom.
	 * @return the boolean value
	 */
	public boolean toBoolean() {
		return true;
	}
	
	/**
	 * Transform an array of Atom into an array of booleans.
	 * @param values the array of atoms
	 * @return array of booleans
	 */
	public static boolean[] toBoolean(Atom[] values) {
	    boolean[] ret = new boolean[values.length];
	    
	    for(int i=0;i<values.length;i++) {
	        ret[i] = values[i].toBoolean();
	    }
	    return ret;
	}

	/**
	 * Transform an array of Atom into an array of strings.
	 * @param array the array of atoms
	 * @return array of strings
	 */
	public static String[] toString(Atom[] array) {
	    String[] ret = new String[array.length];
	    
	    for(int i=0;i<array.length;i++) {
	        ret[i] = array[i].toString();
	    }
	    return ret;
	}
	
	/**
	 * Returns true if the Atom has been created with a float.
	 * @return true if the Atom has been created with a float
	 */
	public boolean isFloat() {
		return false;
	}
	
	/**
	 * Returns true if the Atom has been created with a String.
	 * @return true if the Atom has been created with a String
	 */
	public boolean isString() {
		return false;
	}
	
	/**
	 * Returns true if the Atom has been created with a int.
	 * @return true if the Atom has been created with a int
	 */
	public boolean isInt() {
		return false;
	}
	
	/**
	 * Returns the array of Atom into one string. Atom elements are 
	 * seperated by spaces.
	 * @param array the array of atom
	 * @return one string representation of the atom
	 */
	public static String toOneString(Atom[] array) {
	    StringBuffer sb = new StringBuffer();
	    boolean appendSpace = false;
	    
	    for(int i=0;i<array.length;i++) {
	        if ( !appendSpace ) { 
	            sb.append(' ');
	            appendSpace = true;
	        }
	        sb.append(array[i].toString());
	    }
	    
	    return sb.toString();
	}
	
	/**
	 * Returns true if the instance has the same value of the object
	 * <code>object</code>. Similar to <code>"ok".equals("ok");</code>
	 */
	public abstract boolean equals(Object object);

	/**
	 * Returns the hashCode representation for this Atom. If it is an float,
	 * the float into bit value is return and if it is an String, the hashcode
	 * value of the string is returned.
	 */
	public abstract int hashCode();
	
	// Array utility classes
	///////////////////////////////////////////////////////////////////

	/**
	 * Returns the index of the first atom that is found in the list.
	 * @param org the atom to find
	 * @param list the list of atom to search
	 * @return the index of the atom found. -1 if not found.
	 */
	public static int isIn(Atom org, Atom[] list) {
		return isIn(org, list, 0, list.length-1);
	}

	/**
	 * Returns the index of the first atom that is found in the list.
	 * @param org the atom to find
	 * @param list the list of atom to search
	 * @param from the start index to check
	 * @param to the last index to check
	 * @return the index of the atom found. -1 if not found.
	 */
	public static int isIn(Atom org, Atom[] list, int from, int to) {
		for(int i=from;i<to+1;i++) {
			if ( list[i].equals(org) ) 
				return i;
		}
		return -1;
	}

	/**
	 * Remove at index <code>from</code> to index <code>to</code>.
	 * @param list the list to strip
	 * @param from the start index
	 * @param to the last index
	 * @return the stripped array
	 */
	public static Atom[] removeSome(Atom[] list, int from, int to) {
		if ( from == 0 && from == list.length - 1 ) 
			return new Atom[] {};

		Atom[] ret = new Atom[list.length - (to-from+1)];
		System.arraycopy(list, 0, ret, 0, from);
	    System.arraycopy(list, to + 1, ret, from, list.length - to - 1);
		
		return ret;
	}
	
	/**
	 * Removes one atom in the list.
	 * @param list the list to strip
	 * @param i the index of the atom to remove
	 * @return the stripped array
	 */
	public static Atom[] removeOne(Atom[] list, int i) {
		return removeSome(list, i, i);
	}
	
	/**
	 * Removes the first atom in the list.
	 * @param list the list to strip
	 * @return the stripped array
	 */
	public static Atom[] removeFirst(Atom[] list) {
	    return removeFirst(list, 1);
	}
	
	/**
	 * Removes the first <code>howmany</code> atoms in the list. 
	 * @param list the list to strip
	 * @param howmany how many element to remove
	 * @return the stripped array
	 */
	public static Atom[] removeFirst(Atom[] list, int howmany) {
	    return removeSome(list, 0, howmany-1);
	}

	/**
	 * Remove the last atom in the list.
	 * @param list the list to strip
	 * @return the stripped array
	 */
	public static Atom[] removeLast(Atom[] list) {
	    return removeLast(list, 1);
	}
	
	/**
	 * Removes the last <code>howmany</code> atoms in the list.
	 * @param list the list to strip
	 * @param howmany how many element to remove
	 * @return the stripped array
	 */
	public static Atom[] removeLast(Atom[] list, int howmany) {
	    return removeSome(list, list.length-howmany, list.length-1);
	}
	
	/**
	 * Reverses the element content; the first element is the last and so on.
	 * @param list the list to reverse
	 * @return the reversed list
	 */
	public static Atom[] reverse(Atom[] list) {
	    Atom[] ret = new Atom[list.length];
	    int last = list.length - 1;
	    for(int i=0;i<list.length;i++) {
	        ret[last-i] = list[i];
	    }
	    return ret;
	}
	
	/**
	 * Rotates array content x number of times.
	 * @param list the list to rotate
	 * @param nbTimes the number of time that the array must be rotated
	 * @return the rotated list
	 */
	public static Atom[] rotate(Atom[] list, int nbTimes) {
	    Atom ret[] = new Atom[list.length];
	    
	    for(int i=0;i<list.length;i++) {
	        ret[(i+nbTimes) % list.length] = list[i];
	    }
	    
	    return ret;
	}

	/**
	 * Don't know what this does. Max/MSP says: it does an union. If
	 * you want me to support this, send me an email and what it does.
	 * @param first
	 * @param second
	 * @return nothing yet
	 * @throws UnsupportedOperationException
	 */
	public static Atom[] union(Atom[] first, Atom[] second) throws UnsupportedOperationException {
	    throw new UnsupportedOperationException("not implemented.");
	}
	
	/**
	 * Don't know what this does. Max/MSP says: it does an intersection. 
	 * If you want me to support this, send me an email and what it does.
	 * @param first
	 * @param second
	 * @return nothing yet
	 * @throws UnsupportedOperationException
	 */
	public static Atom[] intersection(Atom[] first, Atom[] second)  throws UnsupportedOperationException {
	    throw new UnsupportedOperationException("not implemented.");
	}
	
	/**
	 * Used to return a string representation of the list with atom type.
	 * @param values the array of atoms
	 * @return the string representation of the array
	 */
	public static String toDebugString(Atom[] values) {
	    StringBuffer sb = new StringBuffer();
	    
	    sb.append("Atom[");
	    sb.append(values.length);
	    sb.append("]=");
	    for(int i=0;i<values.length;i++) {
	        sb.append('{');
	        sb.append(values[i].toString());
	        if ( values[i] instanceof AtomString ) 
	            sb.append(":S}");
	        else 
	            sb.append(":F}");
	    }
	    
	    return sb.toString();
	}
}
