/* Copyright (c) 2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file calls Ross Bencina's and Phil Burk's Portaudio package.  It's
    the main way in for Mac OS and, with Michael Casey's help, also into
    ASIO in Windows. */

/* dolist.
    write a version of OpenAudioStream to take nchannels param 
    how do we specify latency?
    listing devices?
    choosing devices?
*/

#include "m_imp.h"
#include <stdio.h>
#include <stdlib.h>
#include "pablio_pd.h"
#include "portaudio.h"
#ifndef NT
#include "portmidi.h"
#include "porttime.h"
#include "pminternal.h"
#endif

    /* public interface declared in m_imp.h */

    /* implementation */
static PABLIO_Stream  *pa_stream;
static int pa_inchans, pa_outchans;
static float *pa_soundin, *pa_soundout;

#define MAX_PA_CHANS 32
#define MAX_SAMPLES_PER_FRAME MAX_PA_CHANS * DACBLKSIZE

int pa_open_audio(int inchans, int outchans, int rate, t_sample *soundin,
    t_sample *soundout, int framesperbuf, int nbuffers,
    int indeviceno, int outdeviceno)
{
    PaError err;
    /* post("in %d out %d rate %d device %d", inchans, outchans, rate, deviceno); */
    if (inchans != 0 && outchans != 0 && inchans != outchans)
    	error("portaudio: number of input and output channels must match");
    if (sys_verbose)
	post("portaudio: opening for %d channels in, %d out",
		inchans, outchans);
    if (inchans > MAX_PA_CHANS)
    {
	post("input channels reduced to maximum %d", MAX_PA_CHANS);
	inchans = MAX_PA_CHANS;
    }
    if (outchans > MAX_PA_CHANS)
    {
	post("output channels reduced to maximum %d", MAX_PA_CHANS);
	outchans = MAX_PA_CHANS;
    }

    if (inchans && outchans)
    	err = OpenAudioStream( &pa_stream, rate, paFloat32,
	    PABLIO_READ_WRITE, inchans, framesperbuf, nbuffers, indeviceno, outdeviceno);
    else if (inchans)
    	err = OpenAudioStream( &pa_stream, rate, paFloat32,
	    PABLIO_READ, inchans, framesperbuf, nbuffers, indeviceno, outdeviceno);
    else if (outchans)
    	err = OpenAudioStream( &pa_stream, rate, paFloat32,
	    PABLIO_WRITE, outchans, framesperbuf, nbuffers, indeviceno, outdeviceno);
    else err = 0;
    if ( err != paNoError ) 
    {
	fprintf( stderr, "Error number %d occured opening portaudio stream\n",
	    err); 
	fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	Pa_Terminate();
	return (1);
    }
    else if (sys_verbose)
	post("... opened OK.");
    pa_inchans = inchans;
    pa_outchans = outchans;
    pa_soundin = soundin;
    pa_soundout = soundout;
    return (0);
}

void pa_close_audio( void)
{
    if (pa_inchans || pa_outchans)
    	CloseAudioStream( pa_stream );
    pa_inchans = pa_outchans = 0;
}

int pa_send_dacs(void)
{
    float samples[MAX_SAMPLES_PER_FRAME], *fp1, *fp2;
    int i, j;
    double timebefore;
    
    timebefore = sys_getrealtime();
    if (pa_inchans)
    {
    	ReadAudioStream(pa_stream, samples, DACBLKSIZE);
    	for (j = 0, fp1 = pa_soundin; j < pa_inchans; j++, fp1 += DACBLKSIZE)
    	    for (i = 0, fp2 = samples + j; i < DACBLKSIZE; i++,
	    	fp2 += pa_inchans)
	{
	    fp1[i] = *fp2;
	}
    }
    if (pa_outchans)
    {
    	for (j = 0, fp1 = pa_soundout; j < pa_outchans; j++, fp1 += DACBLKSIZE)
    	    for (i = 0, fp2 = samples + j; i < DACBLKSIZE; i++,
	    	fp2 += pa_outchans)
	{
	    *fp2 = fp1[i];
            fp1[i] = 0;
	}
    	WriteAudioStream(pa_stream, samples, DACBLKSIZE);
    }

    if (sys_getrealtime() > timebefore + 0.002)
    	return (SENDDACS_SLEPT);
    else return (SENDDACS_YES);
}


void pa_listdevs(void)     /* lifted from pa_devs.c in portaudio */
{
    int      i,j;
    int      numDevices;
    const    PaDeviceInfo *pdi;
    PaError  err;
    Pa_Initialize();
    numDevices = Pa_CountDevices();
    if( numDevices < 0 )
    {
	fprintf(stderr, "ERROR: Pa_CountDevices returned 0x%x\n", numDevices );
	err = numDevices;
	goto error;
    }
    fprintf(stderr, "Number of devices = %d\n", numDevices );
    for( i=0; i<numDevices; i++ )
    {
	pdi = Pa_GetDeviceInfo( i );
	fprintf(stderr, "---------------------------------------------- #%d",
	    i+1 );
	if( i == Pa_GetDefaultInputDeviceID() ) fprintf(stderr, " DefaultInput");
	if( i == Pa_GetDefaultOutputDeviceID() ) fprintf(stderr, " DefaultOutput");
	fprintf(stderr, "\nName         = %s\n", pdi->name );
	fprintf(stderr, "Max Inputs   = %d", pdi->maxInputChannels  );
	fprintf(stderr, ", Max Outputs = %d\n", pdi->maxOutputChannels  );
	if( pdi->numSampleRates == -1 )
	{
	    fprintf(stderr, "Sample Rate Range = %f to %f\n", pdi->sampleRates[0], pdi->sampleRates[1] );
	}
	else
	{
	    fprintf(stderr, "Sample Rates =");
	    for( j=0; j<pdi->numSampleRates; j++ )
	    {
		    fprintf(stderr, " %8.2f,", pdi->sampleRates[j] );
	    }
	    fprintf(stderr, "\n");
	}
	fprintf(stderr, "Native Sample Formats = ");
	if( pdi->nativeSampleFormats & paInt8 )        fprintf(stderr, "paInt8, ");
	if( pdi->nativeSampleFormats & paUInt8 )       fprintf(stderr, "paUInt8, ");
	if( pdi->nativeSampleFormats & paInt16 )       fprintf(stderr, "paInt16, ");
	if( pdi->nativeSampleFormats & paInt32 )       fprintf(stderr, "paInt32, ");
	if( pdi->nativeSampleFormats & paFloat32 )     fprintf(stderr, "paFloat32, ");
	if( pdi->nativeSampleFormats & paInt24 )       fprintf(stderr, "paInt24, ");
	if( pdi->nativeSampleFormats & paPackedInt24 ) fprintf(stderr, "paPackedInt24, ");
	fprintf(stderr, "\n");
    }

    fprintf(stderr, "----------------------------------------------\n");
    goto midi;

error:
    fprintf( stderr, "An error occured while using the portaudio stream\n" ); 
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );

    	    /* and MIDI */
midi: ;
#ifndef NT
    for (i = 0; i < Pm_CountDevices(); i++)
    {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        printf("%d: %s, %s", i, info->interf, info->name);
        if (info->input) printf(" (input)");
        if (info->output) printf(" (output)");
        printf("\n");
    }
#endif
}
