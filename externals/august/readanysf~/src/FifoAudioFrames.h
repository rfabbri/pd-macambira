#ifndef _FIFOAUDIOFRAMES_H_
#define _FIFOAUDIOFRAMES_H_

#include <string.h> // memcpy
#include <stdio.h>
#include <pthread.h>

#ifndef _AVDEC_H_
#define _AVDEC_H_
extern "C" {
#include <gmerlin/avdec.h>
}
#endif

class FifoAudioFrames {
	public:
		FifoAudioFrames(int s, gavl_audio_format_t * format) ; 
		~FifoAudioFrames();// { delete [] fifoPtr; }
		bool Append( gavl_audio_frame_t * af);
		bool Get( gavl_audio_frame_t * af) ;  // pop an element off the fifo
		void Flush();
		//void Dump(char *c);
		bool FreeSpace();
		bool isEmpty();
		bool isFull();
		void setDebug( bool b); 
		gavl_audio_format_t * getFormat();
		float getSizePercentage();
	private:
		int size ;  // Number of elements on FifoAudioFrames
		int start ;
		int end ;
		int count;
		gavl_audio_frame_t ** fifoPtr ;  
		gavl_audio_format_t * format;  
		pthread_mutex_t mut;
} ;


#endif

