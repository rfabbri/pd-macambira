/* comport - PD external for unix/windows

 (c) 1998-2005  Winfried Ritsch (see LICENCE.txt)
 Institute for Electronic Music - Graz

 V 1.0 RC2

*/

#include "m_pd.h"

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#include <windows.h>
#include <commctrl.h>
#else
#include <sys/time.h>
#include <fcntl.h> 
#include <termios.h>               /* for TERMIO ioctl calls */
#include <unistd.h>
#define HANDLE int
#define INVALID_HANDLE_VALUE -1
#endif

#include <string.h>
#include <errno.h>


typedef struct comport
{
  t_object x_obj;

  long n;           /* the state of a last input */

  HANDLE comhandle;              /* holds the comport handle */

#ifdef NT
  DCB dcb;                      /* holds the comm pars */
  DCB dcb_old;                  /* holds the comm pars */
  COMMTIMEOUTS old_timeouts;
#else
  struct termios oldcom_termio;    /* save the old com config */
  struct termios com_termio;       /* for the new com config */
#endif

  short comport;            /* holds the comport # */
  float baud;                /* holds the current baud rate */

  short rxerrors;             /* holds the rx line errors */

  t_clock *x_clock;
  int x_hit;
  double x_deltime;

} t_comport;

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#ifndef ON
#define ON      1
#define OFF     0
#endif

/*
    Serial Port Return Values
*/
#define NODATAAVAIL     -1
#define RXERRORS        -2
#define RXBUFOVERRUN    -4
#define TXBUFOVERRUN    -5

#ifdef NT

#define COMPORT_MAX 8
static char *sys_com_port[COMPORT_MAX] = 
{
  "COM1", "COM2", "COM3", "COM4",
  "COM5", "COM6", "COM7", "COM8"
};

static
long baudspeedbittable[] = 
{ 
  CBR_256000,
  CBR_128000,
  CBR_115200,
  CBR_57600,
  CBR_56000,
  CBR_38400,
  CBR_19200,
  CBR_14400,
  CBR_9600,
  CBR_4800,
  CBR_2400,
  CBR_1200,
  CBR_600,
  CBR_300,
  CBR_110
}; 

#else /* NT */

#ifdef  IRIX  
#define COMPORT_MAX 2
static char *sys_com_port[COMPORT_MAX] = 
{
  "/dev/ttyd1", "/dev/ttyd2"
};
#define OPENPARAMS (O_RDWR|O_NDELAY|O_NOCTTY)
#define TIONREAD FIONREAD         /* re map the IOCTL function */
#define BAUDRATE_256000 -1
#define BAUDRATE_128000 -1
#define BAUDRATE_115200 -1
#define BAUDRATE_57600  -1
#define BAUDRATE_56000  -1
#define BAUDRATE_38400  B38400
#define BAUDRATE_14400  B19200 /* 14400 gibts nicht */
#else /* IRIX */
#define COMPORT_MAX 16
static char *sys_com_port[COMPORT_MAX] = 
{
  "/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyS3",
  "/dev/ttyS4", "/dev/ttyS5", "/dev/ttyS6", "/dev/ttyS7",
  "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyUSB3",
  "/dev/ttyUSB4", "/dev/ttyUSB5", "/dev/ttyUSB6", "/dev/ttyUSB7"
};
#define OPENPARAMS (O_RDWR|O_NDELAY|O_NOCTTY)
#define BAUDRATE_256000 -1
#define BAUDRATE_128000 -1
#define BAUDRATE_115200 B115200
#define BAUDRATE_57600  B57600
#define BAUDRATE_56000  B57600 /* 56000 gibts nicht */
#define BAUDRATE_38400  B38400
#define BAUDRATE_14400  B19200 /* 14400 gibts nicht */

#endif /* else IRIX */

static
short baudspeedbittable[] = 
{ 
  BAUDRATE_256000,  /* CPU SPECIFIC */
  BAUDRATE_128000,  /* CPU SPECIFIC */
  BAUDRATE_115200,  /* CPU SPECIFIC */
  BAUDRATE_57600,   /* CPU SPECIFIC */
  BAUDRATE_56000,
  BAUDRATE_38400,   /* CPU SPECIFIC */
  B19200,
  BAUDRATE_14400,
  B9600,
  B4800,
  B2400,
  B1200,
  B600,
  B300,
  B110
}; 

