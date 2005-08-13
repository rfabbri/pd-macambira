#include "InputStream.h"
#include <iostream>		// cout, cerr
#include <string>		// strcpy, etc.

using namespace std;

int receive (int fd, unsigned char *rcvbuffer, int size) {
  fd_set set;
  struct timeval tv;
  int ret = -1;
  int selret = -1;
  tv.tv_sec = 1;
  tv.tv_usec = 500;
  FD_ZERO(&set);
  FD_SET(fd, &set);

  selret= select(fd +1, &set, NULL, NULL, &tv);
  if ( selret > 0 ) {
    // we can now be certain that ret will return something.
    ret = recv (fd, rcvbuffer, size, 0);
    if (ret < 0 ) {
      cerr << "InputStream:: receive error" << endl;
      return -1;
    }
    return ret;
  } else if ( selret == -1 ){
    cerr << "InputStream:: receive: select timed out, returned "<< selret << endl;
    return -1;
  }
  // return zero...means keep  on selecting
  return 0;
}

void * fill_infifo (void *zz) {
  int ret, wret, last_type, last_state;
  unsigned char tmp[SOCKET_READSIZE];
  InputStream *instream = (InputStream *) zz;
	
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &last_type);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
  
  pthread_mutex_lock (instream->get_mutex ());
  instream->set_threaded(true);
  pthread_cond_signal (instream->get_condition ());
  pthread_mutex_unlock (instream->get_mutex ());	
  //cout << "signalled parent thread" << endl;
  while ( 1 ) {
    while (instream->get_fifo()->FreeSpace() > SOCKET_READSIZE + 576) {
      // it's possible to hang here, watch this 
      if ( instream->get_quit() ) break;
      ret = receive (instream->get_fd() , tmp, SOCKET_READSIZE);
      if (ret > 0){
	wret = instream->get_fifo()->Write ((void *) tmp, ret * sizeof (unsigned char));
      } else if ( ret == -1){
	// got -1 on recieve. ...this means select failed and there is no data
	cerr << "InputStream:: fill_infifo: select failed, our socket must have died" << endl;
	if ( instream->get_recover() ) {
	  cout << "InputStream:: try to reconnect to server..." << endl;
	  if ( instream->socket_connect () < 0 ) {
	    cout << "InputStream:: tried to recover stream but socket connect failed" <<endl;
	    break;
	  }
	} else {
	  break;
	}
      } else {
	// got 0?? on recieve. ...select timed out, cause there wasn't any data
	//cerr << "InputStream: fill_infifo: select timed out, no data.  ret = " << ret << endl;
	// keep on truckin' until we get a select and recv that sticks
      }
    }
    if ( instream->get_quit() ) break;
    //cerr << "InputStream: fifo is full" << endl;
    pthread_mutex_lock (instream->get_mutex ());
    pthread_cond_wait (instream->get_condition (), instream->get_mutex ());
    pthread_mutex_unlock (instream->get_mutex ());	
    
  }
  instream->set_threaded(false);
  return NULL;
}

InputStream::InputStream () {
  fd = 0;
  format = -1;
  threaded = false;
  port = 0;
  infifo = new Fifo( STREAM_FIFOSIZE );
  quit = false;
  recover = false;
  verbosity = 1;
  pthread_mutex_init(&mut, 0);
  pthread_cond_init(&cond, 0);
}

InputStream::~InputStream () {
  quit = true;
  void *status;
  
  if (threaded ) 
    {
      //cout << "canceling thread" << endl;
      pthread_cond_signal ( &cond );
      //pthread_cancel( childthread );
      pthread_join( childthread, &status);
      threaded = -1;
      //cout << "thread canceled" << endl;
    }
  delete infifo;
}


// Open() returns the file type, either WAV, MP3, OGG, etc. see input.h
// this is a blocking call, in order to use the open command we need
// to figure out the format (ogg, mp3, etc).  this has to block.

