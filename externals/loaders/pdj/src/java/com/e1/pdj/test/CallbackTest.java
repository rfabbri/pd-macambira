package com.e1.pdj.test;

import com.cycling74.max.*;
import junit.framework.TestCase;

public class CallbackTest extends TestCase {
    
    
    
    public void testArgs() {
        Callback callback = new Callback(this, "callme", new Object[] { new Integer[0] });
        Integer[] args = new Integer[] { new Integer(1), new Integer(2) };
        callback.setArgs((Object[]) args);
        callback.execute();
    }
    
    public void callme(Integer args[]) {
        
    }
}
