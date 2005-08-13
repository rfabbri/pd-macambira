#ifndef _INPUT_H_
#define _INPUT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>		// open
#include <sys/stat.h>		// open
#include <fcntl.h>		// open
#include <unistd.h>		// read
#include <netdb.h>		// for gethostbyname
#include <sys/socket.h>		// socket

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


#include <string>  // save filename
using namespace std;
#include "generic.h"

class Input {
 public:
  Input();
  virtual ~ Input();
  
  virtual int Open(const char *pathname);	// open file or stream and return formt
  virtual int Close();	// close or disconnect
  virtual int Read(void *buf, unsigned int count);	//read into buf count times.
  
  virtual long SeekSet(long offset);	// lseek using SEEK_SET
  virtual long SeekCur(long offset);	// lseek using SEEK_CUR
  virtual long SeekEnd(long offset);	// lseek using SEEK_END
  //int getEof() { return  eof(fd); }
  virtual float get_cachesize();      // return amount of buffer that is used.  0.0 for InputFile
  
  virtual bool get_recover( ) { return recover; }
  void set_recover( bool x) { recover =x;}
  

  void SetVerbosity(int d) { verbosity = d; } // set debug level 0-3 protected:
  int get_fd() { return fd;}
  int get_format() { return format;}
  const char * get_filename() { return filename.c_str(); }
  
 protected:
  int fd;			//file descriptor for files and sockets
  int format;		//what format?  OGG,MP3,NEXT etc. see defines above
  int verbosity;	//how much debugging/info to print
  //we need to be able to set this dynamically for http 
  bool recover;        // whether to recover connections on the net 
  string filename;    // store the path/filename of what is opened for reading
};
#endif
