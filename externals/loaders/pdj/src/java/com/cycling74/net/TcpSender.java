package com.cycling74.net;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;

import com.cycling74.max.Atom;
import com.cycling74.max.MaxRuntimeException;

/** 
 * This portion of code is scheduled for pdj-0.8.5
 * IT IS NOT FUNCTIONAL
 */
public class TcpSender {
	DatagramSocket sender;
	DatagramPacket packet;
	String address = null;
	int port = -1;
	
	public TcpSender() {
	}
	
	public TcpSender(String address, int port) {
		this.address= address;
		this.port = port;
	}
	
	public void send(Atom args[]) {
		if ( sender == null )
			initsocket();
		
		byte buff[] = Atom.toOneString(args).getBytes();
		packet.setData(buff, 0, buff.length);
		try {
			sender.send(packet);
		} catch (IOException e) {
			throw new MaxRuntimeException(e);
		}
	}
	
	public void send(int i) {
		if ( sender == null )
			initsocket();
		
		byte buff[] = Integer.toString(i).getBytes();
		packet.setData(buff, 0, buff.length);
		try {
			sender.send(packet);
		} catch (IOException e) {
			throw new MaxRuntimeException(e);
		}
	}
	
	public void send(float f) {
		if ( sender == null ) 
			initsocket();
		
		byte buff[] = Float.toString(f).getBytes();
		packet.setData(buff, 0, buff.length);
		try {
			sender.send(packet);
		} catch (IOException e) {
			throw new MaxRuntimeException(e);
		}
	}

	public String getAddress() {
		return address;
	}
	
	public void setAddress(String address) {
		if ( sender != null ) { 
			sender = null;
			sender.close();
		}
		this.address = address;
	}
	
	public int getPort() {
		return port;
	}
	
	public void setPort(int port) {
		if ( sender != null ) {
			sender = null;
			sender.close();
		}
		this.port = port;
	}
	
	private synchronized void initsocket() {
		if ( sender != null )
			return;
		try {
			sender = new DatagramSocket();
			sender.connect(new InetSocketAddress(address, port));
			packet = new DatagramPacket(new byte[0], 0);
		} catch (Exception e) {
			throw new MaxRuntimeException(e);
		}
	}
}
