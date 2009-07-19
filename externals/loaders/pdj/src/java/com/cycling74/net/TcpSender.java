package com.cycling74.net;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import com.cycling74.max.Atom;
import com.cycling74.max.MaxRuntimeException;

/**
 * Class wrapper to send atoms via TCP/IP. The host on the other side
 * must use TcpReceive to read the sended atoms. 
 * 
 * This class is a work in progress and have been lightly tested.
 */
public class TcpSender {
    InetAddress inetAddress;
	String address = null;
	int port = -1;
	
	public TcpSender() {
	}
	
	public TcpSender(String address, int port) {
	    setAddress(address);
		this.port = port;
	}
	
	public void send(Atom args[]) {
		send(Atom.toOneString(args));
	}
	
	public void send(int i) {
		send(Integer.toString(i));
	}
	
	public void send(float f) {
		send(Float.toString(f));
	}

	public void send(String msg) {
       if ( address == null )
            throw new MaxRuntimeException("TcpSender has no active hosts");
        if ( port == -1 )
            throw new MaxRuntimeException("TcpSender has no active port");
	        
	    try {
	        Socket sender = new Socket(inetAddress, port);
	        sender.getOutputStream().write(msg.getBytes());
	        sender.close();
	    } catch (IOException e) {
            throw new MaxRuntimeException(e);
	    }
	}
	
	public String getAddress() {
		return address;
	}
	
	public void setAddress(String address) {
	    try {
            inetAddress = InetAddress.getByName(address);
        } catch (UnknownHostException e) {
            throw new MaxRuntimeException(e);
        }
		this.address = address;
	}
	
	public int getPort() {
		return port;
	}
	
	public void setPort(int port) {
		this.port = port;
	}
}
