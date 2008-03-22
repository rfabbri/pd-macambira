package com.e1.pdj;

import com.cycling74.max.MaxSystem;

/**
 * Win32 has a special stream since it can contains /r/n that will
 * be duplicated in the console
 */
public class ConsoleStreamWin32 extends ConsoleStream {
	protected void send(String message) {
		StringBuffer ret = new StringBuffer();
        
		for (int i=0;i<message.length();i++) {
			char c = message.charAt(i);

			if ( c != '\r' )
				ret.append(c);
		}
        
		message = ret.toString();
		if ( !message.equals("\n") )
			MaxSystem.post("pdj: " + ret.toString());
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
}
