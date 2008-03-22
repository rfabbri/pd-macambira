package com.cycling74.io;

import com.e1.pdj.*;
import java.io.*;

public class ErrorStream extends PrintStream {
	public ErrorStream() {
		super(PDJSystem.err, true);
	}
}
