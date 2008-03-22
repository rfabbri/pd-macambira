package com.e1.pdj;

public class PDJClassLoaderException extends Exception {
    private static final long serialVersionUID = 8714491904913363787L;

    public PDJClassLoaderException(String msg) {
		super(msg);
	}
	
	public PDJClassLoaderException(String msg, Exception root) {
		super(msg, root);
	}

	public PDJClassLoaderException(Exception root) {
		super(root);
	}
}
