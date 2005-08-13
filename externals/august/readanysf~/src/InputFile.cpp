#include "InputFile.h"
#include <iostream.h>

InputFile::InputFile () {
  fd = 0;
  format = -1;
  recover = false;
}

InputFile::~InputFile () {
}

// returns the file type, either WAV, MP3, OGG, etc. see input.h
int InputFile::Open (const char *pathname) {
  char buf[18];
  filename = pathname;
  
  fd = open (pathname, O_RDONLY);
  
  if (fd == -1) {
    // error opening the file, no dice
    return -1;
  }
  
  int bytesread = read (fd, buf, 16);
  
  if (bytesread < 4) {
    // fill is too fucking small dude
    close (fd);
    return -1;
  }
  

  if (!strncmp (buf, ".snd", 4))
    {
      //rewind the stream
      if ((lseek (fd, 0, SEEK_SET)) == -1)
	return -1;
      return format = FORMAT_NEXT;	//, bigendian = 1;
    }
  else if (!strncmp (buf, "dns.", 4))
    {
      //rewind the stream
      if ((lseek (fd, 0, SEEK_SET)) == -1)
	return -1;
      return format = FORMAT_NEXT;	//, bigendian = 0;
    }
  else if (!strncmp (buf, "RIFF", 4))
    {
      if (bytesread < 12 || strncmp (buf + 8, "WAVE", 4))
	{
	  cout << "bad header ?" << endl;
	  return -1;
	}
      //rewind the stream
      if ((lseek (fd, 0, SEEK_SET)) == -1)
	return -1;
      return format = FORMAT_WAVE;	//, bigendian = 0;
    }
  else if (!strncmp (buf, "FORM", 4))
    {
      if (bytesread < 12 || strncmp (buf + 8, "AIFF", 4))
	return -1;	//goto badheader;
      //rewind the stream
      if ((lseek (fd, 0, SEEK_SET)) == -1)
	return -1;
      return format = FORMAT_AIFF;	//, bigendian = 1;
    }
  
  else if (!strncmp (buf, "OggS", 4)) {
    //rewind the stream
    if ((lseek (fd, 0, SEEK_SET)) == -1)
      return -1;
#ifdef READ_VORBIS
    return format = FORMAT_VORBIS;
#else 
      return -1;
#endif
    }
  
  else if (!strncmp (buf, "ID3", 3))
    {
      //rewind the stream
      if ((lseek (fd, 0, SEEK_SET)) == -1)
	return -1;
#ifdef READ_MAD
      return format = FORMAT_MAD;
#else 
      return -1;
#endif
    }
  
  else if (!strncasecmp (buf, "FLAC", 4))
    {
      // } else if( !strncasecmp(thefile+strlen(thefile)-4,".fla",4) ) {
      //rewind the stream
      if ((lseek (fd, 0, SEEK_SET)) == -1)
	return -1;
#ifdef READ_FLAC
      return format = FORMAT_FLAC;
#else 
      return -1;
#endif
    }
  else
    {
      unsigned int sync;
      sync = (unsigned char) buf[0];
      sync = sync << 3;
      sync |= ((unsigned char) buf[1] & 0xE0) >> 5;
      if (sync == 0x7FF)
	{
	  //rewind the stream
	  if ((lseek (fd, 0, SEEK_SET)) == -1)
	    return -1;
#ifdef READ_MAD
	  return format = FORMAT_MAD;
#else 
	  return -1;
#endif
	}
      else if (!strncasecmp
	       (pathname + strlen (pathname) - 4, ".mp3", 4))
	{
	  //trust that its mp3
	  //rewind the stream
	  if ((lseek (fd, 0, SEEK_SET)) == -1)
	    return -1;
	  cout << "doesnt seem like its an mp3, but if you say so" << endl;
#ifdef READ_MAD
	  return format = FORMAT_MAD;
#else 
	  return -1;
#endif  
	}
    }
  
  
  
  return -1;
  
}

int
InputFile::Close ()
{
	return close (fd);
}

int
InputFile::Read (void *buf, unsigned int count)
{
	return read (fd, buf, count);
}

long
InputFile::SeekSet (long offset)
{
	return lseek (fd, offset, SEEK_SET);
}

long
InputFile::SeekCur (long offset)
{
	return lseek (fd, offset, SEEK_CUR);
}

long
InputFile::SeekEnd (long offset)
{
	return lseek (fd, offset, SEEK_END);
}
