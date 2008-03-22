package com.cycling74.net;

import java.net.DatagramPacket;
import java.net.DatagramSocket;

import com.cycling74.max.Atom;
import com.cycling74.max.MaxRuntimeException;
import com.cycling74.max.MaxSystem;
import com.cycling74.max.Callback;

/** 
 * This portion of code is scheduled for pdj-0.8.5
 * IT IS NOT FUNCTIONAL
 */
public class UdpReceiver implements Runnable {
	DatagramSocket receiver;
	DatagramPacket packet;
	
	Callback callback;
	
	String debugString = null;
	int port;
	boolean runnable = true;
	
	public void close() {
		if ( receiver == null ) 
			return;
		runnable = false;
		receiver.close();		
	}
	
	public int getPort() {
		return port;
	}
	
	public void setActive(boolean active) {
		if ( active == false ) {
			runnable = true;
			new Thread(this).start();
		} else {
			close();
		}
		
	}
	
	public void setCallback(Object caller, String methodName) {
	    callback = new Callback(caller, methodName);
	}
	
	public void setPort(int port) {
		setActive(false);
		this.port = port;
	}
	
	public void setDebugString(String debugString) {
		this.debugString = debugString;
	}
	
	public void run() {
		DatagramPacket packet = new DatagramPacket(new byte[4096], 4096);
		Object callerArgs[] = new Object[1];
		
		try {
			while(runnable) {
					receiver.receive(packet);
					String msg = new String(packet.getData(), 0, packet.getLength());
					if ( debugString != null )
						MaxSystem.post(debugString + " " + msg);
					callerArgs[0] = Atom.parse(msg);

		/*			try {
						callback.invoke(instance, callerArgs);
					} catch( Exception e ) {
						e.printStackTrace();
					} */
			}
		} catch (Exception e) {
			if ( runnable != false) {
				runnable = false;
				throw new MaxRuntimeException(e);
			}
		}
	}
}
