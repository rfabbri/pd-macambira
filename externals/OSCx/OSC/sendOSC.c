/*
Copyright (c) 1996,1997.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for educational, research, and not-for-profit purposes, without
fee and without a signed licensing agreement, is hereby granted, provided that
the above copyright notice, this paragraph and the following two paragraphs
appear in all copies, modifications, and distributions.  Contact The Office of
Technology Licensing, UC Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley,
CA 94720-1620, (510) 643-7201, for commercial licensing opportunities.

Written by Matt Wright, The Center for New Music and Audio Technologies,
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
     ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
     REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
*/

/* sendOSC.c

    Matt Wright, 6/3/97
   based on sendOSC.c, which was based on a version by Adrian Freed

    Text-based OpenSoundControl client.  User can enter messages via command
    line arguments or standard input.

    Version 0.1: "play" feature
   Version 0.2: Message type tags.
   


   pd
   -------------
   -- added bundle stuff to send. jdl 20020416
   -- tweaks for Win32    www.zeggz.com/raf	13-April-2002
   -- ost_at_test.at + i22_at_test.at, 2000-2002
      modified to compile as pd externel

*/

//#define VERSION "http://cnmat.berkeley.edu/OpenSoundControl/sendOSC-0.1.html"
#define MAX_ARGS 2000
#define SC_BUFFER_SIZE 64000

/*
compiling:
        cc -o sendOSC sendOSC.c htmsocket.c OpenSoundControl.c OSC_timeTag.c
*/

#ifdef WIN32
	#include "m_pd.h"
	#include "OSC-client.h"
	#include "htmsocket.h"
	#include "OSC-common.h"
	#include <winsock2.h>	
	#include <io.h>    
	#include <errno.h>
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/types.h>
#else
	#include "m_pd.h"
	//#include "x_osc.h"
	#include "OSC-client.h"
	#include "htmsocket.h"

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <netdb.h>
#endif

///////////////////////
// from sendOSC

typedef struct {
    //enum {INT, FLOAT, STRING} type;
    enum {INT_osc, FLOAT_osc, STRING_osc} type;
    union {
        int i;
        float f;
        char *s;
    } datum;
} typedArg;

void CommandLineMode(int argc, char *argv[], void *htmsocket);
//void InteractiveMode(void *htmsocket);
OSCTimeTag ParseTimeTag(char *s);
void ParseInteractiveLine(OSCbuf *buf, char *mesg);
typedArg ParseToken(char *token);
int WriteMessage(OSCbuf *buf, char *messageName, int numArgs, typedArg *args);
void SendBuffer(void *htmsocket, OSCbuf *buf);
void SendData(void *htmsocket, int size, char *data);
void fatal_error(char *s);
void send_complain(char *s, ...);

//static void *htmsocket;
static int exitStatus = 0;  
static int useTypeTags = 0;

static char bufferForOSCbuf[SC_BUFFER_SIZE];


/////////
// end from sendOSC

static t_class *sendOSC_class;

typedef struct _sendOSC
{
  t_object x_obj;
  int x_protocol;      // UDP/TCP (udp only atm)
  t_int x_typetags;    // typetag flag
  void *x_htmsocket;   // sending socket
  int x_bundle;        // bundle open flag
  OSCbuf x_oscbuf[1];  // OSCbuffer
  t_outlet *x_bdpthout;// bundle-depth floatoutlet
} t_sendOSC;

