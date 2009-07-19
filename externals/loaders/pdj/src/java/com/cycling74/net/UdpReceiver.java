package com.cycling74.net;

import java.net.DatagramPacket;
import java.net.DatagramSocket;

import com.cycling74.max.Atom;
import com.cycling74.max.MaxRuntimeException;
//import com.cycling74.max.MaxSystem;
import com.cycling74.max.Callback;

/**
 * Class wrapper to receive atoms via UDP/IP using the class
 * UdpSender. 
 * 
 * This class is a work in progress and have been lightly tested.
 */
public class UdpReceiver implements Runnable {
	DatagramSocket receiver;
	DatagramPacket packet;
	
	Callback callback = null;
	String debugString = null;
	boolean runnable = true;
    int port = -1;
	
    public UdpReceiver() {
    }
    
    public UdpReceiver(int port) {
        this.port = port;
    }
    
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
		if ( active == true ) {
		    if ( port == -1 )
		        throw new MaxRuntimeException("No UDP port specified");
			try {
			    receiver = new DatagramSocket(port);
			} catch ( Exception e ) {
			    throw new MaxRuntimeException(e);
			}
            runnable = true;
			new Thread(this, "UdpReceiver[" + port + "]").start();
		} else {
			close();
		}
	}
	
	public void setCallback(Object caller, String methodName) {
	    callback = new Callback(caller, methodName, new Object[] { new Atom[0] });
	}
	
	public void setPort(int port) {
		setActive(false);
		this.port = port;
	}
	
	public void setDebugString(String debugString) {
		this.debugString = debugString;
	}
	
	public void run() {
		try {
			while(runnable) {
			        DatagramPacket packet = new DatagramPacket(new byte[4096], 4096);
					receiver.receive(packet);
					String msg = new String(packet.getData(), 0, packet.getLength());
					//if ( debugString != null )
					//	MaxSystem.post(debugString + " " + msg);
					
					if ( callback != null ) {
					    callback.setArgs(Atom.parse(msg));
					    try {
					        callback.execute();
					    } catch( Exception e ) {
					        e.printStackTrace();
					    }
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
