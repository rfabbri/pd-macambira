package com.e1.pdj;

import java.io.*;

import com.cycling74.max.MaxSystem;

public class ConsoleStream extends OutputStream {
	ByteArrayOutputStream buffer = new ByteArrayOutputStream(4096);

	protected void send(String message) {
		MaxSystem.post("pdj: " + message);
	}

	public void flush() {
		String msg = buffer.toString();
		if ( msg.endsWith("\n") ) {
			msg = msg.substring(0, msg.length()-1);
		}
		if ( !msg.equals("") )
			send(buffer.toString());
		buffer.reset();
	}

	public void write(byte[] b) throws IOException {
		buffer.write(b);
	}

	public void write(int b) throws IOException {
		buffer.write(b);
	}

	public void write(byte[] b, int off, int len) throws IOException {
		buffer.write(b, off, len);
	}
}