struct timeval null_tv;

#endif /* else NT */


#define BAUDRATETABLE_LEN 15

static
long baudratetable[] = 
{ 
  256000L,
  128000L,
  115200L,
  57600L,
  56000L,
  38400L,
  19200L,
  14400L,
  9600L,
  4800L,
  2400L,
  1200L,
  600L,
  300L,
  110L
};    /* holds the baud rate selections */

t_class *comport_class;

/* --------- sys independend serial setup helpers ---------------- */

static long get_baud_ratebits(t_float *baud)
{
 int i = 0;

  while(i < BAUDRATETABLE_LEN && baudratetable[i] > *baud)
    i++;
  
  /* nearest Baudrate finding */
  if(i==BAUDRATETABLE_LEN ||  baudspeedbittable[i] < 0){
    post("*Warning* The baud rate %d is not suported or out of range, using 9600\n",*baud);
    i = 7;
  }
  *baud =  baudratetable[i];

  return baudspeedbittable[i];
}


/* ------------ sys dependend serial setup helpers ---------------- */


/* --------------------- NT ------------------------------------ */

#ifdef NT


static float set_baudrate(t_comport *x,t_float baud)
{
  x->dcb.BaudRate = get_baud_ratebits(&baud);

  return baud;
}

/* bits are 5,6,7,8(default) */

static float set_bits(t_comport *x, int nr)
{

  if(nr < 4 && nr > 8)
    nr = 8;

 /*  number of bits/byte, 4-8  */
  return x->dcb.ByteSize = nr;
}


/* 1 ... Parity even, -1 parity odd , 0 (default) no parity */
static float set_parity(t_comport *x,int n)
{
  switch(n){
  case 1:
    x->dcb.fParity = TRUE; /*  enable parity checking */
    x->dcb.Parity = 2;     /*  0-4=no,odd,even,mark,space  */
    return 1;

  case -1:
    x->dcb.fParity = TRUE; /*  enable parity checking */
    x->dcb.Parity = 1;     /*  0-4=no,odd,even,mark,space  */
    return -1;

  default:
    x->dcb.fParity = FALSE; /*  enable parity checking */
    x->dcb.Parity = 0;     /*  0-4=no,odd,even,mark,space  */
  }
  return 0;
}


/* aktivate second stop bit with 1, 0(default)*/
static float set_stopflag(t_comport *x, int nr)
{
  if(nr == 1){
    x->dcb.StopBits = 1;             /*  0,1,2 = 1, 1.5, 2  */
    return 1;
  }
  else
    x->dcb.StopBits = 0;             /*  0,1,2 = 1, 1.5, 2  */

  return 0;
}

/* never testet */
static int set_ctsrts(t_comport *x, int nr)
{
  if(nr == 1){
  x->dcb.fOutxCtsFlow = TRUE;      /*  CTS output flow control  */
  x->dcb.fRtsControl = RTS_CONTROL_ENABLE;       /*  RTS flow control  */
  return 1;
  }
  x->dcb.fOutxCtsFlow = FALSE;      /*  CTS output flow control  */
  x->dcb.fRtsControl = RTS_CONTROL_DISABLE;       /*  RTS flow control  */
  return 0;
}

static int set_xonxoff(t_comport *x, int nr)
{
  /*   x->dcb.fTXContinueOnXoff = FALSE;  XOFF continues Tx  */

  if(nr == 1){ 
    x->dcb.fOutX  = TRUE;           /*  XON/XOFF out flow control  */
    x->dcb.fInX  = TRUE;             /*  XON/XOFF in flow control */
    return 1;
  }

  x->dcb.fOutX  = FALSE;           /*  XON/XOFF out flow control  */
  x->dcb.fInX  = FALSE;             /*  XON/XOFF in flow control */
  return 0;
}


