package com.cycling74.max;

/**
 * This is a utility callback class that can be used to notify background job.
 */
public interface MessageReceiver {
    /**
     * Called when a event has occured.
     */
    public void messageReceived(Object src, int messageId, Object data);
}