int InputStream::Open (const char *pathname) {
  int rettype, thret;
  filename = pathname;
  if (verbosity > 1) 
    cout << "trying to open a socket connection" << endl;
  SetUrl (pathname);

  rettype = socket_connect( ); //hostname, mountpoint, port );
  if (rettype < 0) {  // couldn't connect or got a bad filetype
    cerr << "InputStream:: Couldn't connect or got a bad filetype" <<endl;
    return -1;
  }
  // start thread here to fill infifo
  //cout << "creating thread" << endl;
  thret = pthread_create(&childthread, NULL, fill_infifo, (void *)this);
  if ( thret!= 0 )
    return -1;
  //wait for thread to be created.
  pthread_mutex_lock( &mut );
  while ( !threaded )
    pthread_cond_wait( &cond, &mut );
  pthread_mutex_unlock( &mut );
  //cout << "threaded = " << threaded << "rettype = " << rettype << endl;
  
  while ( get_fifo()->UsedSpace () < (unsigned int)(STREAM_FIFOSIZE / 2) ) {
    usleep(100);  // we need to wait here for some of the input buffer to fill.
    //cout << "waiting for HTTP buffer to fill  " << get_fifo()->UsedSpace () <<endl;
  }
  /*  if ( rettype == FORMAT_HTTP_VORBIS) {
    cout << "---------->Lets try to flush fifo and fill again" <<endl;
    get_fifo()->Flush();
    pthread_cond_signal ( &cond );
    while ( get_fifo()->UsedSpace () < (unsigned int)(8500*2) ) {
      usleep(1000);  // we need to wait here for some of the input buffer to fill.
       cout << "waiting for HTTP buffer to fill  " << get_fifo()->UsedSpace () <<endl;
    }
    }*/
  return rettype;
}

int InputStream::Close () {
  // returns zero on success, or -1 if an error occurred
  return sys_closesocket (fd);
}

int InputStream::Read (void *buf, unsigned int count) {
  //if (quit) return -1;  	// return negative if the childthread exits
  //and sets quit true
  infifo->Read( buf ,  count);
  pthread_cond_signal ( &cond );
  return count;
}

long InputStream::SeekSet (long offset) {
  return -1;
}

long InputStream::SeekCur (long offset) {
  return -1;
}

long InputStream::SeekEnd (long offset) {
  return -1;
}

int InputStream::get_line( char * str, int sock, int maxget) {
  int i = 0;
  while(i < maxget - 1) {
    
  if ( recv(sock, str + i, 1, 0) <=  0 )  {
    cerr << "InputStream : could not read from socket" << endl;
    sys_closesocket(sock);
    return (-1);
  }
  if ( str[i] == '\n' )
    break;
  if( str[i] == 0x0A)  /* leave at end of line */
    break;
  if ( str[i] != '\r' )
    i++;
  }
  str[i] = '\0';
  return i;
}

