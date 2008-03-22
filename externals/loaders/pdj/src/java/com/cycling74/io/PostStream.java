package com.cycling74.io;

import com.e1.pdj.*;
import java.io.*;

public class PostStream extends PrintStream {
	public PostStream() {
		super(PDJSystem.out, true);
	}
}
