package com.cycling74.net;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

import com.cycling74.max.Atom;
import com.cycling74.max.MaxRuntimeException;

/** 
 * This portion of code is scheduled for pdj-0.8.5
 * IT IS NOT FUNCTIONAL
 */
public class UdpSender {
    InetAddress inetAddress;
	DatagramSocket sender;
	boolean init;
	
	String address = null;
	int port = -1;
	
	public UdpSender() {
	}
	
	public UdpSender(String address, int port) {
		this.address = address;
		this.port = port;
		initsocket();
	}
	
	public void send(Atom args[]) {
		send(Atom.toOneString(args).getBytes());
	}
	
	public void send(int i) {
		send(Integer.toString(i).getBytes());
	}
	
	public void send(float f) {
		send(Float.toString(f).getBytes());
	}

	public String getAddress() {
		return address;
	}
	
	public void setAddress(String address) {
		this.address = address;
		initsocket();
	}
	
	public int getPort() {
		return port;
	}
	
	public void setPort(int port) {
		this.port = port;
		initsocket();
	}
	
	synchronized void initsocket() {
		try {
		    sender = null;
		    inetAddress = InetAddress.getByName(address);
			sender = new DatagramSocket();
		} catch (Exception e) {
			throw new MaxRuntimeException(e);
		}
	}

	void send(byte buff[]) {
	    if ( sender == null )
	        throw new MaxRuntimeException("UdpSender is not initialized");

	    try {
	        DatagramPacket packet = new DatagramPacket(buff, buff.length, inetAddress, port);
	        sender.send(packet);
	    } catch (IOException e) {
	        throw new MaxRuntimeException(e);
	    }

	}
}