// connect to shoutcast server 
int InputStream::socket_connect ( ) {
  struct sockaddr_in server;
  struct hostent *hp;
  int flags;
  // variables used for communication with server 
  string strtmp;		// tmp string for manipulating server strings
  string line, parsed;	        // string for parsing x-audicast vars
  char strret[STRBUF_SIZE];	// returned string from server
  
  fd_set fdset;
  struct timeval tv;
  int relocate = false;
  std::string::size_type ret;
  

  fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) {
    if (verbosity > 0)
      cerr << "InputStream: internal error while attempting to open socket" << endl;
    return (-1);
  }

  //connect socket using hostname 
  server.sin_family = AF_INET;
  hp = gethostbyname (hostname.c_str ());
  
  if (hp == NULL) {
    if (verbosity > 0)
      cerr << "InputStream:: bad host?" << endl;
    sys_closesocket (fd);
    return (-1);
  }


  memcpy ((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
  // assign client port number 
  server.sin_port = htons ((unsigned short) port);
  

  flags = fcntl( fd, F_GETFL, 0);
  fcntl( fd, F_SETFL, FNDELAY);     // make this socket's calls non-blocking 
  //  fcntl( fd, F_SETFL, flags | O_NONBLOCK);

  if (connect( fd, (struct sockaddr *) &server, sizeof(server) ) == -1 && errno != EINPROGRESS) { 
    /*
     * If non-blocking connect() couldn't finish, it returns
     * EINPROGRESS.  That's OK, we'll take care of it a little
     * later, in the select().  But if some other error code was
     * returned there's a real problem...
     */
    sys_closesocket (fd);
    return(-1);
  
  } else {
    //cout << " error is EINPROGRESS " << endl;
    FD_ZERO (&fdset);
    FD_SET (fd, &fdset);
    tv.tv_sec = 1;		/* seconds */
    tv.tv_usec = 0;	/* microseconds */

    //  you want to do the select on the WRITEablity of the socket, HTTP expects a get
    // command, so make sure to pass args to both read and write fdset
    switch (select( fd+1 , &fdset, &fdset, NULL, &tv) ) {
      /*
       * select() will return when the socket is ready for action,
       * or when there is an error, or the when timeout specified
       * using tval is exceeded without anything becoming ready.
       */
      
    case 0:     // timeout 
      //do whatever you do when you couldn't connect
      cout << "InputStream:: connect timed out, bailing..." <<endl;
      sys_closesocket (fd);
      return (-1);
      break;
    case -1:    // error 
      cout << "InputStream:: connection error, bailing..." <<endl;
      sys_closesocket (fd);
      return (-1);
      break;
    default:    // your file descriptor is ready... 
      fcntl( fd, F_SETFL, flags);	
      break;
    }
  }


  // build up stuff we need to send to server 
  // should change it to send GET and then read the header until 
  // it recieves "\n\n", and then parse the header
  strtmp = "GET /" + mountpoint + " HTTP/1.0 \r\nHost: " + hostname +
    "\r\nUser-Agent: Readanysf~ 0.5\r\nAccept: */*\r\n\r\n";
  if (verbosity > 2)
    cout << "sending...." << strtmp << endl;
  if (send (fd, strtmp.c_str (), strtmp.length (), 0) < 0)
    {
      if (verbosity > 0)
	cerr << "InputStream:: could not contact server... " << endl;
      return (-1);
    }
  
  get_line( strret , fd , STRBUF_SIZE ) ;
  strtmp = strret;
  //cout << strtmp << endl;

  ret = strtmp.find ("HTTP", 0);
  if (ret != string::npos) {  /* seems to be IceCast server */
    ret = strtmp.find ("302", 0);
    if (ret != string::npos) {
      if (verbosity > 0)
	cerr << "InputStream::need to relocate...not implemented yet, bailing" << endl;
      relocate = true;
      return (-1);
    }
    ret = strtmp.find ("200", 0);
    if (ret == string::npos) {
      if (verbosity > 0)
	cerr <<  "InputStream : cannot connect to the (default) stream" << endl;
      sys_closesocket (fd);
      return (-1);
    }
    if (verbosity > 2)
      cerr << "everything seems to be fine, now lets parse the server strings" << endl;
    
    // go through header 10 times, line by line. this should be enough.
    // we only need the Content-Type for checking if it's mp3 or vorbis
    for (int i = 0; i < 10; i++)
      {
	get_line( strret , fd , STRBUF_SIZE ) ;
	line = strret;
	//cout << " Got line: " << line << endl; 
	// we could probable parse the Server flag for icecast 1
	// or 2 
	// server type, but that is more trouble than what its
	// worth
	parsed = ParseHttp (line, "Server");
	if (!parsed.empty ())
	  if (verbosity > 1)
	    cout << "server" << parsed << endl;
	parsed = ParseHttp (line, "Content-Type");
	if (!parsed.empty ()) {
	  std::string::size_type n;
	  if (verbosity > 1)  cout << "Content-Type " << parsed << endl;
	  n = parsed.find ("ogg");
	  if (n != string::npos) {
	    if (verbosity > 1)
	      cout << "we have an ogg vorbis stream" << endl;
	    format = FORMAT_HTTP_VORBIS;
	    break;	// found what we were looking for
	  }
	  n = parsed.find ("mpeg");
	  if (n != string::npos){
	    if (verbosity > 1)
	      cout << "we have an Mp3 stream" << endl;
	    format = FORMAT_HTTP_MP3;
	    break;	// found what we were looking for
	  }
	}
	
      }
    
  } else {
    //cout << "not HTTP, could be ICY for shoutcast" << endl;
    ret = strtmp.find ("ICY 200 OK", 0);
    if (ret != string::npos) {  /* seems to be IceCast server */
      // we are only interested in mp3 or ogg content type
      cout << "we have an ICY Mp3 stream" << endl;
      format = FORMAT_HTTP_MP3;

    } else {
      cout << "Neither a Shoutcast or Icecast stream, hafta bail." << endl;
      return -1;
    }
	
  }

  
  return (format);
}

// parses string "str" for the item "parse", if found return the part of 
// "str" after the ":" ex.

string InputStream::ParseHttp (string str, string parse) {
  std::string::size_type ret;
  
  ret = str.find (parse, 0);
  if (ret != string::npos) {
    return str.substr (parse.length () + 1, str.length () - parse.length ());
  }
  return "";
}



int InputStream::SetUrl (const char *url) {
  string strtmp = url;
  std::string::size_type p1, p2, tmp;
  
  tmp = strtmp.find ("http://");
  if (tmp < 0 || tmp > strtmp.length ())
    return 0;
  
  tmp = tmp + 7;
  strtmp = strtmp.substr (tmp, strtmp.length () - tmp);
  
  p2 = strtmp.find ("/", 0);
  if (p2 < 0 || p2 > strtmp.length ()) {
    p2 = strtmp.length();
    //cout << "didn't find the / in the url" <<endl;
    p1 = strtmp.find (":");
    if (p1 < 0 || p1 > strtmp.length ()) {
      port = 80;	// set port to default 80
      hostname = strtmp;
      mountpoint = " "; // send blank mntpoint
    } else {
      // found the ":", setting port number
      port = atoi (strtmp.substr (p1 + 1, p2 - p1 -1).c_str ());
      hostname = strtmp.substr (0, p1);
      mountpoint = " ";  // send blank mntpoint
    }
    return 1;	// didn't find the / in the URL
  }
  p1 = strtmp.find (":");
  if (p1 < 0 || p1 > strtmp.length ()) {
    // didn't find a ":", that 
    // 
    // means there's no port
    port = 80;	// set port to default 8000
    hostname = strtmp.substr (0, p2);
    mountpoint = strtmp.substr (p2 + 1, strtmp.length () - p2);
    if (verbosity > 1)
      cerr << "port is: default " << port << endl;
  } else {
    // found the ":", setting port number
    port = atoi (strtmp.substr (p1 + 1, p2 - p1 - 1).c_str ());
    hostname = strtmp.substr (0, p1);
    mountpoint = strtmp.substr (p2 + 1, strtmp.length () - p2);
  }
  
  if (verbosity > 2 ) {
    cout << "port: " << port << endl;
    cout << "hostname: " << hostname << endl;
    cout << "mount: " << mountpoint << endl;
  }
  return 1;
}

float InputStream::get_cachesize() {
  return (float)infifo->UsedSpace();
}
// ~ parsed = ParseHttp( line, "x-audiocast-location" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-admin" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-server-url" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-mount" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-name" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-description" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-url:http" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-genre" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-bitrate" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
// ~ parsed = ParseHttp( line, "x-audiocast-public" ) ;
// ~ if ( !parsed.empty()) cout << parsed << endl;