static int set_serial(t_comport *x)
{

 if (SetCommState(x->comhandle, &(x->dcb)))
   return 1;

 return 0;
}

static HANDLE open_serial(int com_nr, t_comport *x)
{
  HANDLE fd;

  COMMTIMEOUTS timeouts;

  float *baud = &(x->baud);

  if(com_nr < 0 || com_nr >= COMPORT_MAX) {
	 post("comport open %d, baud %d not valid (args: [portnum] [baud])",com_nr,*baud);
	 return INVALID_HANDLE_VALUE;
  }

  fd = CreateFile( sys_com_port[com_nr],  
		   GENERIC_READ | GENERIC_WRITE, 
		   0, 
		   0, 
		   OPEN_EXISTING,
#ifdef WIN2000
		   0,
#elif
		   FILE_FLAG_OVERLAPPED,
#endif
		   0);

  if(fd == INVALID_HANDLE_VALUE)
    {
      post("** ERROR ** could not open device %s:\n failure(%d): %s\n",
	   sys_com_port[com_nr],errno,strerror(errno));
      return INVALID_HANDLE_VALUE;
    }

  /*   Save the Current Port Configuration  */

  if (!GetCommState(fd, &(x->dcb_old))){
    post("** ERROR ** could not get old dcb of device %s\n",
	 sys_com_port[com_nr]);

    CloseHandle(fd); 
    return INVALID_HANDLE_VALUE;
  }

  memset(&(x->dcb), sizeof(DCB), 0);

  if (!GetCommState(fd, &(x->dcb))){
    post("** ERROR ** could not get new dcb of device %s\n",
	 sys_com_port[com_nr]);

    CloseHandle(fd); 
    return INVALID_HANDLE_VALUE;
  }


  x->dcb.fBinary = TRUE;          /*  binary mode, no EOF check  */

  /*   x->dcb.fOutxDsrFlow = FALSE;       DSR output flow control  */
  /*   x->dcb.fDtrControl = DTR_CONTROL_DISABLE;        DTR flow control type  */

  /*   x->dcb.fDsrSensitivity = FALSE;    DSR sensitivity  */

  x->dcb.fErrorChar = FALSE;       /*  enable error replacement  */
  /*    x->dcb.fNull = FALSE;             enable null stripping  */

  /*     DWORD x->dcb.fAbortOnError:1;      abort reads/writes on error  */

  /*    char x->dcb.XonChar;               Tx and Rx XON character  */
  /*    char x->dcb.XoffChar;              Tx and Rx XOFF character  */
  /*    char x->dcb.ErrorChar;             error replacement character  */
  /*    char x->dcb.EofChar;               end of input character  */
  /*    char x->dcb.EvtChar;               received event character  */

  set_bits(x,8);      /* CS8 */
  set_stopflag(x,0);  /* ~CSTOPB */
  set_ctsrts(x,0);  /* ~CRTSCTS;*/
  set_xonxoff(x,1); /* (IXON | IXOFF | IXANY) */
  set_baudrate(x,*baud);

  x->comhandle = fd;

  if(set_serial(x))
    {
      post("Opened serial line device  %s\n",sys_com_port[com_nr]);
    }
  else 
    {
      post("** ERROR ** could not set params to control dcb of device %s\n",
	   sys_com_port[com_nr]);
      CloseHandle(fd); 
      return INVALID_HANDLE_VALUE;
    }
  

  
  if (!GetCommTimeouts(fd, &(x->old_timeouts))){
	  post("Couldnt get old timeouts for serial device");
  };

  /* setting new timeouts for read to immidiatly return */
  timeouts.ReadIntervalTimeout = MAXDWORD; 
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = 0;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 0;

  if (!SetCommTimeouts(fd, &timeouts)){
  	  post("Couldnt set timeouts for serial device");
	  return INVALID_HANDLE_VALUE;	
  };

  
  return fd;
}