static void *sendOSC_new(t_floatarg udpflag)
{
    t_sendOSC *x = (t_sendOSC *)pd_new(sendOSC_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_htmsocket = 0;		// {{raf}}
    // set udp
    x->x_protocol = SOCK_STREAM;
    // set typetags to 1 by default
    x->x_typetags = 1;
    // bunlde is closed
    x->x_bundle   = 0;
    OSC_initBuffer(x->x_oscbuf, SC_BUFFER_SIZE, bufferForOSCbuf);
    x->x_bdpthout = outlet_new(&x->x_obj, 0); // outlet_float();
    //x->x_oscbuf   =
    return (x);
}


void sendOSC_openbundle(t_sendOSC *x)
{
  if (x->x_oscbuf->bundleDepth + 1 >= MAX_BUNDLE_NESTING ||
      OSC_openBundle(x->x_oscbuf, OSCTT_Immediately()))
    {
    send_complain("Problem opening bundle: %s\n", OSC_errorMessage);
    return;
  }
  x->x_bundle = 1;
  outlet_float(x->x_bdpthout, (float)x->x_oscbuf->bundleDepth);
}

static void sendOSC_closebundle(t_sendOSC *x)
{
  if (OSC_closeBundle(x->x_oscbuf)) {
    send_complain("Problem closing bundle: %s\n", OSC_errorMessage);
    return;
  }
  outlet_float(x->x_bdpthout, (float)x->x_oscbuf->bundleDepth);
  // in bundle mode we send when bundle is closed?
  if(!OSC_isBufferEmpty(x->x_oscbuf) > 0 && OSC_isBufferDone(x->x_oscbuf)) {
    // post("x_oscbuf: something inside me?");
    if (x->x_htmsocket) {
      SendBuffer(x->x_htmsocket, x->x_oscbuf);
    } else {
      post("sendOSC: not connected");
    }
    OSC_initBuffer(x->x_oscbuf, SC_BUFFER_SIZE, bufferForOSCbuf);
    x->x_bundle = 0;
    return;
  }
  // post("x_oscbuf: something went wrong");
}

static void sendOSC_settypetags(t_sendOSC *x, t_float *f)
 {
   x->x_typetags = (int)f;
   post("sendOSC.c: setting typetags %d",x->x_typetags);
 }


static void sendOSC_connect(t_sendOSC *x, t_symbol *hostname,
			    t_floatarg fportno)
{
	int portno = fportno;
	/* create a socket */

	//	make sure handle is available
	if(x->x_htmsocket == 0) {
		//
		x->x_htmsocket = OpenHTMSocket(hostname->s_name, portno);
		if (!x->x_htmsocket)
			post("Couldn't open socket: ");
		else {
			post("connected to port %s:%d (hSock=%d)",  hostname->s_name, portno, x->x_htmsocket);
			outlet_float(x->x_obj.ob_outlet, 1);
		}
	}
	else 
		perror("call to sendOSC_connect() against UNavailable socket handle");
}

void sendOSC_disconnect(t_sendOSC *x)
{
  if (x->x_htmsocket)
    {
      post("disconnecting htmsock (hSock=%d)...", x->x_htmsocket);
      CloseHTMSocket(x->x_htmsocket);
	  x->x_htmsocket = 0;	// {{raf}}  semi-quasi-semaphorize this
      outlet_float(x->x_obj.ob_outlet, 0);
    }
  else {
	perror("call to sendOSC_disconnect() against unused socket handle");
  }
}

void sendOSC_senduntyped(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv)
{
  char* targv[MAXPDARG];
  char tmparg[MAXPDSTRING];
  char* tmp = tmparg;
  //char testarg[MAXPDSTRING];
  int c;

  post("sendOSC: use typetags 0/1 message and plain send method so send untypetagged...");
  return;

  //atom_string(argv,testarg, MAXPDSTRING);
  for (c=0;c<argc;c++) {
    atom_string(argv+c,tmp, 80);
    //    post ("sendOSC: %d, %s",c, tmp);
    targv[c] = tmp;
    tmp += strlen(tmp)+1;
    //post ("sendOSC: %d, %s",c, targv[c]);
  }
  // this sock needs to be larger than 0, not >= ..
  if (x->x_htmsocket)
    {
      CommandLineMode(argc, targv, x->x_htmsocket);
      //      post("test %d", c);
    }
  else {
    post("sendOSC: not connected");
    //    exit(3);
  }
}

//////////////////////////////////////////////////////////////////////
// this is the real and only sending routine now, for both typed and undtyped mode.

static void sendOSC_sendtyped(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv)
{
  char* targv[MAX_ARGS];
  char tmparg[MAXPDSTRING];
  char* tmp = tmparg;
  int c;

  char *messageName;
  char *token;
  typedArg args[MAX_ARGS];
  int i,j;
  int numArgs = 0;

  messageName = "";
#ifdef DEBUG
  post ("sendOSC: messageName: %s", messageName);
#endif


  
  for (c=0;c<argc;c++) {
    atom_string(argv+c,tmp, 80);

#ifdef DEBUG
    // post ("sendOSC: %d, %s",c, tmp);
#endif

    targv[c] = tmp;
    tmp += strlen(tmp)+1;

#ifdef DEBUG
    // post ("sendOSC: %d, %s",c, targv[c]);
#endif
  }

  // this sock needs to be larger than 0, not >= ..
  if (x->x_htmsocket > 0)
    { 
#ifdef DEBUG
    post ("sendOSC: type tags? %d", useTypeTags);
#endif 

      messageName = strtok(targv[0], ",");
      j = 1;
      for (i = j; i < argc; i++) {
	token = strtok(targv[i],",");
	args[i-j] = ParseToken(token);
#ifdef DEBUG
	printf("cell-cont: %s\n", targv[i]);
	printf("  type-id: %d\n", args[i-j]);
#endif
	numArgs = i;
      }
      

      if(WriteMessage(x->x_oscbuf, messageName, numArgs, args)) {
	post("sendOSC: usage error, write-msg failed: %s", OSC_errorMessage);
	return;
      }
      
      if(!x->x_bundle) {
/* 	// post("sendOSC: accumulating bundle, not sending things ...");	 */
/*       } else { */
	// post("sendOSC: yeah and OUT!");
	SendBuffer(x->x_htmsocket, x->x_oscbuf);
	OSC_initBuffer(x->x_oscbuf, SC_BUFFER_SIZE, bufferForOSCbuf);
      }
      
      //CommandLineMode(argc, targv, x->x_htmsocket);
      //useTypeTags = 0;
    }
  else {
    post("sendOSC: not connected");
    //    exit(3);
  }
}

void sendOSC_send(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv) 
{
  if(!argc) {
    post("not sending empty message.");
    return;
  }
  if(x->x_typetags) {
    useTypeTags = 1;
    sendOSC_sendtyped(x,s,argc,argv);
    useTypeTags = 0;
  } else {
    sendOSC_sendtyped(x,s,argc,argv);
  }
}

static void sendOSC_free(t_sendOSC *x)
{
    sendOSC_disconnect(x);
}

#ifdef WIN32
	OSC_API void sendOSC_setup(void) { 
#else
	void sendOSC_setup(void) {
#endif
    sendOSC_class = class_new(gensym("sendOSC"), (t_newmethod)sendOSC_new,
			      (t_method)sendOSC_free,
			      sizeof(t_sendOSC), 0, A_DEFFLOAT, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_connect,
		    gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_disconnect,
		    gensym("disconnect"), 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_settypetags,
		    gensym("typetags"),
		    A_FLOAT, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_send,
		    gensym("send"),
		    A_GIMME, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_send,
		    gensym("senduntyped"),
		    A_GIMME, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_send,
		    gensym("sendtyped"),
		    A_GIMME, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_openbundle,
		    gensym("["),
		    0, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_closebundle,
		    gensym("]"),
		    0, 0);
    class_sethelpsymbol(sendOSC_class, gensym("OSC/sendOSC-help.pd"));
}





/* Exit status codes:
    0: successful
    2: Message(s) dropped because of buffer overflow
    3: Socket error
    4: Usage error
    5: Internal error
*/

void CommandLineMode(int argc, char *argv[], void *htmsocket) {
    char *messageName;
    char *token;
    typedArg args[MAX_ARGS];
    int i,j, numArgs;
    OSCbuf buf[1];

  OSC_initBuffer(buf, SC_BUFFER_SIZE, bufferForOSCbuf);

  if (argc > 1) {
    post("argc (%d) > 1", argc);
/* 	if (OSC_openBundle(buf, OSCTT_Immediately())) { */
/* 	    send_complain("Problem opening bundle: %s\n", OSC_errorMessage); */
/* 	    return; */
/* 	} */
    }

  //    ParseInteractiveLine(buf, argv);
  messageName = strtok(argv[0], ",");

    j = 1;
    for (i = j; i < argc; i++) {
      token = strtok(argv[i],",");
      args[i-j] = ParseToken(token);
#ifdef DEBUG
      printf("cell-cont: %s\n", argv[i]);
      printf("  type-id: %d\n", args[i-j]);
#endif
      numArgs = i;
    }

    if(WriteMessage(buf, messageName, numArgs, args)) {
      post("sendOSC: usage error. write-msg failed: %s", OSC_errorMessage);
      return;
    }

/*     for (i = 0; i < argc; i++) { */
/*         messageName = strtok(argv[i], ","); */
/* 	//send_complain ("commandlinemode: count: %d %s\n",i, messageName); */
/*         if (messageName == NULL) { */
/*             break; */
/*         } */

/*         j = 0; */
/*         while ((token = strtok(NULL, ",")) != NULL) { */
/*             args[j] = ParseToken(token); */
/*             j++; */
/* 	    if (j >= MAX_ARGS) { */
/* 		send_complain("Sorry; your message has more than MAX_ARGS (%d) arguments; ignoring the rest.\n", */
/* 			 MAX_ARGS); */
/* 		break; */
/* 	    } */
/*         } */
/*         numArgs = j; */

/*         WriteMessage(buf, messageName, numArgs, args); */

/*     } */

/*     if (argc > 1) { */
/* 	if (OSC_closeBundle(buf)) { */
/* 	    send_complain("Problem closing bundle: %s\n", OSC_errorMessage); */
/* 	    return; */
/* 	} */
/*     } */

    SendBuffer(htmsocket, buf);
}

#define MAXMESG 2048

void InteractiveMode(void *htmsocket) {
    char mesg[MAXMESG];
    OSCbuf buf[1];
    int bundleDepth = 0;    /* At first, we haven't seen "[". */

    OSC_initBuffer(buf, SC_BUFFER_SIZE, bufferForOSCbuf);

    while (fgets(mesg, MAXMESG, stdin) != NULL) {
        if (mesg[0] == '\n') {
	  if (bundleDepth > 0) {
	    /* Ignore blank lines inside a group. */
	  } else {
            /* blank line => repeat previous send */
            SendBuffer(htmsocket, buf);
	  }
	  continue;
        }

	if (bundleDepth == 0) {
	    OSC_resetBuffer(buf);
	}

	if (mesg[0] == '[') {
	    OSCTimeTag tt = ParseTimeTag(mesg+1);
	    if (OSC_openBundle(buf, tt)) {
		send_complain("Problem opening bundle: %s\n", OSC_errorMessage);
		OSC_resetBuffer(buf);
		bundleDepth = 0;
		continue;
	    }
	    bundleDepth++;
        } else if (mesg[0] == ']' && mesg[1] == '\n' && mesg[2] == '\0') {
            if (bundleDepth == 0) {
                send_complain("Unexpected ']': not currently in a bundle.\n");
            } else {
		if (OSC_closeBundle(buf)) {
		    send_complain("Problem closing bundle: %s\n", OSC_errorMessage);
		    OSC_resetBuffer(buf);
		    bundleDepth = 0;
		    continue;
		}

		bundleDepth--;
		if (bundleDepth == 0) {
		    SendBuffer(htmsocket, buf);
		}
            }
        } else {
            ParseInteractiveLine(buf, mesg);
            if (bundleDepth != 0) {
                /* Don't send anything until we close all bundles */
            } else {
                SendBuffer(htmsocket, buf);
            }
        }
    }
}

OSCTimeTag ParseTimeTag(char *s) {
    char *p, *newline;
    typedArg arg;

    p = s;
    while (isspace(*p)) p++;
    if (*p == '\0') return OSCTT_Immediately();

    if (*p == '+') {
	/* Time tag is for some time in the future.  It should be a
           number of seconds as an int or float */

	newline = strchr(s, '\n');
	if (newline != NULL) *newline = '\0';

	p++; /* Skip '+' */
	while (isspace(*p)) p++;

	arg = ParseToken(p);
	if (arg.type == STRING_osc) {
	    send_complain("warning: inscrutable time tag request: %s\n", s);
	    return OSCTT_Immediately();
	} else if (arg.type == INT_osc) {
	    return OSCTT_PlusSeconds(OSCTT_CurrentTime(),
				     (float) arg.datum.i);
	} else if (arg.type == FLOAT_osc) {
	    return OSCTT_PlusSeconds(OSCTT_CurrentTime(), arg.datum.f);
	} else {
	    fatal_error("This can't happen!");
	}
    }

    if (isdigit(*p) || (*p >= 'a' && *p <='f') || (*p >= 'A' && *p <='F')) {
	/* They specified the 8-byte tag in hex */
	OSCTimeTag tt;
	if (sscanf(p, "%llx", &tt) != 1) {
	    send_complain("warning: couldn't parse time tag %s\n", s);
	    return OSCTT_Immediately();
	}
#ifndef	HAS8BYTEINT
	if (ntohl(1) != 1) {
	    /* tt is a struct of seconds and fractional part,
	       and this machine is little-endian, so sscanf
	       wrote each half of the time tag in the wrong half
	       of the struct. */
	    uint32 temp;
	    temp = tt.seconds;
	    tt.seconds = tt.fraction ;
	    tt.fraction = temp;
	}
#endif
	return tt;
    }

    send_complain("warning: invalid time tag: %s\n", s);
    return OSCTT_Immediately();
}
	    

void ParseInteractiveLine(OSCbuf *buf, char *mesg) {
    char *messageName, *token, *p;
    typedArg args[MAX_ARGS];
    int thisArg;

    p = mesg;
    while (isspace(*p)) p++;
    if (*p == '\0') return;

    messageName = p;

    if (strcmp(messageName, "play\n") == 0) {
	/* Special kludge feature to save typing */
	typedArg arg;

	if (OSC_openBundle(buf, OSCTT_Immediately())) {
	    send_complain("Problem opening bundle: %s\n", OSC_errorMessage);
	    return;
	}

	arg.type = INT_osc;
	arg.datum.i = 0;
	WriteMessage(buf, "/voices/0/tp/timbre_index", 1, &arg);

	arg.type = FLOAT_osc;
	arg.datum.i = 0.0f;
	WriteMessage(buf, "/voices/0/tm/goto", 1, &arg);

	if (OSC_closeBundle(buf)) {
	    send_complain("Problem closing bundle: %s\n", OSC_errorMessage);
	}

	return;
    }

    while (!isspace(*p) && *p != '\0') p++;
    if (isspace(*p)) {
        *p = '\0';
        p++;
    }

    thisArg = 0;
    while (*p != '\0') {
        /* flush leading whitespace */
        while (isspace(*p)) p++;
        if (*p == '\0') break;

        if (*p == '"') {
            /* A string argument: scan for close quotes */
            p++;
            args[thisArg].type = STRING_osc;
            args[thisArg].datum.s = p;

            while (*p != '"') {
                if (*p == '\0') {
                    send_complain("Unterminated quote mark: ignoring line\n");
                    return;
                }
                p++;
            }
            *p = '\0';
            p++;
        } else {
            token = p;
            while (!isspace(*p) && (*p != '\0')) p++;
            if (isspace(*p)) {
                *p = '\0';
                p++;
            }
            args[thisArg] = ParseToken(token);
        }
        thisArg++;
	if (thisArg >= MAX_ARGS) {
	  send_complain("Sorry, your message has more than MAX_ARGS (%d) arguments; ignoring the rest.\n",
		   MAX_ARGS);
	  break;
	}
    }

    if (WriteMessage(buf, messageName, thisArg, args) != 0)  {
	send_complain("Problem sending message: %s\n", OSC_errorMessage);
    }
}

typedArg ParseToken(char *token) {
    char *p = token;
    typedArg returnVal;

    /* It might be an int, a float, or a string */

    if (*p == '-') p++;

    if (isdigit(*p) || *p == '.') {
        while (isdigit(*p)) p++;
        if (*p == '\0') {
            returnVal.type = INT_osc;
            returnVal.datum.i = atoi(token);
            return returnVal;
        }
        if (*p == '.') {
            p++;
            while (isdigit(*p)) p++;
            if (*p == '\0') {
                returnVal.type = FLOAT_osc;
                returnVal.datum.f = atof(token);
                return returnVal;
            }
        }
    }

    returnVal.type = STRING_osc;
    returnVal.datum.s = token;
    return returnVal;
}

int WriteMessage(OSCbuf *buf, char *messageName, int numArgs, typedArg *args) {
  int j, returnVal;
  const wmERROR = -1;

  returnVal = 0;

#ifdef DEBUG
  printf("WriteMessage: %s ", messageName);

  for (j = 0; j < numArgs; j++) {
    switch (args[j].type) {
    case INT_osc:
      printf("%d ", args[j].datum.i);
      break;
      
    case FLOAT_osc:
      printf("%f ", args[j].datum.f);
      break;
      
    case STRING_osc:
      printf("%s ", args[j].datum.s);
      break;
      
    default:
      fatal_error("Unrecognized arg type, (not exiting)");
      return(wmERROR);
      // exit(5);
    }
  }
  printf("\n");
#endif
  
  if (!useTypeTags) {
    returnVal = OSC_writeAddress(buf, messageName);
    if (returnVal) {
      send_complain("Problem writing address: %s\n", OSC_errorMessage);
    }
  } else {
    /* First figure out the type tags */
    char typeTags[MAX_ARGS+2];
    int i;
    
    typeTags[0] = ',';
    
    for (i = 0; i < numArgs; ++i) {
      switch (args[i].type) {
      case INT_osc:
	typeTags[i+1] = 'i';
	break;
	
      case FLOAT_osc:
	typeTags[i+1] = 'f';
	break;
	
      case STRING_osc:
	typeTags[i+1] = 's';
	break;
	
      default:
	fatal_error("Unrecognized arg type (not exiting)");
	return(wmERROR);
	// exit(5);
      }
    }
    typeTags[i+1] = '\0';
    
    returnVal = OSC_writeAddressAndTypes(buf, messageName, typeTags);
    if (returnVal) {
      send_complain("Problem writing address: %s\n", OSC_errorMessage);
    }
  }

  for (j = 0; j < numArgs; j++) {
    switch (args[j].type) {
    case INT_osc:
      if ((returnVal = OSC_writeIntArg(buf, args[j].datum.i)) != 0) {
	return returnVal;
      }
      break;
      
    case FLOAT_osc:
      if ((returnVal = OSC_writeFloatArg(buf, args[j].datum.f)) != 0) {
	return returnVal;
      }
      break;
      
    case STRING_osc:
      if ((returnVal = OSC_writeStringArg(buf, args[j].datum.s)) != 0) {
	return returnVal;
      }
      break;
      
    default:
      fatal_error("Unrecognized arg type (not exiting)");
      returnVal = wmERROR;
      // exit(5);
    }
  }
  return returnVal;
}

void SendBuffer(void *htmsocket, OSCbuf *buf) {
#ifdef DEBUG
  printf("Sending buffer...\n");
#endif
  if (OSC_isBufferEmpty(buf)) {
		post("SendBuffer() called but buffer empty");
		return;
  }
  if (!OSC_isBufferDone(buf)) {
		fatal_error("SendBuffer() called but buffer not ready!, not exiting");
		// exit(5); 
		return;	//{{raf}}
  }
  SendData(htmsocket, OSC_packetSize(buf), OSC_getPacket(buf));
}

void SendData(void *htmsocket, int size, char *data) {
  if (!SendHTMSocket(htmsocket, size, data)) {
    post("SendData::SendHTMSocket()failure -- not connected");
    CloseHTMSocket(htmsocket);
    //    sendOSC_disconnect();
    //exit(3);
    //    return;
  }
}

void fatal_error(char *s) {
    fprintf(stderr, "FATAL ERROR: %s\n", s);
    post("fatal error, not exiting ...");
    //exit(4);
}

#include <stdarg.h>
void send_complain(char *s, ...) {
    va_list ap;
    va_start(ap, s);
    vfprintf(stderr, s, ap);
    va_end(ap);
}


#ifdef COMPUTE_MESSAGE_SIZE
    /* Unused code to find the size of a message */

    /* Compute size */
    size = SynthControl_effectiveStringLength(messageName);

    for (j = 0; j < numArgs; j++) {
        switch (args[j].type) {
            case INT_osc: case FLOAT_osc:
            size += 4;
            break;

            case STRING_osc:
            size += SynthControl_effectiveStringLength(args[j].datum.s);
            break;

            default:
            fatal_error("Unrecognized token type ( not exiting)");
            // exit(4);
        }
    }

    if (!SynthControl_willMessageFit(buf, size)) {
        send_complain("Message \"%s\" won't fit in buffer: dropping.", messageName);
        return;
    }
#endif
