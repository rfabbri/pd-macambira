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
 * Readsf.cpp
 */


#include "Readsf.h"
#include <pthread.h>


static void *thread_open(void * xp);
static void *fill_fifo(void * xp);

Readsf::Readsf( int sr, int nchannels, int frames_in_fifo, int samples_in_each_fifo_frame ) {

	state = STATE_EMPTY;
	src_factor = 1.0;
	wanted_samplerate = sr;
	samples_per_frame = samples_in_each_fifo_frame;

	opt = NULL;
//	aopt = gavl_audio_options_create ();
//	gavl_audio_options_set_resample_mode (aopt, GAVL_RESAMPLE_SINC_MEDIUM);
//	gavl_audio_options_set_quality( aopt, 3 );  //1-5, 5 is the best

	t2o_audio_converter = gavl_audio_converter_create( );
	i2t_audio_converter = gavl_audio_converter_create( );

	oaf = NULL;
	iaf = NULL;
	taf = NULL;
	
	file = NULL;
	
	is_open = false;
	do_t2o_convert = false;
	do_i2t_convert = false;
	
	// assume we are always using the GAVL_SAMPLE_FLOAT format
	bytes_per_sample = sizeof(float);
	samplesleft = 0;
	timestamp = 0;

	// set up our intermediary format so that it has
	// floating point buffers, interlaced, with proper number
	// of channels.  this will be fixed.
	tmp_audio_format.samples_per_frame = samples_per_frame; 
	tmp_audio_format.sample_format = GAVL_SAMPLE_FLOAT ;
	tmp_audio_format.interleave_mode = GAVL_INTERLEAVE_ALL  ;
	tmp_audio_format.num_channels = nchannels;
	tmp_audio_format.channel_locations[0] = GAVL_CHID_NONE; // Reset
	tmp_audio_format.samplerate = sr;   // we need to really set this to whatever the file in is
	gavl_set_channel_setup (&tmp_audio_format); // Set channel locations

	output_audio_format.sample_format = GAVL_SAMPLE_FLOAT ;
	//output_audio_format.interleave_mode = GAVL_INTERLEAVE_NONE ;
	output_audio_format.interleave_mode = GAVL_INTERLEAVE_ALL ;
	output_audio_format.num_channels = nchannels;
	output_audio_format.channel_locations[0] = GAVL_CHID_NONE; // Reset
	output_audio_format.samplerate = sr;

	output_audio_format.samples_per_frame = samples_per_frame * SRC_MAX +10;
	gavl_set_channel_setup (&output_audio_format); // Set channel locations

	taf = gavl_audio_frame_create(&tmp_audio_format);
	oaf = gavl_audio_frame_create(&output_audio_format);

	//printf("creating fifo with %d frames of size %d = %d samples\n", frames_in_fifo, samples_in_each_fifo_frame, frames_in_fifo * samples_in_each_fifo_frame);	
	fifo= new Fifo( frames_in_fifo ,  &tmp_audio_format); 
	
	open_callback = NULL;
	callback_data = NULL;

	quit = false;
	loop = false;
	
	sprintf(filename, "what up foool!");

	opening = false;

	pthread_cond_init(&cond, 0);
	pthread_mutex_init(&condmut, 0);
	pthread_mutex_init(&amut, 0);
	if( pthread_create(&thr_fillfifo, NULL, fill_fifo, (void *)this) != 0 )
		printf( "Failed to create fillfifo thread.\n");
}

Readsf::~Readsf() {
	quit = true;
	Signal();
	Signal();
	Signal();
	//printf("destroying Readsf\n");
	pthread_join( thr_fillfifo, NULL);
	//printf("joined fillfifo thread\n");
	if ( opening ) {
		pthread_join( thr_open, NULL);
		//printf("joined the open thread\n");
	}
	if (oaf != NULL) {
		gavl_audio_frame_destroy(oaf);
		//printf("destroyed oaf\n");
	}
	if (iaf != NULL) {
		gavl_audio_frame_destroy(iaf);
		//printf("destroyed iaf\n");
	}
	if (taf != NULL) {
		gavl_audio_frame_destroy(taf);
		//printf("destroyed taf\n");
	}
	gavl_audio_converter_destroy(t2o_audio_converter);
	gavl_audio_converter_destroy(i2t_audio_converter);
	if (is_open) {
		bgav_close(file);
		//printf("closed file\n");
	}
	//printf("now, on to deleting fifo...\n");
	delete fifo;
}