static HANDLE close_serial(t_comport *x)
{
  if(x->comhandle != INVALID_HANDLE_VALUE){

    if (!SetCommState(x->comhandle, &(x->dcb_old)) )
      {
	post("** ERROR ** could not reset params to DCB of device %s\n",
	     sys_com_port[x->comport]);
      }

    if (!SetCommTimeouts(x->comhandle, &(x->old_timeouts))){
      post("Couldnt reset old_timeouts for serial device");
    };
    CloseHandle(x->comhandle); 
  }

  return INVALID_HANDLE_VALUE;
}


static int write_serial(t_comport *x, unsigned char  chr)
{
  OVERLAPPED osWrite = {0};
  DWORD dwWritten;
  DWORD dwRes;

  /*  post("open send %d",chr); */

  if (!WriteFile(x->comhandle, &chr, 1, &dwWritten, &osWrite)) {
    if (GetLastError() != ERROR_IO_PENDING) 
      post("WriteFile failed, but isn't delayed on serialdevice");
    return 0; 
  }  
  return 1;
}

#else /* NT */
/* ----------------- POSIX - UNIX ------------------------------ */


static float set_baudrate(t_comport *x,t_float baud)
{
  struct termios *tio = &(x->com_termio);

  long baudbits = get_baud_ratebits(&baud);

  cfsetispeed(tio, baudbits);
  cfsetospeed(tio, baudbits);

  return baud;
}

/* bits are 5,6,7,8(default) */

static float set_bits(t_comport *x, int nr)
{
  struct termios *tio = &(x->com_termio);
  tio->c_cflag &= ~CSIZE;
  switch(nr){
  case 5:     tio->c_cflag |= CS5; return 5;
  case 6:     tio->c_cflag |= CS6; return 6;
  case 7:     tio->c_cflag |= CS7; return 7;
  default:    tio->c_cflag |= CS8; 
  }
  return 8;
}


/* 1 ... Parity even, -1 parity odd , 0 (default) no parity */
static float set_parity(t_comport *x,int n)
{
  struct termios *tio = &(x->com_termio);

  switch(n){
  case 1:
    tio->c_cflag |= PARENB;  tio->c_cflag &= ~PARODD; return 1;
  case -1:
    tio->c_cflag |= PARENB | PARODD; return -1;
  default:
    tio->c_cflag &= ~PARENB;
  }
  return 0;
}


/* aktivate second stop bit with 1, 0(default)*/
static float set_stopflag(t_comport *x, int nr)
{
  struct termios *tio = &(x->com_termio);

  if(nr == 1){
    tio->c_cflag |= CSTOPB;
    return 1;
  }
  else
    tio->c_cflag &= ~CSTOPB;
  return 0;
}

/* never testet */
static int set_ctsrts(t_comport *x, int nr)
{
  struct termios *tio = &(x->com_termio);

  if(nr == 1){
    tio->c_cflag |= CRTSCTS;
    return 1;
  }
  tio->c_cflag &= ~CRTSCTS;
  return 0;
}

static int set_xonxoff(t_comport *x, int nr)
{
  struct termios *tio = &(x->com_termio);

  if(nr == 1){ 
    tio->c_iflag |= (IXON | IXOFF | IXANY); 
    return 1;
  }

  tio->c_iflag &= ~IXON & ~IXOFF &  ~IXANY;
  return 0;
}

