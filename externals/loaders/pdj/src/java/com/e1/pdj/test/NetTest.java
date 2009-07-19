package com.e1.pdj.test;

import com.cycling74.max.*;
import com.cycling74.net.*;
import junit.framework.TestCase;

public class NetTest extends TestCase {
    Atom value[];
    
    public void testUDP() {
        UdpReceiver receive = new UdpReceiver(7500);
        receive.setCallback(this, "callback_test");
        receive.setActive(true);
        
        value = null;
        
        UdpSender send = new UdpSender("localhost", 7500); 
        send.send(7500);
        
        try {
            Thread.sleep(1000);
        } catch ( InterruptedException e ) {
        }

        receive.close();
        
        assertNotNull(value);
        assertEquals(Integer.parseInt(value[0].toString()), 7500);
    }
    
    public void callback_test(Atom args[]) {
        value = args;
    }
    
    public void testTCP() {
        TcpReceiver receive = new TcpReceiver(7500);
        receive.setCallback(this, "callback_test");
        receive.setActive(true);
        
        value = null;
        
        TcpSender send = new TcpSender("localhost", 7500); 
        send.send(7500);
        
        try {
            Thread.sleep(1000);
        } catch ( InterruptedException e ) {
        }

        receive.close();
        
        assertNotNull(value);
        assertEquals(Integer.parseInt(value[0].toString()), 7500);        
    }
}
