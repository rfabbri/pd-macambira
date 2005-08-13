#ifndef _INPUTSTREAM_H_
#define _INPUTSTREAM_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Input.h"
#include "Fifo.h"

#include <string>
#include <pthread.h>
#include <dlfcn.h>


#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifdef UNIX
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define SOCKET_ERROR -1
#else
#include <winsock.h>
#endif

using namespace std;

class InputStream : public Input {
 public:
  InputStream ();
  virtual ~InputStream();
  
  virtual int Open( const char *pathname );
  virtual int Close();
  virtual int Read( void *buf, unsigned int count );
  
  virtual long SeekSet ( long offset );
  virtual long SeekCur ( long offset );
  virtual long SeekEnd ( long offset );
  virtual float get_cachesize(); 
  virtual bool get_recover( ) { return recover; }
 
  pthread_mutex_t *get_mutex() { return &mut;}
  pthread_cond_t *get_condition(){ return &cond;}
  
  void set_threaded( bool b ) { threaded = b;} 
  
  Fifo * get_fifo() { return infifo;}
  bool get_quit() { return quit;}
  
  int socket_connect ( ); 

 private:
  //int socket_connect (string hostname, string mountpoint, int portno); 
  //int socket_connect (char *hostname, char *mountpoint, int portno);  
  // connects to socket and checks for ice or shout
  // returns type of stream(ogg, mp3) or -1 for failure
  
  string ParseHttp( string str, string parse ); // parse x-audio* vars from icecast
  int SetUrl (const char *url);  	// breaks http://server:port/mount down to hostname,port,mountpoint
  // return 1 for success, 0 for failure
  
  int get_line( char * str, int sock, int maxget);
  
  Fifo *infifo;		// fifo for thread buffering
  string hostname;    // hostname of URL
  string mountpoint;  // mountpoint for Icecast URL
  int port;			// port number URL
  bool threaded;		// if thread is running or not, true if running 
  bool quit;			// if we should quit thread or not
  
  pthread_mutex_t mut;
  pthread_cond_t cond;
  pthread_t childthread;
  
};
#endif
