package com.cycling74.net;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;

import com.cycling74.max.Atom;
import com.cycling74.max.Callback;
import com.cycling74.max.MaxRuntimeException;
import com.cycling74.max.MaxSystem;

/**
 * Class wrapper to receive atoms via TCP/IP using the class
 * TcpSender.
 *
 * This class is a work in progress and have been lightly tested.
 */
public class TcpReceiver implements Runnable {
    ServerSocket receiver;
    
	Callback callback = null;
	
	String debugString = null;
	int port = -1;
	boolean runnable = true;
	
	public TcpReceiver() {
	    
	}

    public TcpReceiver(int port) {
        this.port = port;
    }

    public TcpReceiver(int port, Object caller, String method) {
        this.port = port;
        this.callback = new Callback(caller, method, new Object[] { new Atom[0] });
    }

	
	public void close() {
		if ( receiver == null ) 
			return;
		runnable = false;
		try {
            receiver.close();
        } catch (IOException e) {
            e.printStackTrace();
        }		
	}
	
	public int getPort() {
		return port;
	}
	
	public void setActive(boolean active) {
	    if ( port == -1 )
            throw new MaxRuntimeException("No TCP port specified");
	    
		if ( active == true ) {
			try {
                receiver = new ServerSocket(port);
            } catch (IOException e) {
                throw new MaxRuntimeException(e);
            }
            runnable = true;
			new Thread(this, "TcpSender[" + port + "]").start();
		} else {
			close();
		}
		
	}
	
	public void setCallback(Object caller, String methodName) {
		try {
		    this.callback = new Callback(caller, methodName, new Object[] { new Atom[0] });
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
	
	private void parseMessage(BufferedReader reader) throws IOException {
	    while(runnable) { 
	        String msg = reader.readLine();
            if ( debugString != null )
                MaxSystem.post(debugString + " " + msg);
        
            if ( callback != null ) {
                callback.setArgs(Atom.parse(msg));
                try {  
                    callback.execute();
                } catch( Exception e ) {
                    e.printStackTrace();
                }
            }
	    }
	}
	
	public void run() {
		try {
			while(runnable) {
	            Socket socket = receiver.accept();
	            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
	            parseMessage(reader);
			}
		} catch (Exception e) {
			if ( runnable != false) {
				runnable = false;
				throw new MaxRuntimeException(e);
			}
		}
	}
}