static int open_serial(int com_nr, t_comport *x)
{
  HANDLE fd;
  struct termios *old = &(x->oldcom_termio);
  struct termios *new = &(x->com_termio);
  float *baud = &(x->baud);

  if(com_nr < 0 || com_nr >= COMPORT_MAX) {
	 post("comport open %d, baud %d not valid (args: [portnum] [baud])");
	 return INVALID_HANDLE_VALUE;
  }

  if((fd = open(sys_com_port[com_nr], OPENPARAMS)) == INVALID_HANDLE_VALUE)
    {
		post("** ERROR ** could not open device %s:\n failure(%d): %s\n",
			  sys_com_port[com_nr],errno,strerror(errno));
		return INVALID_HANDLE_VALUE;
    }

  /* set no wait on any operation */
  fcntl(fd, F_SETFL, FNDELAY);
  
  /*   Save the Current Port Configuration  */
  if(tcgetattr(fd, old) == -1 || tcgetattr(fd, new) == -1){
    post("** ERROR ** could not get termios-structure of device %s\n",
	 sys_com_port[com_nr]);
    close(fd);
    return INVALID_HANDLE_VALUE;
  }
  
  
  /* Setupt the new port configuration...NON-CANONICAL INPUT MODE
	 .. as defined in termios.h
  */

  /* enable input and ignore modem controls */
  new->c_cflag |= (CREAD | CLOCAL);

  /* always nocanonical, this means raw i/o no terminal */
  new->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  /* no post processing */
  new->c_oflag &= ~OPOST;

  /* setup to return after 0 seconds 
    ..if no characters are received 
    TIME units are assumed to be 0.1 secs */
  /* not needed anymore ??? in new termios in linux
  new->c_cc[VMIN] = 0;     
  new->c_cc[VTIME] = 0;   
  */
  
  /* defaults, see input */

  set_bits(x,8);      /* CS8 */
  set_stopflag(x,0);  /* ~CSTOPB */
  set_ctsrts(x,0);  /* ~CRTSCTS;*/
  set_xonxoff(x,1); /* (IXON | IXOFF | IXANY) */
  set_baudrate(x,*baud);
  
  if(tcsetattr(fd, TCSAFLUSH, new) != -1)
	 {
		post("Opened serial line device  %s\n",sys_com_port[com_nr]);
	 }
  else 
	 {
		post("** ERROR ** could not set params to ioctl of device %s\n",
			  sys_com_port[com_nr]);
		close(fd);
		return INVALID_HANDLE_VALUE;
	 }

  return fd;
}


static int close_serial(t_comport *x)
{
  struct termios *tios = &(x->com_termio);
  HANDLE fd = x->comhandle;

   if(fd != INVALID_HANDLE_VALUE){
    tcsetattr(fd, TCSANOW, tios);
    close(fd);
   }
   return INVALID_HANDLE_VALUE;
}


static int set_serial(t_comport *x)
{
  if(tcsetattr(x->comhandle, TCSAFLUSH, &(x->com_termio)) == -1)
    return 0;
  return 1;
}

static int write_serial(t_comport *x, unsigned char  chr)
{

  return write(x->comhandle,(char *) &chr,1);

  /* flush pending I/O chars */
  /* but nowadays discards them ;-(
  else{
      ioctl(x->comhandle,TCFLSH,TCOFLUSH);
  }
  */  
}


#endif /* else NT */


/* ------------------- serial pd methods --------------------------- */
static void comport_pollintervall(t_comport *x, t_floatarg g)
{
    if (g < 1) g = 1;
    x->x_deltime = g;
}

static void comport_tick(t_comport *x)
{
  unsigned char chr;
  int err;
  HANDLE fd = x->comhandle;

  x->x_hit = 0;


  if(fd == INVALID_HANDLE_VALUE) return;
  
  /* while there are bytes, read them and send them out, ignore errors */
#ifdef NT
  {
    DWORD dwCommEvent;
    DWORD dwRead;

    err = 0;

/*
    if (!SetCommMask(x->comhandle, EV_RXCHAR))
      post(" Error setting communications event mask for serial device");

    for ( ; ; ) {
      if (WaitCommEvent(x->comhandle, &dwCommEvent,NULL)) {
*/
		do {

		  if(ReadFile(x->comhandle, &chr, 1, &dwRead, NULL))
		  	 if(dwRead > 0)
				 outlet_float(x->x_obj.ob_outlet, (t_float) chr);
	      else{	
	         err = -1;
	         break;
		  }
		} while (dwRead);
/*
      }
      else{
	     post("serial dev: Error in WaitCommEvent");
	     break;
      }
    }
*/
  }
#else
  {
    fd_set com_rfds;  	 
    FD_ZERO(&com_rfds);
    FD_SET(fd,&com_rfds);

    while((err=select(fd+1,&com_rfds,NULL,NULL,&null_tv)) > 0) {

      err = read(fd,(char *) &chr,1); 

      /*  while(    (err = read(fd,(char *) &chr,1)) > 0){ */
      outlet_float(x->x_obj.ob_outlet, (t_float) chr);

    };
  }
#endif 

  if(err < 0){                  /* if an readerror detected */
    if(x->rxerrors == 0)            /* post it once */
      post("RXERRORS on serial line\n");
    x->rxerrors = 1;                /* remember */
  }

  if (!x->x_hit) clock_delay(x->x_clock, 1);
}

