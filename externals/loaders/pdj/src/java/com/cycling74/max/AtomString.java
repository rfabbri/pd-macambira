package com.cycling74.max;

class AtomString extends Atom {
    private static final long serialVersionUID = 1738861344247036680L;
    private String value;
	
	AtomString(String value) {
		super(DataTypes.MESSAGE);
		this.value = value;
	}
	
	public int compareTo(Object obj) {
		if ( obj instanceof AtomString ) {
			AtomString s = (AtomString) obj;
			return s.getString().compareTo(getString());
		}
		if ( obj instanceof AtomFloat ) {
			return -1;
		}
		throw new ClassCastException();
	}
	
	public boolean isString() {
		return true;
	}
	
	public Object toObject() {
		return value;
	}
	
	public byte toByte() {
		return (byte) value.charAt(0);
	}
	
	public char toChar() {
		return value.charAt(0);
	}
	
	public String toString() {
		return value;
	}
	
	public boolean toBoolean() {
		return !value.equals("false");
	}
	
	public String getString() {
		return value;
	}
	
	public boolean equals(Object comp) {
	    if ( !(comp instanceof AtomString) ) {
	        return false;
	    }
	    AtomString test = (AtomString) comp;
	    
	    return test.value.equals(value);
	}
	
	public int hashCode() {
	    return value.hashCode();
	}
}