int Readsf::Decode( char * buf, unsigned int lengthinsamples ) {
	int lis = lengthinsamples;
	unsigned int ret,writesize,totalwritesize = 0;
	int bufcnt=0;
	int num_channels = output_audio_format.num_channels;

	while (lis > 0) {
		if ( lis <= samplesleft) {
			writesize = lis * num_channels * bytes_per_sample;
			if( oaf->valid_samples < samplesleft)
				printf("Readsf::Decode,  valid_samples < samplesleft, shouldn't happen\n");
			//printf("OUCH!! lis <= samplesleft| oaf->vs %d, lis=%d, samplesleft=%d, writesize=%d, bufcnt=%d\n", 
			//oaf->valid_samples, lis, samplesleft, writesize, bufcnt);
			// we should watch here.  if oaf->valid_samples is less than samplesleft, we have problems!
			memcpy(  (buf+bufcnt), (void *)( oaf->samples.f + ((oaf->valid_samples  - samplesleft ) * num_channels) ), writesize );
			samplesleft -= lis; 
			lis = 0;
			totalwritesize += writesize;
			bufcnt += writesize;
			//printf("lis <= samplesleft oaf->vs %d, lis=%d  samplesleft=%d, bufcnt=%d\n",
			//oaf->valid_samples, lis, samplesleft, bufcnt);
			break;
		} else if (samplesleft > 0) {
			writesize = samplesleft * num_channels * bytes_per_sample;
			if( oaf->valid_samples < samplesleft)
				printf("Readsf::Decode,  valid_samples < samplesleft, shouldn't happen\n");
			//	printf("samplesleft > 0 oaf->vs %d, lis=%d, samplesleft=%d, writesize=%d, bufcnt=%d\n", 
			//	oaf->valid_samples, lis, samplesleft, writesize, bufcnt);
			memcpy( (buf+bufcnt), (void *) ( oaf->samples.f +  ((oaf->valid_samples - samplesleft)  * num_channels)), writesize );
			lis = lis - samplesleft;
			samplesleft = 0;
			totalwritesize += writesize;
			bufcnt += writesize;
			//printf("samplesleft > 0 oaf->vs %d, lis=%d  samplesleft=%d, bufcnt=%d\n", 
			//oaf->valid_samples, lis, samplesleft, bufcnt);
		} else { //samplesleft should be zero
			// no samples left, get a new frame
			//gavl_audio_frame_null(taf);
			

			// now we try to get a frame...but depending on networks and disk access, 
			// we might not have a full fifo to draw from. So, let's try 5 times
			/*bool gotit = false;
			unsigned int c =0;
			while (gotit == false && c++ < 5) { 
				lockA();
				gotit = fifo->Get( taf );
				unlockA();
			}
			if (!gotit  ) {
				printf("Couldn't get a frame\n"); // this can only happen if the fifo is empty
				return totalwritesize / num_channels / bytes_per_sample;
			}
			*/

			lockA();
			 // try only once to get a frame
			if (!fifo->Get( taf )  ) {
				unlockA();
				printf("Couldn't get a frame\n"); // this can only happen if the fifo is empty
				return totalwritesize / num_channels / bytes_per_sample;
			}
			
			timestamp = taf->timestamp;
			if ( do_t2o_convert  ) {
				//gavl_audio_convert( t2o_audio_converter, taf, oaf );
				gavl_audio_converter_resample( t2o_audio_converter, taf, oaf, src_factor );
				//  Don't know why, but on the first conversion, I get one extra sample
				//  THIS SHOULD NOT HAPPEN...this is a fix for now..check it out later.
				if (src_factor == 1.0 && oaf->valid_samples > samples_per_frame) { 
					//printf("Got wierd return value for audio frames,  taf->vs %d, oaf->vs %d, src_factor=%f\n", taf->valid_samples, oaf->valid_samples, src_factor);
					samplesleft = oaf->valid_samples = samples_per_frame;
				} else {
					samplesleft = oaf->valid_samples;
				}
				//printf("converting taf to oaf,  taf->vs %d, oaf->vs %d\n", taf->valid_samples, oaf->valid_samples);
			} else {
				// copy the samples to the output
				gavl_audio_frame_copy(&tmp_audio_format, oaf,  taf, 0,0, taf->valid_samples, taf->valid_samples) ;
				//printf("copying taf to oaf,  taf->vs %d, oaf->vs %d\n", taf->valid_samples, oaf->valid_samples);
				samplesleft = taf->valid_samples;
				oaf->valid_samples = taf->valid_samples;
			}
			unlockA();
		}
	
	}
	this->Signal();
	//printf("-----\n");
	ret = totalwritesize / num_channels / bytes_per_sample;
	if (ret != lengthinsamples) 
		printf("OUCH %d\n",  ret );
	return ret ;
}

