package com.cycling74.msp;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.UnsupportedAudioFileException;

import com.cycling74.max.MessageReceiver;

/**
 * Work in progress, target: 0.8.5
 */
class AudioFileBuffer {
    
    /** file buffer value */
    public float buf[][];
    
    /** Value of the message that will be sended to MessageReceiver when the file will be loaded */
    public static final int FINISHED_READING = 1;
   
    private MessageReceiver callback = null;
    private String file;
    private AudioFormat audioFormat;
    private AudioFormat targetAudioFormat;
    
    // getters values
    private float sampleRate;
    private int sampleBitFormat;
    private boolean sampleBigEndian;
    private int sampleChannels;
    private long sampleFrames;
    
    public AudioFileBuffer(String filename) throws FileNotFoundException, IOException, UnsupportedAudioFileException {
        open(filename);
    }
    
    public AudioFileBuffer(String filename, MessageReceiver callback) throws FileNotFoundException, IOException, UnsupportedAudioFileException {
        this.callback = callback;
        open(filename);
    }
    
    public void open(String filename) throws FileNotFoundException, IOException, UnsupportedAudioFileException {
        // file format 
        sampleRate = 0;
        sampleBitFormat = 0;
        sampleChannels = 0;
        sampleFrames = 0;
        
        AudioInputStream sourceAudioInputStream = AudioSystem.getAudioInputStream( new FileInputStream(file) );
        AudioFormat sourceAudioFormat = sourceAudioInputStream.getFormat();
        targetAudioFormat = new AudioFormat( 
                AudioFormat.Encoding.PCM_UNSIGNED,      // Encoding 
                getSystemSampleRate(),                  // Sample rate
                16,                                     // bit
                sourceAudioFormat.getChannels(),        // channel
                sourceAudioFormat.getFrameSize(),       // frame size
                sourceAudioFormat.getFrameRate(),       // frame rate 
                false );                                // big Endian
        AudioInputStream audioInputStream = AudioSystem.getAudioInputStream(audioFormat, sourceAudioInputStream );
        
        FileLoader thread = new FileLoader(audioInputStream);
        new Thread(thread).start();
    }
    
    class FileLoader implements Runnable {
        AudioInputStream ais;
        
        FileLoader(AudioInputStream input) {
            ais = input;
        }
        
        public void run() {
            try {
                buf = new float[audioFormat.getChannels()][];
                
                for (int i=0;ais.available() != 0;i++) {
                    //doc.content[i] = audioInputStream.read() / 65535;
                }       
                
                if ( callback != null ) 
                    callback.messageReceived(this, FINISHED_READING, null);
            } catch ( IOException e ) {
            }
        }
    }
    
    public float getSampleRate() {
        return sampleRate;
    }
    
    public int getSampleSizeInBits() {
        return sampleBitFormat;
    }
    
    public boolean isBigEndian() {
        return sampleBigEndian;
    }
    
    public long getFrameLength() {
        return sampleFrames;
    }
    
    public int getChannels() {
        return sampleChannels;
    }
    
    public float getLengthMs() {
        return buf[0].length / (sampleRate / 1000);
    }
    
    private native int getSystemSampleRate();
}