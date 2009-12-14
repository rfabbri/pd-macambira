/*
 * Readsf object for reading and playing multiple soundfile types
 * from disk and from the web using gmerlin_avdecode
 *
 * Copyright (C) 2003-9 August Black
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Readsf.h
 */

#ifndef _READSF_H_
#define _READSF_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "FifoAudioFrames.h"


#ifndef _AVDEC_H_
#define _AVDEC_H_
extern "C" {
#include <avdec.h>
}
#endif


#define STATE_EMPTY              0
#define STATE_STARTUP           1
#define STATE_READY            2
#define SRC_MAX                 256.0
#define SRC_MIN                 1/256.0

class Readsf  {

	public:
		Readsf( int sr, int nchannels, int frames_in_fifo, int samples_in_each_fifo_frame);
		~Readsf();
		void Open( char * filename);
		int Decode( char * buf, unsigned int lengthinsamples);
		bool Rewind();
		bool RewindNoFlush();
		bool PCM_seek(long bytes);
		bool TIME_seek(double seconds);
		int get_wanted_samplerate();
		int get_input_samplerate();
		int get_wanted_num_channels();
		int get_input_num_channels();
		int get_bytes_per_sample();
		float getLengthInSeconds(); 
		int64_t getTimestamp();

		Fifo * getFifo(); 

		bgav_t * getFile();
		char * getFilename();

		gavl_audio_frame_t * getIAF();
		gavl_audio_frame_t * getOAF();
		gavl_audio_frame_t * getTAF();
		gavl_audio_converter_t *  getT2OAudioConverter();
		gavl_audio_converter_t *  getI2TAudioConverter();
		bgav_options_t * getOpt();

		void setSpeed( float f);
		void setSRCFactor(float f);
		float getSRCFactor();
	
		void setOpenFail();

		void setState(int b);
		int getState();
		bool isReady();
	
		float getTimeInSeconds();
		float getFifoSizePercentage();

		void isOpen( bool b);
		bool isOpen(); 

		void isOpening( bool b); 
		bool isOpening(); 

		void doLoop( bool b); 
		bool doLoop(); 


		void setEOF(bool b);
		bool getEOF();

		int getSamplesPerFrame();
		
		bool doT2OConvert(); 
		void doT2OConvert(bool b); 

		bool doI2TConvert(); 
		void doI2TConvert(bool b); 



		int lockA();
		int unlockA();


		void Wait();
		void Signal();

		void setFile();
		void setOptions();
		bool initFormat();
		void setOpenCallback(void (*oc)(void *), void *v );
		void callOpenCallback();
		
		bool quit;

	private:
		void * callback_data;	
		void (* open_callback)(void * v);
	
		int samples_per_frame;
		int wanted_samplerate;
		bool eof;
		int bytes_per_sample;
		int frames;
		int state;
		char filename[1024];
		float src_factor;
		bool do_t2o_convert;
		bool do_i2t_convert;
		bool opening;  // is the thread_open thread running or not
		bool is_open;	 // is there a file open or not
		bool loop;
		int samplesleft;
		int64_t timestamp;

		//gavl_audio_options_t * aopt;
		bgav_t * file;
		bgav_options_t * opt;

		gavl_audio_converter_t *  i2t_audio_converter;
		gavl_audio_converter_t *  t2o_audio_converter;
		
		gavl_audio_frame_t * iaf;
		gavl_audio_frame_t * oaf;
		gavl_audio_frame_t * taf;

		gavl_audio_format_t input_audio_format;
		gavl_audio_format_t output_audio_format;
		gavl_audio_format_t tmp_audio_format;
		
		Fifo  *fifo;
	
		pthread_t thr_open;
		pthread_t thr_fillfifo;
		pthread_mutex_t condmut;
		pthread_mutex_t amut;
		pthread_cond_t cond;
		

};

#endif
