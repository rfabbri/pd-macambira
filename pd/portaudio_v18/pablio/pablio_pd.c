/*
 * $Id: pablio_pd.c,v 1.1.1.2 2004-02-02 11:27:58 ggeiger Exp $
 * pablio.c
 * Portable Audio Blocking Input/Output utility.
 *
 * Author: Phil Burk, http://www.softsynth.com
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.audiomulch.com/portaudio/
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

 /* changes by Miller Puckette to support Pd:  device selection, 
    settable audio buffer size, and settable number of channels.
    LATER also fix it to poll for input and output fifo fill points. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "portaudio.h"
#include "ringbuffer.h"
#include "pablio_pd.h"	/* MSP */
#include <string.h>

    /* MSP -- FRAMES_PER_BUFFER constant removed */
static void NPa_Sleep(int n)	/* MSP wrapper to check we never stall... */
{
#if 0
    fprintf(stderr, "sleep\n");
#endif
    Pa_Sleep(n);
}

/************************************************************************/
/******** Prototypes ****************************************************/
/************************************************************************/

static int blockingIOCallback( void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               PaTimestamp outTime, void *userData );
static PaError PABLIO_InitFIFO( RingBuffer *rbuf, long numFrames, long bytesPerFrame );
static PaError PABLIO_TermFIFO( RingBuffer *rbuf );

/************************************************************************/
/******** Functions *****************************************************/
/************************************************************************/

/* Called from PortAudio.
 * Read and write data only if there is room in FIFOs.
 */
static int blockingIOCallback( void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               PaTimestamp outTime, void *userData )
{
    PABLIO_Stream *data = (PABLIO_Stream*)userData;
    long numBytes = data->bytesPerFrame * framesPerBuffer;
    (void) outTime;

    /* This may get called with NULL inputBuffer during initial setup. */
    if( inputBuffer != NULL )
    {
        RingBuffer_Write( &data->inFIFO, inputBuffer, numBytes );
    }
    if( outputBuffer != NULL )
    {
        int i;
        int numRead = RingBuffer_Read( &data->outFIFO, outputBuffer, numBytes );
        /* Zero out remainder of buffer if we run out of data. */
        for( i=numRead; i<numBytes; i++ )
        {
            ((char *)outputBuffer)[i] = 0;
        }
    }

    return 0;
}

/* Allocate buffer. */
static PaError PABLIO_InitFIFO( RingBuffer *rbuf, long numFrames, long bytesPerFrame )
{
    long numBytes = numFrames * bytesPerFrame;
    char *buffer = (char *) malloc( numBytes );
    if( buffer == NULL ) return paInsufficientMemory;
    memset( buffer, 0, numBytes );
    return (PaError) RingBuffer_Init( rbuf, numBytes, buffer );
}

/* Free buffer. */
static PaError PABLIO_TermFIFO( RingBuffer *rbuf )
{
    if( rbuf->buffer ) free( rbuf->buffer );
    rbuf->buffer = NULL;
    return paNoError;
}

/************************************************************
 * Write data to ring buffer.
 * Will not return until all the data has been written.
 */
long WriteAudioStream( PABLIO_Stream *aStream, void *data, long numFrames )
{
    long bytesWritten;
    char *p = (char *) data;
    long numBytes = aStream->bytesPerFrame * numFrames;
    while( numBytes > 0)
    {
        bytesWritten = RingBuffer_Write( &aStream->outFIFO, p, numBytes );
        numBytes -= bytesWritten;
        p += bytesWritten;
        if( numBytes > 0) NPa_Sleep(10); /* MSP */
    }
    return numFrames;
}

/************************************************************
 * Read data from ring buffer.
 * Will not return until all the data has been read.
 */
long ReadAudioStream( PABLIO_Stream *aStream, void *data, long numFrames )
{
    long bytesRead;
    char *p = (char *) data;
    long numBytes = aStream->bytesPerFrame * numFrames;
    while( numBytes > 0)
    {
        bytesRead = RingBuffer_Read( &aStream->inFIFO, p, numBytes );
        numBytes -= bytesRead;
        p += bytesRead;
        if( numBytes > 0) NPa_Sleep(10); /* MSP */
    }
    return numFrames;
}

/************************************************************
 * Return the number of frames that could be written to the stream without
 * having to wait.
 */
long GetAudioStreamWriteable( PABLIO_Stream *aStream )
{
    int bytesEmpty = RingBuffer_GetWriteAvailable( &aStream->outFIFO );
    return bytesEmpty / aStream->bytesPerFrame;
}

/************************************************************
 * Return the number of frames that are available to be read from the
 * stream without having to wait.
 */
long GetAudioStreamReadable( PABLIO_Stream *aStream )
{
    int bytesFull = RingBuffer_GetReadAvailable( &aStream->inFIFO );
    return bytesFull / aStream->bytesPerFrame;
}

/************************************************************/
static unsigned long RoundUpToNextPowerOf2( unsigned long n )
{
    long numBits = 0;
    if( ((n-1) & n) == 0) return n; /* Already Power of two. */
    while( n > 0 )
    {
        n= n>>1;
        numBits++;
    }
    return (1<<numBits);
}

/************************************************************
 * Opens a PortAudio stream with default characteristics.
 * Allocates PABLIO_Stream structure.
 *
 * flags parameter can be an ORed combination of:
 *    PABLIO_READ, PABLIO_WRITE, or PABLIO_READ_WRITE
 */