bool Readsf::Rewind() {
	if (is_open) {
		iaf->valid_samples = 0;
		if (!doLoop())
			samplesleft = 0;
		PCM_seek( 0);
		eof = false;
		// fifo flush happens when we seek
		return true;
	}
	return false;
}

float Readsf::getLengthInSeconds() {
	gavl_time_t t;

	if (file != NULL && is_open) {
		t= bgav_get_duration ( file, 0);
		return (float)(gavl_time_to_samples( input_audio_format.samplerate, t) / (float)input_audio_format.samplerate);	 
	}
	return 0.0;
}

void Readsf::setSpeed( float f) {
	if (is_open) {
		if (f > SRC_MAX) 
			return;
		if (f < SRC_MIN)
			return;
		//lockA();
		src_factor = 1.0/f;
		output_audio_format.samplerate =  src_factor * tmp_audio_format.samplerate;
		//doT2OConvert( gavl_audio_converter_init( t2o_audio_converter, 
		//		&tmp_audio_format, &output_audio_format) ) ;
		//unlockA();
	}
}

bool Readsf::RewindNoFlush() {
	gavl_time_t gt = gavl_samples_to_time( (int)input_audio_format.samplerate, 0 ) ;        
	if (file != NULL && is_open) {
		if (bgav_can_seek ( file) ) {
			bgav_seek(file, &gt);
			return true;
		}
	}
	return false;
}

bool Readsf::PCM_seek(long samples) {
	gavl_time_t gt = gavl_samples_to_time( (int)input_audio_format.samplerate, (int64_t) samples ) ;        
	if (file != NULL && is_open) {
		if (bgav_can_seek ( this->file) ) {
			//bgav_seek_audio(x->file, 0, (int64_t) f);
			this->lockA();
			this->fifo->Flush();
			bgav_seek(this->file, &gt);
			this->unlockA();
			this->Signal();
			//post ("seeking %d", gt);
			return true;
		}
	}
	return false;
}

bool Readsf::TIME_seek(double seconds) {
	gavl_time_t gt = gavl_seconds_to_time(  seconds ) ;        
	if (this->is_open && file != NULL) {
		if (bgav_can_seek ( this->file) ) {
			//bgav_seek_audio(x->file, 0, (int64_t) f);
			this->lockA();
			this->fifo->Flush();
			bgav_seek(this->file, &gt);
			this->unlockA();
			this->Signal();
			//post ("seeking %d", gt);
			return true;
		}
	}
	return false;
return false;
}

void Readsf::Open(char * fn) {
	if (state == STATE_STARTUP) {
		printf("Still opening a file what to do?\n");
	}
	state = STATE_STARTUP;
	// how can we be careful here of s_name's with commas in them
	//printf("%s\n", fn);
	sprintf(filename, "%s", fn);
	fifo->Flush();

	if ( is_open ) {
		pthread_join( thr_open, NULL);
		printf("joinging thread open\n");
	}
	
	is_open = false;
	
	if ( pthread_create(&thr_open, NULL, thread_open, (void *)this) != 0 )
		printf( "Failed to create thr_open thread.\n");
}


void Readsf::setOptions() {
	//Use the only to destroy options, you created with bgav_options_create. Options returned 
	// by bgav_get_options are owned by the bgav_t instance, and must not be freed by you. 
	//if (opt == NULL)
	//	free (opt);
	//opt = NULL;
	if (file != NULL) {
		opt = bgav_get_options(file);
		bgav_options_set_connect_timeout(opt,  5000);
		bgav_options_set_read_timeout(opt,     5000);
		bgav_options_set_network_bandwidth(opt, 524300);
		bgav_options_set_network_buffer_size(opt, 1024*12);
		bgav_options_set_http_shoutcast_metadata (opt, 1);
		// set up the reading so that we can seek sample accurately
		// bgav_options_set_sample_accurate (rdsf->opt, 1 );
	}
}