static void comport_float(t_comport *x, t_float f)
{
  unsigned char chr = ((int) f) & 0xFF; /* brutal conv */

  if (write_serial(x,chr) != 1)
    {
	 post("Write error, maybe TX-OVERRUNS on serial line");
    }
}

static void *comport_new(t_floatarg comnr, t_floatarg fbaud) {

  t_comport test;
  t_comport *x;
  int com_nr = comnr;
  HANDLE fd;

/*	 Open the Comport for RD and WR and get a handle */
  test.baud = fbaud;
  fd = open_serial(com_nr,&test);


  /* Now  nothing really bad could happen so we create the class */

  x = (t_comport *)pd_new(comport_class);

  if(fd == INVALID_HANDLE_VALUE ){
    /* postings in open routine */
    x->comport = com_nr;
    x->baud = test.baud;
    x->comhandle = fd;           /* holds the comport handle */
  }
  else {
    x->comport = com_nr;
    x->baud = test.baud;
    x->comhandle = fd;           /* holds the comport handle */

#ifdef NT
  memcpy(&(test.dcb_old),&(x->dcb_old),sizeof(DCB));    /*  save the old com config  */
  memcpy(&(test.dcb),&(x->dcb),sizeof(DCB));       /*  for the new com config  */
#else  
  /* save the old com and new com config */
  bcopy(&(test.oldcom_termio),&(x->oldcom_termio),sizeof(struct termios));    
  bcopy(&(test.com_termio),&(x->com_termio),sizeof(struct termios));       
#endif
  }

  x->rxerrors = 0;             /* holds the rx line errors */

  outlet_new(&x->x_obj, &s_float);

  x->x_hit = 0;
  x->x_deltime = 1;
  x->x_clock = clock_new(x, (t_method)comport_tick);

  clock_delay(x->x_clock, x->x_deltime);

  return x;
}


static void
comport_free(t_comport *x)
{
  post("close serial...");

  clock_unset(x->x_clock);
  clock_free(x->x_clock);
  x->comhandle = close_serial(x);
}

/* ---------------- use serial settings ------------- */

static void comport_baud(t_comport *x,t_floatarg f)
{
  if(f == x->baud){
    post("baudrate already %f\n",x->baud);
    return;
  }

  x->baud = set_baudrate(x,f);

  if(x->comhandle == INVALID_HANDLE_VALUE)return;

  if(set_serial(x) == 0){
    post("** ERROR ** could not set baudrate of device %s\n",
	 sys_com_port[x->comport]);
  }
  else post("set baudrate of %s to %f\n",sys_com_port[x->comport],x->baud);
}

static void comport_bits(t_comport *x,t_floatarg f)
{
  f = set_bits(x,f);

  if(x->comhandle == INVALID_HANDLE_VALUE)return;

  if(set_serial(x) == 0){
    post("** ERROR ** could not set bits of device %s\n",
	 sys_com_port[x->comport]);
  }
  else post("set bits of %s to %f\n",sys_com_port[x->comport],f);
}


static void comport_parity(t_comport *x,t_floatarg f)
{
  f = set_parity(x,f);

  if(x->comhandle == INVALID_HANDLE_VALUE)return;

  if(set_serial(x) == 0){
    post("** ERROR ** could not set extra paritybit of device %s\n",
	 sys_com_port[x->comport]);
  }
  else post("set extra paritybit of %s to %f\n",sys_com_port[x->comport],f);
}

static void comport_stopbit(t_comport *x,t_floatarg f)
{
  f = set_stopflag(x,f);

  if(x->comhandle == INVALID_HANDLE_VALUE)return;

  if(set_serial(x) == 0){
    post("** ERROR ** could not set extra stopbit of device %s\n",
	 sys_com_port[x->comport]);
  }
  else post("set extra stopbit of %s to %f\n",sys_com_port[x->comport],f);
}

