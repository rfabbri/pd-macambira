/*
 * readanysf~  external for pd. 
 * 
 * Copyright (C) 2003, 2004 August Black 
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
 * main.cpp
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


#include "main.h"



readanysf::readanysf ()
{
  fifo = new Fifo (FIFOSIZE);
  request = R_NOTHING;
  state = STATE_IDLE;
  loop = false;
  eof = true;
  format = -1;
  readsf = NULL;
  in = NULL;
  floatmsg = 0.0;
  cachemsg = 0.0;
  outtick = 1000;
  counttick = 0;
  lengthinseconds = 0.0;
  sendout = false;
  src_buffer = (float *) malloc ((Blocksize ()) * 2 * FLOATSIZE);
  
  src_mode = SRC_SINC_FASTEST;
  src_state = NULL;
  src_data.input_frames = 0;
  
  AddInAnything ();	// add one inlet for any message
  AddOutSignal ("audio out Left");
  AddOutSignal ("audio out Right");
  AddOutAnything ();	// add one float outlet (has index 2)
  
  FLEXT_ADDMETHOD_ (0, "start", m_start);
  FLEXT_ADDMETHOD_ (0, "play", m_start);
  FLEXT_ADDMETHOD_ (0, "pause", m_pause);
  FLEXT_ADDMETHOD_ (0, "stop", m_stop);
  FLEXT_ADDMETHOD_ (0, "open", m_open);
  FLEXT_ADDMETHOD_ (0, "reopen", m_reopen);
  
  
  FLEXT_ADDMETHOD_ (0, "loop", m_loop_f);
  FLEXT_ADDMETHOD_ (0, "recover", m_recover);
  FLEXT_ADDMETHOD_ (0, "set_tick", m_set_tick);
  FLEXT_ADDMETHOD_ (0, "pcm_seek", m_pcm_seek);
  FLEXT_ADDMETHOD_ (0, "time_seek", m_time_seek);
  
  FLEXT_ADDMETHOD_ (0, "speed", m_src_factor);
  //FLEXT_ADDBANG(0,m_bang);
  FLEXT_CALLMETHOD (m_child);
  //post("Blocksize = %d", Blocksize());
}


readanysf::~readanysf () {

  setSys (STATE_IDLE, R_QUIT);
  cond.Signal ();
  if (readsf != NULL)
    delete (readsf);
  if (fifo != NULL)
    delete (fifo);
  if (in != NULL)
    delete (in);
  
}

void readanysf::m_bang () {
  ToOutBang (2);
}


void readanysf::m_loop_f (float f) {
  if (f == 0.0) {
    post ("readanysf~:: looping Off");
    varmutex.Lock ();
    loop = false;
    varmutex.Unlock ();
    return;
  } else {
    post ("readanysf~:: looping On");
    varmutex.Lock ();
    loop = true;
    varmutex.Unlock ();
    return;
  }
}

void readanysf::m_recover (float f) {
  if (f == 0.0) {  //dangerous!!!!
    post ("readanysf~:: stream recover Off");
    varmutex.Lock ();
    if (in != NULL) in->set_recover(false);
    varmutex.Unlock ();
    return;
  } else {
    post ("readanysf~:: stream recover On");
    varmutex.Lock ();
    if (in != NULL)  in->set_recover(true);
    varmutex.Unlock ();
    return;
  }
}

void readanysf::m_start () {
  int st = getState();
  if ( st == STATE_STARTUP ) {
    post("readanysf~:: still starting up....wait a bit please");
    m_bang();
    return;
  }
  
  if ( st != STATE_STREAM ) {
    varmutex.Lock ();
    eof = false;
    pcmseek = 0;
    timeseek = 0.0;
    //if (  st != STATE_IDLE ) 
    //	floatmsg = 0.0;
    varmutex.Unlock ();
    
    bzero ((void *) src_buffer, sizeof (src_buffer));
    if ( readsf == NULL )  { 		// nothing is opened, lets open it
      					//setSys (STATE_IDLE, R_OPEN);
      post ("readanysf~:: first select a file.");
      m_bang();
    } else {
      setSys (STATE_STREAM, R_PROCESS);
    }
  } else {
    post ("readanysf~:: already playing");
  }
  cond.Signal ();
  return;
}

void readanysf::m_time_seek (float f) {

  if (f > 0.0) {
    varmutex.Lock ();
    timeseek = (double) f;
    varmutex.Unlock ();
    setRequest (R_PROCESS);
  }
  cond.Signal ();
}

void readanysf::m_pcm_seek (int i) {

  if (i > 0)
    {
      varmutex.Lock ();
      pcmseek = (long) i;
      varmutex.Unlock ();
      setRequest (R_PROCESS);
    }
  cond.Signal ();
}

void readanysf::m_set_tick (int i) {
  varmutex.Lock ();
  if (i > 1)
    outtick = i;
  varmutex.Unlock ();
}

void readanysf::m_stop () {
  
  setSys (STATE_IDLE, R_STOP);
  varmutex.Lock ();
  floatmsg = 0.0;
  bzero ((void *) src_buffer, sizeof (src_buffer));
  varmutex.Unlock ();
  cond.Signal ();
}

void readanysf::m_pause () {
  setState (STATE_IDLE);
  cond.Signal ();
}

void readanysf::m_open (t_symbol * s) {
  //cond.Signal();
  sprintf (filename, GetString (s));
  if (getRequest () != R_OPEN) {
    //post ("readanysf~:: opening...");
    setSys (STATE_IDLE, R_OPEN);
    varmutex.Lock ();
    floatmsg = 0.0;
    pcmseek = 0;
    timeseek = 0.0;		
    format = -1;
    varmutex.Unlock ();

    ToOutFloat (2, 0.0 );       // send a 0.0 to float output
                                // we are at the beginning of the file

  } else {			// kill old file ???
    post ("readanysf~:: still initailizing old file, please be patient");
  }
  cond.Signal ();

}

void readanysf::m_reopen ( ) {
  int st = getState();
  int rq = getRequest ();
  post ("readanysf~:: reopening...");
  if ( rq == R_NOTHING && filename[0] != 0 ) {    
    setSys (STATE_IDLE, R_OPEN);
    varmutex.Lock ();
    floatmsg = 0.0;
    pcmseek = 0;
    timeseek = 0.0;		
    format = -1;
    varmutex.Unlock ();
    ToOutFloat (2, 0.0 );       // send a 0.0 to float output
                                // we are at the beginning of the file
    cond.Signal ();  
  } else {
    setSys (STATE_IDLE, R_PROCESS);
    cond.Signal ();    
  }
}


void readanysf::m_src_factor (float f) {
  if (src_is_valid_ratio (f))  {
    varmutex.Lock ();
    src_factor = (double) f;
    varmutex.Unlock ();
  }
}


void readanysf::FillFifo () {
  int ret, wret;
  
  if (readsf == NULL || in == NULL)
    return;
  varmutex.Lock ();
  if (pcmseek) {
    //post("readanysf~:: seeking..");
    if (readsf->PCM_seek (pcmseek)) {
      floatmsg = pcmseek / samplerate;
      fifo->Flush ();
      //setRequest( R_PROCESS );
    } 
    pcmseek = 0;
  }
  if (timeseek != 0.0) {
    if (readsf->TIME_seek (timeseek))
      {
	floatmsg = timeseek;
	fifo->Flush ();
	//setRequest( R_PROCESS ); 	  
      }
    timeseek = 0.0;
  }
  varmutex.Unlock ();
  
  while (fifo->FreeSpace () > INOUTSIZE) {	// leave enough space for odd chunks
    
    ret = readsf->Decode (read_buffer, READBUFFER);
    
    varmutex.Lock ();
    cachemsg = in->get_cachesize ();
    // we should fix this - this is only a relative position
    // we need to account for FIFO size

    floatmsg += (float) (ret / num_channels / samplerate);

    varmutex.Unlock ();
    
    if (ret > 0) {
      wret = fifo->Write ((void *) read_buffer, ret * FLOATSIZE);
    } else {
      //post("eof, %d", ret);
      readsf->Rewind ();
      //post("Rewound the file");
      varmutex.Lock ();
      floatmsg = 0.0;
      if (!loop) {
	//state = STATE_IDLE;      //this is premature. we check for eof in m_signal routine
	eof = true;
	varmutex.Unlock ();
	setRequest (R_NOTHING);
	break;
      } //else {
	//post("Should loop here");
	//setSys (STATE_STREAM, R_PROCESS);
      //}
      varmutex.Unlock ();
      
    }
  }
}


void readanysf::m_child () {
  int req;
  
  while ((req = getRequest ()) != R_QUIT) {
    switch (req) {
      
    case R_STOP:
      if (readsf != NULL) {
	//post("Trying to kill the readsf");
	delete (readsf);
	readsf = NULL;
	//post("killed it");
      }
      if (in != NULL) {
	delete (in);
	in = NULL;
      }
      fifo->Flush ();
      setSys (STATE_IDLE, R_NOTHING);      
      break;

    case R_OPEN:
      if (readsf != NULL && in != NULL  && strcmp (filename, in->get_filename ()) == 0) {
	// all is well here, we are just opening the same file again
	// this happens when you open a file , hit play and then try to
	// open it again
	post("opening file again...");
	fifo->Flush ();
	readsf->Rewind();
	setSys (STATE_IDLE, R_PROCESS);
      } else {
	// Set state to STARTUP at begingin of open and make sure to set
	// it to IDLE or NOTHING after succesfully opening; 
	setState( STATE_STARTUP );
	
	if (readsf != NULL) {
	  delete (readsf);
	  readsf = NULL;
	}
	if (in != NULL) {
	  delete (in);
	  in = NULL;
	  
	}
	//check if its a stream or file;  if ( strcmp( "http://")..
	if (!strncasecmp (filename, "http://", 7)) {
	  in = new InputStream ();
	  post("Opening stream: %s", filename);
	} else {
	  in = new InputFile ();
	  post("Opening file: %s", filename);
	}
	//if ( in != NULL )
	format = in->Open (filename);
	
	switch (format) {
	case FORMAT_WAVE:
	case FORMAT_AIFF:
	case FORMAT_NEXT:
	  readsf = new ReadRaw (in);
	  break;
#ifdef READ_MAD
	case FORMAT_HTTP_MP3:
	case FORMAT_MAD:
	  readsf = new ReadMad (in);
	  break;
#endif
#ifdef READ_VORBIS
	case FORMAT_VORBIS:
	case FORMAT_HTTP_VORBIS:
	  readsf = new ReadVorbis (in);
	  break;
#endif
#ifdef READ_FLAC
	case FORMAT_FLAC:
	  //post("readanysf~::  trying to make a ReadFLAC");
	  readsf = new ReadFlac (in);
	  break;
#endif
	default:
	  // probably got here 'cause we opend a stream and didn't connect
	  // InputStream will then return a -1 on Open
	  m_bang();
	  break;
	}
	
	fifo->Flush ();
	if (format >= 0 && readsf != NULL) {
	  if (readsf->Initialize ()) {
	    //post("readanysf~:: successfully initialized, format = %d", format);
	    varmutex.Lock ();
	    num_channels = readsf->get_channels ();
	    samplerate = readsf->get_samplerate ();
	    if (num_channels > 2)
	      num_channels = 2;
	    src_factor = Samplerate() / samplerate;
	    if (!src_is_valid_ratio (src_factor) )
	      src_factor = 1.0;
	    lengthinseconds = readsf->get_lengthinseconds();
	    sendout = true;
	    varmutex.Unlock();
	    setSys (STATE_IDLE, R_PROCESS);
	    fifo->Flush ();
	  } else {
	    post("Readanysf:: Couldn't initialize the file/stream!!!, sucks for you dude");
	    if (readsf != NULL)	// safe without locking ???
	      delete (readsf);
	    if ( in != NULL)  // es bueno aqui?
	      delete (in);
	    
	    setSys (STATE_IDLE, R_NOTHING);
	    readsf = NULL;
	    in = NULL;  // muy importante
	    varmutex.Lock ();
	    filename[0] = 0;
	    varmutex.Unlock ();
	    m_bang ();
	  }
	} else {	// not a recognized file type
	  post ("file not recognized, format = %d", format);
	  setSys (STATE_IDLE, R_NOTHING);
	  m_bang ();
	}
      }
      break;
    case R_PROCESS:
      FillFifo ();	//take care of mutex locking in FillFifo routine
    case R_NOTHING:
    default:
      cond.Wait ();
    }
  }
  return;
}


int readanysf::m_resample (int frames) {
  unsigned int size, out_gen;
  
  if (src_factor == 1.0) {
    size = frames * num_channels * FLOATSIZE;
    fifo->Read ((void *) src_buffer, size);
    return size / FLOATSIZE;
  }
  
  if (src_channels != num_channels) {
    src_state = src_delete (src_state);
    src_state = src_new (src_mode, num_channels, &src_error);
    src_channels = num_channels;
  }
  
  src_data.output_frames = frames;
  out_gen = 0;
  
  while (src_data.output_frames > 0) {
    if (src_data.input_frames <= 0) {
      if (src_factor > 1.0)
	size = frames * num_channels * FLOATSIZE;
      else
	size = 1024 * num_channels * FLOATSIZE;
      fifo->Read ((void *) pd_buffer, size);
      src_data.data_in = pd_buffer;
      src_data.data_out = src_buffer;
      src_data.input_frames =
	size / FLOATSIZE / num_channels;
    }
    
    src_data.src_ratio = src_factor;
    if ((src_error = src_process (src_state, &src_data))) {
      post ("readanysf~:: SRC error: %s", src_strerror (src_error));
      return 0;
    } else {
      if (src_data.output_frames_gen == 0)
	{
	  break;
	}
      src_data.data_in += src_data.input_frames_used * num_channels;
      src_data.input_frames -= src_data.input_frames_used;
      src_data.output_frames -= src_data.output_frames_gen;
      out_gen += src_data.output_frames_gen;
    }
  }
  //if (out_gen != frames) {
  //post("outgen %d, frames %d", out_gen, frames);
  //}
  
  return out_gen * num_channels;
}


FLEXT_NEW_DSP ("readanysf~", readanysf)
 
void readanysf::m_signal (int n, float *const *in, float *const *out) {
  float *outs1 = out[0];
  float *outs2 = out[1];
  t_atom lst[2];
  
  varmutex.Lock ();
  if (counttick++ > outtick) {
    SetString (lst[0], "cache");
    SetFloat (lst[1], cachemsg);
    ToOutAnything (2, GetSymbol (lst[0]), 2, lst + 1);
    counttick = 0;
  }
  varmutex.Unlock ();
  
  if (getState () == STATE_STREAM) {
    varmutex.Lock ();
    int ch = num_channels;
    int size = m_resample (n);
    bool endoffile = eof;
    int fmt = format;
    
    if (counttick == 0)	{		//lock it here? 
      //ToOutFloat (2, floatmsg - (FIFOSECONDS / ch));
      ToOutFloat ( 2, floatmsg );
    }
    varmutex.Unlock ();
    
    if (size < n * ch)
      {
	if (endoffile)  // we should do a mutex here!!!
	  {
	    
	    fifo->Flush ();
	    if (fmt >= FORMAT_HTTP_MP3) {
	      post("-------------------> End of Stream");
	      setSys (STATE_IDLE, R_STOP);
	      cond.Signal ();
	    } else {
	      setSys (STATE_IDLE, R_PROCESS);
	    }
	    m_bang ();	//  bang out at end of file
	  }
	else
	  {
	    //post("not EOF, but buffer is empty");
	  }
	//if not eof, buffer is stuck somehow so lets keep crunching. 
	//in any case fill from size returned to the end with zeros
	for (int x = size; x < (n * ch); x += ch)
	  {
	    *outs1++ = 0.0;
	    *outs2++ = 0.0;
	  }
      }
    
    
    
    if (fifo->FreeSpace () > INOUTSIZE)
      {
	if (getRequest () == R_PROCESS && !endoffile)
	  {
	    cond.Signal ();	// tell the child that the fifo is hungry
	  }
      }
    
    if (ch > 1)
      {
	for (int x = 0; x < size; x += 2)
	  {
	    *outs1++ = src_buffer[x];
	    *outs2++ = src_buffer[x + 1];
	  }
      }
    else
      {		//mono
	for (int x = 0; x < size; x++)
	  {
	    *outs1++ = src_buffer[x];
	    *outs2++ = src_buffer[x];
	  }
      }
    
  }
  else
    {			// we're not streaming.  just zero out the audio outlets
      varmutex.Lock ();
      if (sendout)
	{
	  SetString (lst[0], "length");
	  SetFloat (lst[1], lengthinseconds);
	  ToOutAnything (2, GetSymbol (lst[0]), 2, lst + 1);
	  SetString (lst[0], "rate");
	  SetFloat (lst[1], (float) samplerate);
	  ToOutAnything (2, GetSymbol (lst[0]), 2, lst + 1);
	  sendout = false;
	}
      varmutex.Unlock ();
      
      while (n--)
	{
	  *outs1++ = 0.0;
	  *outs2++ = 0.0;
	}
    }
}



int readanysf::getState () {
	int i;
	sysmut.Lock ();
	i = state;
	sysmut.Unlock ();
	return i;
}

int readanysf::getRequest () {
	int i;
	sysmut.Lock ();
	i = request;
	sysmut.Unlock ();
	return i;
}

void readanysf::setSys (int sys, int req) {
	sysmut.Lock ();
	state = sys;
	request = req;
	sysmut.Unlock ();
	return;
}

void readanysf::setState (int i) {
	sysmut.Lock ();
	state = i;
	sysmut.Unlock ();
	return;
}

void readanysf::setRequest (int i) {
	sysmut.Lock ();
	request = i;
	sysmut.Unlock ();
	return;
}
