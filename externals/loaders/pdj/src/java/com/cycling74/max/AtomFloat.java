package com.cycling74.max;

class AtomFloat extends Atom {
    private static final long serialVersionUID = 6497612039215711052L;
    private float value;
	
	AtomFloat(float value) {
		super(DataTypes.FLOAT);
		this.value = value;
	}
	
	public int compareTo(Object obj) {
		if ( obj instanceof AtomFloat ) {
			AtomFloat f = (AtomFloat) obj;
			if ( f.getFloat() <= getFloat() ) 
				return 1;
			else 
				return -1;
		}
		
		if ( obj instanceof AtomString ) {
			return 1;
		}

		throw new ClassCastException();
	}
	
	public boolean isFloat() {
		return true;
	}
	
	public boolean isInt() {
		return true;
	}
	
	public Object toObject() {
		return new Float(value);
	}
	
	public int toInt() {
		return (int) value;
	}
	
	public double toDouble() {
		return value;
	}
	
	public long toLong() {
		return (long) value;
	}
	
	public float toFloat() {
		return value;
	}
	
	public short toShort() {
		return (short) value;
	}
	
	public byte toByte() {
		return (byte) value;
	}
	
	public char toChar() {
		return (char) value;
	}
	
	public String toString() {
		return new Float(value).toString();
	}
	
	public boolean toBoolean() {
		return value != 0;
	}
	
	public float getFloat() {
		return value;
	}
    
    public int getInt() {
        return (int) value;
    }
	
	public boolean equals(Object comp) {
	    if ( !(comp instanceof AtomFloat) ) {
	        return false;
	    }
	    AtomFloat test = (AtomFloat) comp;
	    
	    return test.value == value; 
	}
	
	public int hashCode() {
	    return new Float(value).hashCode();
	}
}