PaError OpenAudioStream( PABLIO_Stream **rwblPtr, double sampleRate,
                         PaSampleFormat format, long flags, int nchannels,
			 int framesperbuf, int nbuffers,
                         int indeviceno, int outdeviceno) /* MSP */
{
    long   bytesPerSample;
    long   doRead = 0;
    long   doWrite = 0;
    PaError err;
    PABLIO_Stream *aStream;
    long   minNumBuffers;
    long   numFrames;

    /* fprintf(stderr,
    	"open %lf fmt %d flags %d ch: %d fperbuf: %d nbuf: %d devs: %d %d\n",
	   sampleRate, format, flags, nchannels,
	   framesperbuf, nbuffers, indeviceno, outdeviceno); */

    if (indeviceno < 0)  /* MSP... */
    {
	indeviceno = Pa_GetDefaultInputDeviceID();
	fprintf(stderr, "using default input device number: %d\n", indeviceno);
    }
    if (outdeviceno < 0)
    {
	outdeviceno = Pa_GetDefaultOutputDeviceID();
	fprintf(stderr, "using default output device number: %d\n", outdeviceno);
    }
    /* fprintf(stderr, "nchan %d, flags %d, bufs %d, framesperbuf %d\n",
	    nchannels, flags, nbuffers, framesperbuf); */
    	/* ...MSP */

    /* Allocate PABLIO_Stream structure for caller. */
    aStream = (PABLIO_Stream *) malloc( sizeof(PABLIO_Stream) );
    if( aStream == NULL ) return paInsufficientMemory;
    memset( aStream, 0, sizeof(PABLIO_Stream) );

    /* Determine size of a sample. */
    bytesPerSample = Pa_GetSampleSize( format );
    if( bytesPerSample < 0 )
    {
        err = (PaError) bytesPerSample;
        goto error;
    }
    aStream->samplesPerFrame = nchannels;  /* MSP */
    aStream->bytesPerFrame = bytesPerSample * aStream->samplesPerFrame;

    /* Initialize PortAudio  */
    err = Pa_Initialize();
    if( err != paNoError ) goto error;

/* Warning: numFrames must be larger than amount of data processed per
  interrupt inside PA to prevent glitches. */  /* MSP */
    minNumBuffers = Pa_GetMinNumBuffers(framesperbuf, sampleRate);
    if (minNumBuffers > nbuffers)
	fprintf(stderr,
	"warning: number of buffers %d less than recommended minimum %d\n",
	    (int)nbuffers, (int)minNumBuffers);
    numFrames = nbuffers * framesperbuf;
    /* fprintf(stderr, "numFrames %d\n", numFrames); */
    /* Initialize Ring Buffers */
    doRead = ((flags & PABLIO_READ) != 0);
    doWrite = ((flags & PABLIO_WRITE) != 0);
    if(doRead)
    {
        err = PABLIO_InitFIFO( &aStream->inFIFO, numFrames, aStream->bytesPerFrame );
        if( err != paNoError ) goto error;
    }
    if(doWrite)
    {
        long numBytes;
        err = PABLIO_InitFIFO( &aStream->outFIFO, numFrames, aStream->bytesPerFrame );
        if( err != paNoError ) goto error;
        /* Make Write FIFO appear full initially. */
        numBytes = RingBuffer_GetWriteAvailable( &aStream->outFIFO );
        RingBuffer_AdvanceWriteIndex( &aStream->outFIFO, numBytes );
    }

    /* Open a PortAudio stream that we will use to communicate with the underlying
     * audio drivers. */
    err = Pa_OpenStream(
              &aStream->stream,
              (doRead ? indeviceno : paNoDevice),  /* MSP */
              (doRead ? aStream->samplesPerFrame : 0 ),
              format,
              NULL,
              (doWrite ? outdeviceno : paNoDevice),  /* MSP */
              (doWrite ? aStream->samplesPerFrame : 0 ),
              format,
              NULL,
              sampleRate,
	      framesperbuf,  /* MSP */
	      nbuffers,      /* MSP */
	      paNoFlag,      /* MSP -- portaudio will clip for us */
              blockingIOCallback,
              aStream );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( aStream->stream );
    if( err != paNoError )	/* MSP */
    {
        fprintf(stderr, "Pa_StartStream failed; closing audio stream...\n");
        CloseAudioStream( aStream );
        goto error;
    }

    *rwblPtr = aStream;
    return paNoError;

error:
    *rwblPtr = NULL;
    return err;
}

/************************************************************/
PaError CloseAudioStream( PABLIO_Stream *aStream )
{
    PaError err;
    int bytesEmpty;
    int byteSize = aStream->outFIFO.bufferSize;

    /* If we are writing data, make sure we play everything written. */
    if( byteSize > 0 )
    {
        bytesEmpty = RingBuffer_GetWriteAvailable( &aStream->outFIFO );
        while( bytesEmpty < byteSize )
        {
            NPa_Sleep( 10 ); /* MSP */
            bytesEmpty = RingBuffer_GetWriteAvailable( &aStream->outFIFO );
        }
    }

    err = Pa_StopStream( aStream->stream );
    if( err != paNoError ) goto error;
    err = Pa_CloseStream( aStream->stream );
    if( err != paNoError ) goto error;
    Pa_Terminate();

error:
    PABLIO_TermFIFO( &aStream->inFIFO );
    PABLIO_TermFIFO( &aStream->outFIFO );
    free( aStream );
    return err;
}
