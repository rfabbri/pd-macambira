#ifndef _INPUTFILE_H_
#define _INPUTFILE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "Input.h"

class InputFile : public Input {
 public:
  InputFile();
  virtual ~InputFile();
  
  virtual int Open( const char *pathname );
  virtual int Close();
  virtual int Read( void *buf, unsigned int count );
  
  virtual long SeekSet ( long offset );
  virtual long SeekCur ( long offset );
  virtual long SeekEnd ( long offset );
  virtual float get_cachesize() { return 0.0; }; 
  virtual bool get_recover( ) { return false; }
 private:
		 	
	
};
#endif
