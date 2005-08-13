#ifndef _GENERIC_H_
#define _GENERIC_H_


#define FLOATSIZE sizeof(float)
#define SHORTSIZE sizeof(short)
#define CHUNKSIZE 4096*FLOATSIZE
#define WAVCHUNKSIZE 1024
//#define MAD_CHUNKSIZE 2016*FLOATSIZE //576
#define INOUTSIZE (CHUNKSIZE*4)+1

#define FIFOSIZE (CHUNKSIZE*16)
#define FIFOSECONDS (FIFOSIZE/FLOATSIZE/44100) //assume samplerate
#define READBUFFER (1024*16)

#define R_NOTHING   0
#define R_OPEN      1
#define R_CLOSE     2
#define R_QUIT      3
#define R_PROCESS   4
#define R_STOP      5 

#define STATE_IDLE			0
#define STATE_STARTUP		1
#define STATE_STREAM		2
#define STATE_IDLE_CLOSED	3

#define FORMAT_WAVE         0
#define FORMAT_AIFF         1
#define FORMAT_NEXT         2
#define FORMAT_VORBIS       3
#define FORMAT_MAD          4
#define FORMAT_FLAC         5
#define FORMAT_HTTP_MP3     6
#define FORMAT_HTTP_VORBIS  7



#define STREAM_FIFOSIZE       (32 * 1152)
#define SOCKET_READSIZE       1024 //1152/4
#define     sys_closesocket close  // windows uses sys_closesocket 
#define     STRBUF_SIZE             1024

//#define STREAM_BUFFERSIZE (4 * 1152)
//#define FRAME_RESERVE 2000
//#define     STRDUP strdup

#endif
