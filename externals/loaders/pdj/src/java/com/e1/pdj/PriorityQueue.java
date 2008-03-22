package com.e1.pdj;

import java.util.*;
import com.cycling74.max.Executable;

public class PriorityQueue implements Runnable {
	List list = new ArrayList();	
	private Thread thread;
	boolean tostop = false;
	
	public PriorityQueue(int priority) {
		thread = new Thread(this);
		
		switch( priority ) {
		case Thread.MIN_PRIORITY :
			thread.setName("PriorityQueue:low");
			break;
		case Thread.NORM_PRIORITY :
			thread.setName("PriorityQueue:norm");
			break;
		case Thread.MAX_PRIORITY :
			thread.setName("PriorityQueue:max");
			break;
		}
		thread.setPriority(priority);
		thread.setDaemon(true);
		thread.start();
	}
	
	public void shutdown() {
		synchronized(this) {
			tostop = true;
			notify();
		}
	}
	
	public void run() {
        Executable exec;
        
		while(true) {
			try {
				synchronized(this) {
                    if ( list.size() == 0 )
                        wait();
					
					if ( tostop ) 
						break;
					exec = (Executable) list.remove(0);
				}
				try {
					exec.execute();
				} catch (Exception e) {
					e.printStackTrace();
				}
			} catch (InterruptedException e) {
				break;
			}
		}
	}
    
	public void defer(Executable e) {
		synchronized(this) {
			list.add(e);
			notify();
		}
	}
	
}
