package com.cycling74.max;

/**
 * Background job utility class. This is used to execute code in the 
 * background that might take time to execute. Calling <code>set</code>
 * will trigger/execute the "job". If <code>set</code> is called while 
 * the "job" is running, the "job" won't be called again.
 * <p>On PDJ, a Qelem is simply a Java thread that wait to be triggered.
 * </p> 
 */
public class MaxQelem {
    Thread job;
	Executable exec;
	boolean incall;
	static int nbQelem = 0;
	boolean stopThread = true;
	
    /**
     * Constructs a Qelem that is bound the overriden class with method
     * name qfn.
     */
	public MaxQelem() {
		exec = new Callback(this, "qfn");
		do_init();
	}

    /**
     * Constructs a Qelem that is bound to an executable.
     * @param exec the executable to run
     */
	public MaxQelem(Executable exec) {
		this.exec = exec;
		do_init();
	}
	
    /**
     * Constructs a Qelem that is bound to a class with method name.
     * @param src the object that contains the method
     * @param method the method to execute
     */
	public MaxQelem(Object src, String method) {
		exec = new Callback(src, method);
		do_init();
	}
	
	private void do_init() {
	    job = new Thread(new Dispatcher(), "MaxQelem#" + (++nbQelem));
	    job.setPriority(Thread.MIN_PRIORITY);
	}
	
	/**
	 * Puts thread in front execution. <b>Does nothing on PDJ</b>
	 */
	public void front() {
	}

	/**
	 * The callback method, otherwise it calls the 'executable' method 
	 * provided in constructor.
	 */
	public void qfn() {	
	}
	
	/** 
	 * Ask qfn to execute. If it is already set, the qfn won't be called
	 * twice.
	 */
	public synchronized void set() {
	    if ( !incall ) {
	        job.notify();
	    }
	}
	
	/**
	 * Cancels execution of qelem. Use with caution since it will throw
	 * an InterruptedException on PDJ.
	 */
	public synchronized void unset() {
	    job.interrupt();
	}
	
	
    /**
     * Releases current Qelem and cancel the running thread.
     */
	public void release() {
		unset();
		stopThread = true;
	}

    /**
     * Returns the executable object.
     * @return the executble object that is bound to this Qelem
     */
	public Executable getExecutable() {
		return exec;
	}
	
	class Dispatcher implements Runnable {
	    public void run() {
	        while( !stopThread ) {
	            try {
	                incall = false;
	                job.wait();
	                incall = true;
	                exec.execute();
	            } catch (InterruptedException e) {
                   // TODO Auto-generated catch block
	                e.printStackTrace();
                }
		    }
	    }
	}
}
