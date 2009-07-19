package com.cycling74.net;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

import com.cycling74.max.Atom;
import com.cycling74.max.MaxRuntimeException;

/**
 * Class wrapper to send atoms via UDP/IP. The host on the other side
 * must use UdpReceive to read the sended atoms. 
 * 
 * This class is a work in progress and have been lightly tested.
 */
public class UdpSender {
    InetAddress inetAddress;
	DatagramSocket sender;
	boolean init;
	
	String address = null;
	int port = -1;
	
	public UdpSender() {
	}
	
	/**
	 * Create a UpdSender.
	 * @param address the hostname/ip address of the host to reach
	 * @param port the UDP port to use
	 */
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

	public void send(String msg, Atom args[]) {
        send((msg + " " + Atom.toOneString(args)).getBytes());
	}
	
	/**
	 * Returns the hostname/ip address to reach.
	 * @return hostname/ip address to reach
	 */
	public String getAddress() {
		return address;
	}
	
	/**
	 * Sets hostname/ip address to reach.
	 * @param address hostname/ip address to reach
	 */
	
	public void setAddress(String address) {
		this.address = address;
		if ( port != -1 )
		    initsocket();
	}
	
	/**
	 * Returns the UDP port to use.
	 * @return the UDP port to use
	 */
	public int getPort() {
		return port;
	}
	
    /**
     * Sets the UDP port to use.
     * @param port the UDP port to use
     */
	public void setPort(int port) {
		this.port = port;
		if ( address != null )
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
	        throw new MaxRuntimeException("UdpSender: UPD port or address is missing");
	    
	    try {
	        DatagramPacket packet = new DatagramPacket(buff, buff.length, inetAddress, port);
	        sender.send(packet);
	    } catch (IOException e) {
	        throw new MaxRuntimeException(e);
	    }
	}
}
