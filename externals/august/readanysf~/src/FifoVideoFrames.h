#ifndef _FIFOVIDEOFRAMES_H_
#define _FIFOVIDEOFRAMES_H_

#include <string.h> // memcpy
#include <stdio.h>
#include <pthread.h>

#ifndef _AVDEC_H_
#define _AVDEC_H_
extern "C" {
#include <gmerlin/avdec.h>
}
#endif

class FifoVideoFrames {
	public:
		FifoVideoFrames(int s, gavl_video_format_t * format) ; 
		~FifoVideoFrames();// { delete [] fifoPtr; }
		bool Append( gavl_video_frame_t * af);
		bool Get( gavl_video_frame_t * af) ;  // pop an element off the fifo
		bool Get( ) ;  // discard an element off the fifo
		void Flush();
		//void Dump(char *c);
		bool FreeSpace();
		bool isEmpty();
		bool isFull();
		void setDebug( bool b); 
		gavl_video_format_t * getFormat();
		int getSize();
		float getSizePercentage();
	private:
		int size ;  // Number of elements on FifoVideoFrames
		int start ;
		int end ;
		int count;
		gavl_video_frame_t ** fifoPtr ;  
		gavl_video_format_t * format;  
		pthread_mutex_t mut;
} ;


#endif