bool Readsf::initFormat() {

	const gavl_audio_format_t * open_audio_format;

	// only concerned with the first audio stream
	open_audio_format = bgav_get_audio_format(file, 0);    

		// we can get audio formats that are unkown
	if (open_audio_format->sample_format == GAVL_SAMPLE_NONE) {
 		printf("sorry, this file has unsupported audio.\n"); 
		return false;	
	}

	gavl_audio_format_copy(&input_audio_format, open_audio_format);
	input_audio_format.samples_per_frame = samples_per_frame;

	if (iaf != NULL)
		gavl_audio_frame_destroy(iaf);

	iaf = gavl_audio_frame_create(&input_audio_format);

	setSRCFactor( wanted_samplerate / (float) input_audio_format.samplerate); 
	//printf("set src to %f\n", wanted_samplerate / (float) input_audio_format.samplerate);
	//make sure the input samplerate is the same as the tmp samplerate
	tmp_audio_format.samplerate = input_audio_format.samplerate;
	
	//doT2OConvert( gavl_audio_converter_init( t2o_audio_converter, &tmp_audio_format, &output_audio_format) ) ;
		
	doT2OConvert( gavl_audio_converter_init_resample( t2o_audio_converter, &output_audio_format) ) ;

	doI2TConvert( gavl_audio_converter_init( i2t_audio_converter, 
				&input_audio_format, &tmp_audio_format) ) ;

	return true;

	/*
	printf("\n-----------\ninput_audio_format\n");
	gavl_audio_format_dump ( &input_audio_format);

	printf("\n-----------\ntmp_audio_format\n");
	gavl_audio_format_dump ( &tmp_audio_format);

	printf("\n-----------\noutput_audio_format\n");
	gavl_audio_format_dump ( &output_audio_format);
	*/
}
void Readsf::setOpenFail() {
	is_open =  false;
	opening = false;
	state = STATE_EMPTY;
	free( file );
	file = NULL;
}
void *thread_open(void *xp) {
	Readsf *rdsf = (Readsf *)xp;
	int num_urls, i, num_audio_streams;

	rdsf->isOpening(true);	
	rdsf->lockA();
	
	rdsf->setFile();
	rdsf->setOptions();

	if(!bgav_open(rdsf->getFile(), rdsf->getFilename())) {
		printf( "Could not open file %s\n", rdsf->getFilename());
		rdsf->setOpenFail();
		rdsf->unlockA();
		rdsf->callOpenCallback();
		return NULL;
	} else {
		rdsf->isOpen( true );
		printf("opened %s\n", rdsf->getFilename());
	}


	if(bgav_is_redirector(rdsf->getFile() )) {
		num_urls = bgav_redirector_get_num_urls(rdsf->getFile() );
		printf( "Found redirector with %d urls inside, we will try to use the first one.\n", num_urls);
		printf( "Name %d: %s\n", 1, bgav_redirector_get_name(rdsf->getFile() , 0));
		printf("URL %d: %s\n",  1, bgav_redirector_get_url(rdsf->getFile(), 0));
		sprintf(rdsf->getFilename(), "%s", bgav_redirector_get_url(rdsf->getFile(), 0) );
		rdsf->setFile();
		if (!bgav_open( rdsf->getFile(), rdsf->getFilename() ) ) {
			printf("Could not open redirector\n");
			rdsf->setOpenFail( );
			rdsf->unlockA();
			rdsf->callOpenCallback();
			return NULL;
		} else {
			rdsf->isOpen(true);
			printf("opened redirector %s\n", rdsf->getFilename());
		}
	}
	//rdsf->num_tracks = bgav_num_tracks(rdsf->getFile());
	//track =0;

	bgav_select_track(rdsf->getFile(), 0);

	num_audio_streams = bgav_num_audio_streams(rdsf->getFile(), 0);
	for(i = 0; i < num_audio_streams; i++)
		bgav_set_audio_stream(rdsf->getFile(), i, BGAV_STREAM_DECODE);

	
	if(!bgav_start(rdsf->getFile())) {
		printf( "failed to start file\n");
		rdsf->setOpenFail();
		rdsf->unlockA();
		rdsf->callOpenCallback();
		return NULL;
	}
	//bgav_dump(rdsf->getFile());
	
	if( !rdsf->initFormat() ){
		rdsf->setOpenFail();
		rdsf->unlockA();
		rdsf->callOpenCallback();
		return NULL;
	}

	rdsf->setState( STATE_READY);
	rdsf->unlockA();
	rdsf->Signal();
	rdsf->callOpenCallback();
	rdsf->isOpening(false);	
	return NULL;
}