static void comport_rtscts(t_comport *x,t_floatarg f)
{
  f = set_ctsrts(x,f);

  if(x->comhandle == INVALID_HANDLE_VALUE)return;

  if(set_serial(x) == 0){
    post("** ERROR ** could not set rts_cts of device %s\n",
	 sys_com_port[x->comport]);
  }
  else post("set rts-cts of %s to %f\n",sys_com_port[x->comport],f);
}

static void comport_xonxoff(t_comport *x,t_floatarg f)
{
  f = set_xonxoff(x,f);

  if(x->comhandle == INVALID_HANDLE_VALUE)return;

  if(set_serial(x) == 0){
    post("** ERROR ** could not set xonxoff of device %s\n",
	 sys_com_port[x->comport]);
  }
  else post("set xonxoff of %s to %f\n",sys_com_port[x->comport],f);
}

static void comport_close(t_comport *x)
{
  clock_unset(x->x_clock);
  x->x_hit = 1;
  x->comhandle = close_serial(x);
}

static void comport_open(t_comport *x, t_floatarg f)
{
  if(x->comhandle != INVALID_HANDLE_VALUE)
    comport_close(x);


  x->comhandle = open_serial(f,x);

  clock_delay(x->x_clock, x->x_deltime);
}

/* 
   dangerous but if you really have some stupid devicename ... 
   you can open any file on systems if suid is set on pd be careful
*/

static void comport_devicename(t_comport *x, t_symbol *s)
{
  if(x->comport >= 0 && x->comport < COMPORT_MAX){
    sys_com_port[x->comport] = s->s_name;   
  }
}

static void comport_print(t_comport *x, t_symbol *s, int argc, t_atom *argv)
{
  static char buf[256];
  char *pch = buf;

  while(argc--) {
    atom_string(argv++, buf, 255);
    while(*pch != 0) {
      write_serial(x, *pch++);
    }
    if(argc > 0) {
      write_serial(x, ' ');
    }
  }
}
/* ---------------- SETUP OBJECTS ------------------ */

void comport_setup(void)
{
  comport_class 
	 = class_new(gensym("comport"), (t_newmethod)comport_new,
					 (t_method)comport_free, sizeof(t_comport), 
					 0, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addfloat(comport_class, (t_method)comport_float);

  /*
    class_addbang(comport_class, comport_bang
  */  
  class_addmethod(comport_class, (t_method)comport_baud, gensym("baud"), 
		  A_FLOAT, 0); 
  
  class_addmethod(comport_class, (t_method)comport_bits, gensym("bits"), 
		  A_FLOAT, 0);
  class_addmethod(comport_class, (t_method)comport_stopbit, gensym("stopbit"), 
		  A_FLOAT, 0);
  class_addmethod(comport_class, (t_method)comport_rtscts, gensym("rtscts"), 
		  A_FLOAT, 0);
  class_addmethod(comport_class, (t_method)comport_parity, gensym("parity"), 
		  A_FLOAT, 0);
  class_addmethod(comport_class, (t_method)comport_xonxoff, gensym("xonxoff"), 
		  A_FLOAT, 0);
  class_addmethod(comport_class, (t_method)comport_close, gensym("close"), 0);
  class_addmethod(comport_class, (t_method)comport_open, gensym("open"),
		  A_FLOAT, 0);
  class_addmethod(comport_class, (t_method)comport_devicename, gensym("devicename"),
		  A_SYMBOL, 0);
  class_addmethod(comport_class, (t_method)comport_print, gensym("print"),
  		  A_GIMME, 0);

  class_addmethod(comport_class, (t_method)comport_pollintervall, gensym("pollintervall"), 
		  A_FLOAT, 0);
#ifndef NT
  null_tv.tv_sec = 0; /* no wait */
  null_tv.tv_usec = 0;
#endif
  post("comport - PD external for unix/windows\n"
       "LGPL 1998-2005,  Winfried Ritsch and others (see LICENCE.txt)\n"
       "Institute for Electronic Music - Graz");
}
