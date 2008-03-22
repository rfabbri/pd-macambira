package com.cycling74.net;

import java.lang.reflect.Method;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

import com.cycling74.max.Atom;
import com.cycling74.max.MaxRuntimeException;
import com.cycling74.max.MaxSystem;

/** 
 * This portion of code is scheduled for pdj-0.8.5
 * IT IS NOT FUNCTIONAL
 */
public class TcpReceiver implements Runnable {
	DatagramSocket receiver;
	DatagramPacket packet;
	
	Method callback = null;
	Object instance;
	
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
		try {
			callback = caller.getClass().getDeclaredMethod(methodName, new Class[] { Atom.class });
			instance = caller;
		} catch (Exception e) {
			throw new MaxRuntimeException(e);
		}
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
					try {
						callback.invoke(instance, callerArgs);
					} catch( Exception e ) {
						e.printStackTrace();
					}
			}
		} catch (Exception e) {
			if ( runnable != false) {
				runnable = false;
				throw new MaxRuntimeException(e);
			}
		}
	}
}