void *fill_fifo( void * xp) {
	int samples_returned;
	Readsf *rdsf = (Readsf *)xp;
	
	while (!rdsf->quit) {
	
		if (rdsf->isReady() ) {
			
			while ( rdsf->getFifo()->FreeSpace() ) {
				//printf("got a frame\n");	
				rdsf->lockA();
				samples_returned = bgav_read_audio(rdsf->getFile(), rdsf->getIAF(), 0,  rdsf->getSamplesPerFrame() );

				if (samples_returned == 0 ) {
					if (rdsf->doLoop() ) {
						//printf("looping ...\n");
						rdsf->RewindNoFlush();
					} else {
						//printf("end of file \n");
						rdsf->setEOF(true);
					}
					//quit = true;
					rdsf->unlockA();
					break;
				}
				if (rdsf->doI2TConvert() ) {
					//printf("samps_returned %d, tmp_format spf %d\n", samples_returned, rdsf->getFifo()->getFormat()->samples_per_frame);
					gavl_audio_convert( rdsf->getI2TAudioConverter(), rdsf->getIAF(), rdsf->getTAF()) ;
					if( !rdsf->getFifo()->Append( rdsf->getTAF() )) {
						printf("problem with appending TAF\n");
					}
				} else {
					if( !rdsf->getFifo()->Append( rdsf->getIAF() ))
						printf("problem with appending TAF\n");
				}
				rdsf->unlockA();
			}
		}
		rdsf->Wait();
		// wait for external signal to continue in this thread.
		//printf("waiting\n");	
	}
	return NULL;

}
int Readsf::get_wanted_samplerate() { return wanted_samplerate;}
int Readsf::get_input_samplerate() { return input_audio_format.samplerate;}
int Readsf::get_wanted_num_channels() { return output_audio_format.num_channels;}
int Readsf::get_input_num_channels() { return input_audio_format.num_channels;}
int Readsf::get_bytes_per_sample() { return bytes_per_sample;}

Fifo * Readsf::getFifo() { return fifo; } 

bgav_t * Readsf::getFile() {return  file;}
char * Readsf::getFilename() {return  filename;}

gavl_audio_frame_t * Readsf::getIAF() { return iaf;}
gavl_audio_frame_t * Readsf::getOAF() { return oaf;}
gavl_audio_frame_t * Readsf::getTAF() { return taf;}
gavl_audio_converter_t *  Readsf::getT2OAudioConverter(){ return t2o_audio_converter;};
gavl_audio_converter_t *  Readsf::getI2TAudioConverter(){ return i2t_audio_converter;};
bgav_options_t * Readsf::getOpt() {return opt; }

void Readsf::setSpeed( float f);
void Readsf::setSRCFactor(float f) {  src_factor = f;}
float Readsf::getSRCFactor() { return src_factor;}

void Readsf::setState(int b) { this->state = b;}
int Readsf::getState() { return state;}
bool Readsf::isReady() { if (state == STATE_READY) return true; else return false;}

int64_t Readsf::getTimestamp() { return timestamp;};
float Readsf::getTimeInSeconds() { return timestamp / (float)input_audio_format.samplerate;};
float Readsf::getFifoSizePercentage() { return fifo->getSizePercentage();};

void Readsf::isOpen( bool b) { is_open = b; } 
bool Readsf::isOpen() { return is_open; } 

void Readsf::isOpening( bool b) { opening = b; } 
bool Readsf::isOpening() { return opening; } 

void Readsf::doLoop( bool b) { loop = b; } 
bool Readsf::doLoop() { return loop; } 

void Readsf::setEOF(bool b) { eof = b;}
bool Readsf::getEOF() { return eof;}

int Readsf::getSamplesPerFrame() { return samples_per_frame;}

bool Readsf::doT2OConvert() { return do_t2o_convert; } 
void Readsf::doT2OConvert(bool b) { do_t2o_convert = b; } 

bool Readsf::doI2TConvert() { return do_i2t_convert; } 
void Readsf::doI2TConvert(bool b) { do_i2t_convert = b; } 

int Readsf::lockA() { return pthread_mutex_lock(&amut);}
int Readsf::unlockA() { return pthread_mutex_unlock(&amut);}

void Readsf::Wait() { pthread_cond_wait( &cond, &condmut); }
void Readsf::Signal() { pthread_cond_signal( &cond); }

void Readsf::setFile() { if (file != NULL) { bgav_close(file); } file = bgav_create(); }
void Readsf::setOpenCallback(void (*oc)(void *), void *v ) { this->open_callback = oc; this->callback_data = v;  };
void Readsf::callOpenCallback() {	if(open_callback != NULL) this->open_callback( this->callback_data); };

