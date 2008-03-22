package com.cycling74.max;

/**
 * Java object that encapsulate the pd patch. Minimal support on pdj; only
 * getPath() is supported.
 */
public class MaxPatcher {
    String patchPath;
    
    /**
     * Returns the absolute path of the associated patch.
     * @return the absolute patch path
     */
    public String getPath() {
        return patchPath;
    }
}
