package com.e1.pdj;


public class PDJError extends Error {
    private static final long serialVersionUID = 1264000707984047887L;
    
    public PDJError(String msg) {
		super(msg);
	}
	public PDJError(Throwable t) {
		initCause(t);
	}
    public PDJError() {
    }
}
