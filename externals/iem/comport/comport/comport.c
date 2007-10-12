/* comport - PD external for unix/windows

 (c) 1998-2005  Winfried Ritsch (see LICENCE.txt)
 Institute for Electronic Music - Graz

 V 1.0
MP 20060603 memset and memcpy arguments were backwards for Windows version. close_serial doesn't crash now.
MP 20060618 make sure name is set up in comport_new (x->serial_device = test.serial_device) so help message works.
MP 20060619 implemented status outlet
MP 20060620 Add DTR and RTS control, add outputs to reflect CTS and DSR states.
MP 20060621 Do all the above for Windows too.
MP 20060709 All status goes out the status outlet when an info message is received
MP 20060824 added clock_delay call in comport_devicename
MP 20060924 added comport_enum to list available ports in Windows
MP 20060925 add devices message to enumerate actual devices, info just outputs current port state
MP 20061016 write_serial checks for GetOverlappedResult to avoid tx buffer overflow errors
MP 20070719 added "ports" method to output list of available ports on status outlet
MP 20071011 added comport_list and write_serials for list processing based on code by Thomas O Fredericks <tof@danslchamp.org>
*/

#include "m_pd.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#else
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h> /* for ioctl DTR */
#include <termios.h> /* for TERMIO ioctl calls */
#include <unistd.h>
#include <glob.h>
#define HANDLE int
#define INVALID_HANDLE_VALUE -1
#endif /* _WIN32 */

#include <string.h>
#include <errno.h>
#include <stdio.h>


typedef struct comport
{
    t_object       x_obj;
    long           n; /* the state of a last input */
#ifdef _WIN32
    HANDLE         comhandle; /* holds the comport handle */
    DCB            dcb; /* holds the comm pars */
    DCB            dcb_old; /* holds the comm pars */
    COMMTIMEOUTS   old_timeouts;
#else
    int            comhandle; /* holds the comport handle */
    struct termios oldcom_termio; /* save the old com config */
    struct termios com_termio; /* for the new com config */
#endif
    t_symbol       *serial_device;
    char           serial_device_prefix[FILENAME_MAX];/* the device name without the number */
    short          comport; /* holds the comport # */
    t_float        baud; /* holds the current baud rate */
    t_float        data_bits; /* holds the current number of data bits */
    t_float        parity_bit; /* holds the current parity */
    t_float        stop_bits; /* holds the current number of stop bits */
    int            xonxoff; /* nonzero if xonxoff handshaking is on */
    int            ctsrts; /* nonzero if ctsrts handshaking is on */
    int            hupcl; /* nonzero if hang-up on close is on */
    short          rxerrors; /* holds the rx line errors */
    t_clock        *x_clock;
    int            x_hit;
    double         x_deltime;
    int            verbose;
    t_outlet       *x_data_outlet;
    t_outlet       *x_status_outlet;
} t_comport;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef ON
#define ON 1
#define OFF 0
#endif

/* Serial Port Return Values */

#define NODATAAVAIL -1
#define RXERRORS -2
#define RXBUFOVERRUN -4
#define TXBUFOVERRUN -5

#define COMPORT_MAX 99
#define USE_DEVICENAME 9999 /* use the device name instead of the number */
#define MAX_LIST 1000 /* arbitrary maximum list length for comport_list */

#ifdef _WIN32
/* we don't use the  table for windos cos we can set the number directly. */
/* This may result in more possible baud rates than the table contains. */
/*
static long baudspeedbittable[] =
{
    CBR_115200,
    CBR_57600,
    CBR_56000,
    CBR_38400,
    CBR_19200,
    CBR_14400,
    CBR_9600,
    CBR_4800,
    CBR_2400,
    CBR_1800,
    CBR_1200,
    CBR_600,
    CBR_300,
    CBR_110
};
*/
#else /* _WIN32 */

#ifdef  IRIX
#define OPENPARAMS (O_RDWR|O_NDELAY|O_NOCTTY)
#define TIONREAD FIONREAD         /* re map the IOCTL function */
#define BAUDRATE_230400 -1
#define BAUDRATE_115200 -1
#define BAUDRATE_57600  -1
#define BAUDRATE_38400  B38400
#else /* IRIX */
#define OPENPARAMS (O_RDWR|O_NDELAY|O_NOCTTY)
#define BAUDRATE_230400 B230400
#define BAUDRATE_115200 B115200
#define BAUDRATE_57600  B57600
#define BAUDRATE_38400  B38400
#endif /* else IRIX */

static
long baudspeedbittable[] =
{
    BAUDRATE_230400,
    BAUDRATE_115200,  /* CPU SPECIFIC */
    BAUDRATE_57600,   /* CPU SPECIFIC */
    BAUDRATE_38400,   /* CPU SPECIFIC */
    B19200,
    B9600,
    B4800,
    B2400,
    B1800,
    B1200,
    B600,
    B300,
    B200,
    B150,
    B134,
    B110,
    B75,
    B50,
    B0
};

struct timeval null_tv;

#endif /* else _WIN32 */

#define BAUDRATETABLE_LEN 19

static long baudratetable[] =
{
    230400L,
    115200L,
    57600L,
    38400L,
    19200L,
    9600L,
    4800L,
    2400L,
    1800L,
    1200L,
    600L,
    300L,
    200L,
    150L,
    134L,
    110L,
    75L,
    50L,
    0L

}; /* holds the baud rate selections */

/* From man cfsetospeed:
       cfsetospeed()  sets  the  output  baud  rate stored in the
       termios structure pointed to by termios_p to speed,  which
       must be one of these constants:
            B0
            B50
            B75
            B110
            B134
            B150
            B200
            B300
            B600
            B1200
            B1800
            B2400
            B4800
            B9600
            B19200
            B38400
            B57600
            B115200
            B230400
  The  zero  baud rate, B0, is used to terminate the connec­
  tion.  If B0 is specified, the modem control  lines  shall
  no longer be asserted.  Normally, this will disconnect the
  line.*/

t_class *comport_class;

static void comport_pollintervall(t_comport *x, t_floatarg g);
static void comport_tick(t_comport *x);
static float set_baudrate(t_comport *x,t_float baud);
static float set_bits(t_comport *x, int nr);
static float set_parity(t_comport *x,int n);
static float set_stopflag(t_comport *x, int nr);
static int set_ctsrts(t_comport *x, int nr);
static int set_dtr(t_comport *x, int nr);
static int set_rts(t_comport *x, int nr);
static int set_xonxoff(t_comport *x, int nr);
static int set_serial(t_comport *x);
static int write_serial(t_comport *x, unsigned char serial_byte);
static int write_serials(t_comport *x, unsigned char *serial_buf, size_t buf_length);
static int comport_get_dsr(t_comport *x);
static int comport_get_cts(t_comport *x);
#ifdef _WIN32
static HANDLE open_serial(unsigned int com_num, t_comport *x);
static HANDLE close_serial(t_comport *x);
#else
static int set_hupcl(t_comport *x, int nr);
static int open_serial(unsigned int com_num, t_comport *x);
static int close_serial(t_comport *x);
static long get_baud_ratebits(t_float *baud);
#endif
static void comport_pollintervall(t_comport *x, t_floatarg g);
static void comport_tick(t_comport *x);
static void comport_float(t_comport *x, t_float f);
static void comport_list(t_comport *x, t_symbol *s, int argc, t_atom *argv);
static void *comport_new(t_floatarg com_num, t_floatarg fbaud);
static void comport_free(t_comport *x);
static void comport_baud(t_comport *x,t_floatarg f);
static void comport_bits(t_comport *x,t_floatarg f);
static void comport_parity(t_comport *x,t_floatarg f);
static void comport_stopbit(t_comport *x,t_floatarg f);
static void comport_rtscts(t_comport *x,t_floatarg f);
static void comport_dtr(t_comport *x,t_floatarg f);
static void comport_rts(t_comport *x,t_floatarg f);
static void comport_xonxoff(t_comport *x,t_floatarg f);
static void comport_hupcl(t_comport *x,t_floatarg f);
static void comport_close(t_comport *x);
static void comport_open(t_comport *x, t_floatarg f);
static void comport_devicename(t_comport *x, t_symbol *s);
static void comport_print(t_comport *x, t_symbol *s, int argc, t_atom *argv);
static void comport_output_status(t_comport *x, t_symbol *selector, t_float output_value);
static void comport_output_port_status(t_comport *x);
static void comport_output_dsr_status(t_comport *x);
static void comport_output_cts_status(t_comport *x);
static void comport_output_baud_rate(t_comport *x);
static void comport_output_parity_bit(t_comport *x);
static void comport_output_stop_bits(t_comport *x);
static void comport_output_data_bits(t_comport *x);
static void comport_output_rtscts(t_comport *x);
static void comport_output_xonxoff(t_comport *x);
static void comport_output_hupcl(t_comport *x);
static void comport_enum(t_comport *x);
static void comport_info(t_comport *x);
static void comport_devices(t_comport *x);
static void comport_ports(t_comport *x);
static void comport_verbose(t_comport *x, t_floatarg f);
static void comport_help(t_comport *x);
void comport_setup(void);

/* --------- sys independent serial setup helpers ---------------- */



/* ------------ sys dependent serial setup helpers ---------------- */


/* --------------------- NT ------------------------------------ */

#ifdef _WIN32

static float set_baudrate(t_comport *x,t_float baud)
{
    x->dcb.BaudRate = (DWORD)baud ;//!!!try directly setting any baud rate...was get_baud_ratebits(&baud);
    return baud;
}

/* bits are 5,6,7,8(default) */

static float set_bits(t_comport *x, int nr)
{
    if(nr < 4 && nr > 8) nr = 8;

    /*  number of bits/byte, 4-8  */
    return x->dcb.ByteSize = nr;
}


/* 1 ... Parity even, -1 parity odd , 0 (default) no parity */
static float set_parity(t_comport *x,int n)
{
    switch(n)
    {
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


/* activate second stop bit with 1, 0(default)*/
static float set_stopflag(t_comport *x, int nr)
{
    if(nr == 1)
    {
        x->dcb.StopBits = 1; /*  0,1,2 = 1, 1.5, 2  */
        return 1;
    }
    else x->dcb.StopBits = 0; /*  0,1,2 = 1, 1.5, 2  */

    return 0;
}

/* never tested */
static int set_ctsrts(t_comport *x, int nr)
{
    if(nr == 1)
    {
        x->dcb.fOutxCtsFlow = TRUE; /*  CTS output flow control  */
        x->dcb.fRtsControl = RTS_CONTROL_ENABLE; /*  RTS flow control  */
        return 1;
    }
    x->dcb.fOutxCtsFlow = FALSE; /*  CTS output flow control  */
    x->dcb.fRtsControl = RTS_CONTROL_DISABLE; /*  RTS flow control  */
    return 0;
}

static int set_dtr(t_comport *x, int nr)
{
    HANDLE  fd = x->comhandle;
    BOOL    status;
    DWORD   dwFunc = (nr==0)?CLRDTR:SETDTR;

    if (fd == INVALID_HANDLE_VALUE) return -1;

    status = EscapeCommFunction(fd, dwFunc);
    if (status != 0) return nr;
    return -1; /* didn't work, GetLastError tells why */
}

static int set_rts(t_comport *x, int nr)
{
    HANDLE  fd = x->comhandle;
    BOOL    status;
    DWORD   dwFunc = (nr==0)?CLRRTS:SETRTS;

    if (fd == INVALID_HANDLE_VALUE) return -1;

    status = EscapeCommFunction(fd, dwFunc);
    if (status != 0) return nr;
    return -1; /* didn't work, GetLastError tells why */
}

static int set_xonxoff(t_comport *x, int nr)
{
    /*   x->dcb.fTXContinueOnXoff = FALSE;  XOFF continues Tx  */
    if(nr == 1)
    {
        x->dcb.fOutX  = TRUE; /*  XON/XOFF out flow control  */
        x->dcb.fInX  = TRUE; /*  XON/XOFF in flow control */
        return 1;
    }
    x->dcb.fOutX  = FALSE; /*  XON/XOFF out flow control  */
    x->dcb.fInX  = FALSE; /*  XON/XOFF in flow control */
    return 0;
}

static int set_serial(t_comport *x)
{
    if (SetCommState(x->comhandle, &(x->dcb))) return 1;
    /* Didn't work. Get the actual state of the device */
    GetCommState(x->comhandle, &(x->dcb));
    x->baud = x->dcb.BaudRate;
    x->data_bits = x->dcb.ByteSize;
    x->parity_bit = x->dcb.fParity;
    x->stop_bits = x->dcb.StopBits;
    x->xonxoff = (x->dcb.fOutX)?1:0;
    x->ctsrts = (x->dcb.fOutxCtsFlow)?1:0;
    return 0;
}

static int set_hupcl(t_comport *x, int nr)
{
    // this is a dummy function since Windows doesn't use HUPCL, AFAIK hans@eds.org
}

static HANDLE open_serial(unsigned int com_num, t_comport *x)
{
    HANDLE          fd;
    COMMTIMEOUTS    timeouts;
    char            buffer[MAX_PATH];
    float           *baud = &(x->baud);
    DWORD           dw;
    int             i;
    char            *errStr;

    if (com_num != USE_DEVICENAME)
    {
        if(com_num < 1 || com_num >= COMPORT_MAX)
        {
            post("comport number %d out of range (1-%d)", com_num, COMPORT_MAX);
            return INVALID_HANDLE_VALUE;
        }
    
        sprintf(buffer, "%s%d", x->serial_device_prefix, com_num);
        x->serial_device = gensym(buffer);
    }
    else
    {
        sprintf(buffer, "\\\\.\\%s", x->serial_device->s_name); /* assume the slashes were not prefixed by user */
        x->serial_device = gensym(buffer);
    }
    post("Opening %s", &x->serial_device->s_name[4]);/* skip slashes and dot */
    fd = CreateFile( x->serial_device->s_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        0);

    if(fd == INVALID_HANDLE_VALUE)
    {
        dw = GetLastError();
        switch (dw)
        {
            case 2:
                errStr = "ERROR_FILE_NOT_FOUND";
                break;
            case 3:
                errStr = "ERROR_PATH_NOT_FOUND";
                break;
            case 5:
                errStr = "ERROR_ACCESS_DENIED";
                break;
            case 53:
                errStr = "ERROR_BAD_NETPATH";
                break;
            case 123:
                errStr = "ERROR_INVALID_NAME";
                break;
            default:
                errStr = " ";
                break;
        }
        pd_error(x, "[comport]: could not open device %s:\n failure(%d) %s\n",
        &x->serial_device->s_name[4], dw, errStr);
        return INVALID_HANDLE_VALUE;
    }

    /*   Save the Current Port Configuration  */

    if (!GetCommState(fd, &(x->dcb_old)))
    {
        pd_error(x, "[comport]: could not get old dcb of device %s\n",
            &x->serial_device->s_name[4]);
        CloseHandle(fd);
        return INVALID_HANDLE_VALUE;
    }

    memset(&(x->dcb), 0, sizeof(DCB));

    if (!GetCommState(fd, &(x->dcb)))
    {
        pd_error(x, "[comport]: could not get new dcb of device %s\n",
            &x->serial_device->s_name[4]);

        CloseHandle(fd);
        return INVALID_HANDLE_VALUE;
    }

    x->dcb.fBinary = TRUE; /*  binary mode, no EOF check  */

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
    set_bits(x, x->data_bits);      /* CS8 */
    set_stopflag(x, x->stop_bits);  /* ~CSTOPB */
    set_ctsrts(x, x->ctsrts);  /* ~CRTSCTS;*/
    set_xonxoff(x, x->xonxoff); /* (IXON | IXOFF | IXANY) */
    set_baudrate(x, *baud);

    x->comhandle = fd;

	if (com_num == USE_DEVICENAME)
    {
        /* extract index from device name */
        for (i = 0; x->serial_device->s_name[i] != 0; ++i)
            if ((x->serial_device->s_name[i] >= '0') && (x->serial_device->s_name[i] <= '9'))
        com_num = atoi(&x->serial_device->s_name[i]);
    }
    if(set_serial(x))
    {
        post("[comport] opened serial line device %d (%s)\n",
            com_num, &x->serial_device->s_name[4]);
    }
    else
    {
        pd_error(x, "[comport] could not set params to control dcb of device %s\n",
            &x->serial_device->s_name[4]);
        CloseHandle(fd);
        return INVALID_HANDLE_VALUE;
    }

    if (!GetCommTimeouts(fd, &(x->old_timeouts)))
    {
        post("[comport] Couldn't get old timeouts for serial device (%d)", GetLastError());
    }

    /* setting new timeouts for read to immediately return */
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(fd, &timeouts))
    {
        post("Couldn't set timeouts for serial device (%d)", GetLastError());
        return INVALID_HANDLE_VALUE;
    }
	if (!SetupComm(x->comhandle, 4096L, 4096L))/* try to get big buffers to avoid overruns*/
	{
		post("[comport] Couldn't do SetupComm (%d)", GetLastError());
	}
    x->comport = com_num;/* output on next tick */
    return fd;
}

static HANDLE close_serial(t_comport *x)
{
    if(x->comhandle != INVALID_HANDLE_VALUE)
    {
        if (!SetCommState(x->comhandle, &(x->dcb_old)))
        {
            post("[comport] ** ERROR ** couldn't reset params to DCB of device %s\n",
            &x->serial_device->s_name[4]);
        }
        if (!SetCommTimeouts(x->comhandle, &(x->old_timeouts)))
        {
            post("[comport] Couldn't reset old_timeouts for serial device");
        }
        CloseHandle(x->comhandle);
        post("[comport] closed %s", &x->serial_device->s_name[4]);
    }
    return INVALID_HANDLE_VALUE;
}


static int write_serial(t_comport *x, unsigned char serial_byte)
{
    OVERLAPPED osWrite = {0};
    DWORD      dwWritten;
    DWORD      dwToWrite = 1L;
    DWORD      dwErr;
	DWORD      numTransferred = 0L;

    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL)
    {
        post("Couldn't create event. Transmission aborted.");
        return 0;
    }

    if (!WriteFile(x->comhandle, &serial_byte, dwToWrite, &dwWritten, &osWrite))
    {
        dwErr = GetLastError();
        if (dwErr != ERROR_IO_PENDING)
        {
            post("WriteFile error: %d", (int)dwErr);
            return 0;
        }
    }
	if (!GetOverlappedResult(x->comhandle, &osWrite, &numTransferred, TRUE))
	{/* wait for the character to be sent */
        dwErr = GetLastError();
		post("WriteFile:GetOverlappedResult error: %d", (int)dwErr);
	}
    CloseHandle(osWrite.hEvent);
    return 1;
}

static int write_serials(t_comport *x, unsigned char *serial_buf, size_t buf_length)
{
    OVERLAPPED osWrite = {0};
    DWORD      dwWritten;
    DWORD      dwToWrite = (DWORD)buf_length;
    DWORD      dwErr;
	DWORD      numTransferred = 0L;

    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL)
    {
        post("Couldn't create event. Transmission aborted.");
        return 0;
    }

    if (!WriteFile(x->comhandle, serial_buf, dwToWrite, &dwWritten, &osWrite))
    {
        dwErr = GetLastError();
        if (dwErr != ERROR_IO_PENDING)
        {
            post("WriteFile error: %d", (int)dwErr);
            return 0;
        }
    }
	if (!GetOverlappedResult(x->comhandle, &osWrite, &numTransferred, TRUE))
	{/* wait for the character(s) to be sent */
        dwErr = GetLastError();
		post("WriteFile:GetOverlappedResult error: %d", (int)dwErr);
	}
    CloseHandle(osWrite.hEvent);
    return 1;
}

static int comport_get_dsr(t_comport *x)
{
    short  dsr_state = 0;
    if(x->comhandle != INVALID_HANDLE_VALUE)
    {
        DWORD  modemStat;
        /* get the DSR input state and if it's changed, output it */
        BOOL status = GetCommModemStatus(x->comhandle, &modemStat);
        if (status)
        {
            dsr_state = ((modemStat&MS_DSR_ON)!=0);/* read the DSR input line */
        }
        else
        {
            ;/* call GetLastError, only once so as not to hang pd */
        }
    }
    return dsr_state;
}
int comport_get_cts(t_comport *x)
{
    short cts_state = 0;
    if (x->comhandle != INVALID_HANDLE_VALUE)
    {
        DWORD modemStat;
        /* get the DSR input state and if it's changed, output it */
        BOOL status = GetCommModemStatus(x->comhandle, &modemStat);
        if (status)
        {
            cts_state = ((modemStat&MS_CTS_ON)!=0);/* read the CTS input line */
        }
        else
        {
            ;/* call GetLastError, only once so as not to hang pd */
        }
    }
    return cts_state;
}

#else /* NT */
/* ----------------- POSIX - UNIX ------------------------------ */


static long get_baud_ratebits(t_float *baud)
{
    int i = 0;

    while(i < BAUDRATETABLE_LEN && baudratetable[i] > *baud) i++;

	if(baudratetable[i] != *baud)
		post("[comport]: %d not valid, using closest value: ", *baud, baudratetable[i]);
	
    /* nearest Baudrate finding */
    if(i==BAUDRATETABLE_LEN ||  baudspeedbittable[i] < 0)
    {
        post("*Warning* The baud rate %d is not supported or out of range, using 9600\n",*baud);
        i = 8;
    }
    *baud =  baudratetable[i];
	post("get_baud_ratebits: %f", *baud);

    return baudspeedbittable[i];
}

static float set_baudrate(t_comport *x, t_float baud)
{
    struct termios  *tio = &(x->com_termio);
    speed_t            baudbits = get_baud_ratebits(&baud);
	post("set_baudrate baudbits: %d", baudbits);
    if( cfsetispeed(tio, baudbits) != 0 )
		post("[comport]: ERROR failed to set bitrate: %d", baudbits);
    if( cfsetospeed(tio, baudbits) != 0 )
		post("[comport]: ERROR failed to set bitrate: %d", baudbits);

    return baud;
}

/* bits are 5,6,7,8(default) */

static float set_bits(t_comport *x, int nr)
{
    struct termios *tio = &(x->com_termio);
    tio->c_cflag &= ~CSIZE;
    switch(nr)
    {
        case 5: tio->c_cflag |= CS5; return 5;
        case 6: tio->c_cflag |= CS6; return 6;
        case 7: tio->c_cflag |= CS7; return 7;
        default: tio->c_cflag |= CS8;
    }
    return 8;
}


/* 1 ... Parity even, -1 parity odd , 0 (default) no parity */
static float set_parity(t_comport *x,int n)
{
    struct termios *tio = &(x->com_termio);

    switch(n)
    {
        case 1:
            tio->c_cflag |= PARENB;  tio->c_cflag &= ~PARODD; return 1;
        case -1:
            tio->c_cflag |= PARENB | PARODD; return -1;
        default:
            tio->c_cflag &= ~PARENB;
    }
    return 0;
}


/* activate second stop bit with 1, 0(default)*/
static float set_stopflag(t_comport *x, int nr)
{
    struct termios *tio = &(x->com_termio);

    if(nr == 1)
    {
        tio->c_cflag |= CSTOPB;
        return 1;
    }
    else tio->c_cflag &= ~CSTOPB;

    return 0;
}

/* never tested */
static int set_ctsrts(t_comport *x, int nr)
{
    struct termios *tio = &(x->com_termio);

    if(nr == 1)
    {
        tio->c_cflag |= CRTSCTS;
        return 1;
    }
    tio->c_cflag &= ~CRTSCTS;
    return 0;
}

static int set_dtr(t_comport *x, int nr)
{
    int fd = x->comhandle;
    int status;

    if (fd == INVALID_HANDLE_VALUE) return -1;

    ioctl(fd, TIOCMGET, &status);
     if (nr == 0)
        status &= ~TIOCM_DTR;
    else
        status |= TIOCM_DTR;
    ioctl(fd, TIOCMSET, &status);
    return (nr !=0);
}

static int set_rts(t_comport *x, int nr)
{
    int fd = x->comhandle;
    int status;

    if (fd == INVALID_HANDLE_VALUE) return -1;

    ioctl(fd, TIOCMGET, &status);
    if (nr == 0)
        status &= ~TIOCM_RTS;
    else
        status |= TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    return (nr !=0);
}

static int set_xonxoff(t_comport *x, int nr)
{
    struct termios *tio = &(x->com_termio);

    if(nr == 1)
    {
        tio->c_iflag |= (IXON | IXOFF | IXANY);
        return 1;
    }

    tio->c_iflag &= ~IXON & ~IXOFF &  ~IXANY;
    return 0;
}

static int set_hupcl(t_comport *x, int nr)
{
	struct termios settings;
	int result;

	result = tcgetattr(x->comhandle, &settings);
	if (result < 0)
    {
		perror ("error in tcgetattr");
		return 0;
    }
	settings.c_iflag &= ~HUPCL;
	if(nr)
		settings.c_iflag |= HUPCL;
	result = tcsetattr(x->comhandle, TCSANOW, &settings);
	if (result < 0)
    {
		pd_error(x,"[comport] could not set HUPCL");
		return 0;
	}
	x->hupcl = nr;
	return 1;
}

static int open_serial(unsigned int com_num, t_comport *x)
{
    int            fd;
    struct termios *old = &(x->oldcom_termio);
    struct termios *new = &(x->com_termio);
    float          *baud = &(x->baud);
    glob_t         glob_buffer;

    /* if com_num == USE_DEVICENAME, use device name directly, else try port # */
    if(com_num != USE_DEVICENAME)
    {
        if(com_num >= COMPORT_MAX)
        {
            post("[comport] ** WARNING ** port %d not valid, must be between 0 and %d",
                com_num, COMPORT_MAX - 1);
            return INVALID_HANDLE_VALUE;
        }
        /*  post("[comport] globbing %s",x->serial_device_prefix);*/
        /* get the device path based on the port# and the glob pattern */
        switch( glob( x->serial_device_prefix, 0, NULL, &glob_buffer ) )
        {
            case GLOB_NOSPACE:
                error("[comport] out of memory for \"%s\"",x->serial_device_prefix);
                break;
#ifdef GLOB_ABORTED
            case GLOB_ABORTED:
                error("[comport] aborted \"%s\"",x->serial_device_prefix);
                break;
#endif
#ifdef GLOB_NOMATCH
            case GLOB_NOMATCH:
                error("[comport] no serial devices found for \"%s\"",x->serial_device_prefix);
                break;
#endif
        }
        if(com_num < glob_buffer.gl_pathc)
        {
            x->serial_device = gensym(glob_buffer.gl_pathv[com_num]);
        }
        else
        {
            post("[comport] ** WARNING ** port #%d does not exist! (max == %d)",
                com_num,glob_buffer.gl_pathc - 1);
            return INVALID_HANDLE_VALUE;
        }
        globfree( &(glob_buffer) );
    }
    if((fd = open(x->serial_device->s_name, OPENPARAMS)) == INVALID_HANDLE_VALUE)
    {
        error("[comport] ** ERROR ** could not open device %s:\n failure(%d): %s\n",
            x->serial_device->s_name,errno,strerror(errno));
        return INVALID_HANDLE_VALUE;
    }

    /* set no wait on any operation */
    fcntl(fd, F_SETFL, FNDELAY);

    /*   Save the Current Port Configuration  */
    if(tcgetattr(fd, old) == -1 || tcgetattr(fd, new) == -1)
    {
        error("[comport] ** ERROR ** could not get termios-structure of device %s\n",
            x->serial_device->s_name);
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    /* Setup the new port configuration...NON-CANONICAL INPUT MODE
    .. as defined in termios.h */

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

    set_bits(x, x->data_bits);      /* CS8 */
    set_stopflag(x, x->stop_bits);  /* ~CSTOPB */
    set_ctsrts(x, x->ctsrts);  /* ~CRTSCTS;*/
    set_xonxoff(x, x->xonxoff); /* (IXON | IXOFF | IXANY) */
    set_baudrate(x, *baud);

    if(tcsetattr(fd, TCSAFLUSH, new) != -1)
    {
        post("[comport] opened serial line device %d (%s)\n",
            com_num,x->serial_device->s_name);
    }
    else
    {
		error("[comport] ** ERROR ** could not set params to ioctl of device %s\n",
            x->serial_device->s_name);
        close(fd);
        return INVALID_HANDLE_VALUE;
    }
    x->comport = com_num; /* output at next comport_tick */
    return fd;
}

static int close_serial(t_comport *x)
{
    struct termios *tios = &(x->com_termio);
    HANDLE         fd = x->comhandle;

    if(fd != INVALID_HANDLE_VALUE)
    {
        tcsetattr(fd, TCSANOW, tios);
        close(fd);
        post("[comport] closed %s",x->serial_device->s_name);
    }
    return INVALID_HANDLE_VALUE;
}

static int set_serial(t_comport *x)
{
    if(tcsetattr(x->comhandle, TCSAFLUSH, &(x->com_termio)) == -1)
        return 0;
    return 1;
}

static int write_serial(t_comport *x, unsigned char  serial_byte)
{
    int result = write(x->comhandle,(char *) &serial_byte,1);
    if (result != 1)
        post ("[comport] write returned %d, errno is %d", result, errno);
    return result;
    /* flush pending I/O chars */
/* but nowadays discards them ;-(
    else
    {
        ioctl(x->comhandle,TCFLSH,TCOFLUSH);
    }
*/
}

static int write_serials(t_comport *x, unsigned char *serial_buf, size_t buf_length)
{
    int result = write(x->comhandle,(char *)serial_buf, buf_length);
    if (result != (int)buf_length)
        post ("[comport] write returned %d, errno is %d", result, errno);
    return result;
}

static int comport_get_dsr(t_comport *x)
{
    short  dsr_state = 0;

    if (x->comhandle != INVALID_HANDLE_VALUE)
    {
        int status;/*dsr outlet*/
        /* get the DSR input state and if it's changed, output it */
        ioctl(x->comhandle, TIOCMGET, &status);/*dsr outlet*/
        dsr_state = ((status&TIOCM_LE)!=0);/* read the DSR input line */
    }
    return dsr_state;
}

int comport_get_cts(t_comport *x)
{
    short  cts_state = 0;

    if (x->comhandle != INVALID_HANDLE_VALUE)
    {
        int status;/*cts outlet*/
        /* get the CTS input state and if it's changed, output it */
        ioctl(x->comhandle, TIOCMGET, &status);
        cts_state = ((status&TIOCM_CTS)!=0);/* read the CTS input line */
    }
    return cts_state;
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
#ifdef _WIN32
	HANDLE       fd = x->comhandle;
#else
	int  fd = x->comhandle;
#endif /* _WIN32 */
    int          err;

    x->x_hit = 0;

    if(fd == INVALID_HANDLE_VALUE) return;

    /* while there are bytes, read them and send them out, ignore errors */
#ifdef _WIN32
    {
        unsigned char serial_byte[1000];
        DWORD         dwRead;
        OVERLAPPED    osReader = {0};
        DWORD         dwX;

        err = 0;

        osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(ReadFile(x->comhandle, serial_byte, 1000, &dwRead, &osReader))
        {
            if(dwRead > 0)
            {
                for(dwX=0;dwX<dwRead;dwX++)
                {
                    outlet_float(x->x_data_outlet, (t_float) serial_byte[dwX]);
                }
            }
        }
        else
        {
            err = -1;
        }
        CloseHandle(osReader.hEvent);
    }
#else
    {
        unsigned char serial_byte;
        fd_set        com_rfds;
		int count = 0;

        FD_ZERO(&com_rfds);
        FD_SET(fd,&com_rfds);

        while((err=select(fd+1,&com_rfds,NULL,NULL,&null_tv)) > 0)
        {
            err = read(fd,(char *) &serial_byte,1);
            /*  while(    (err = read(fd,(char *) &serial_byte,1)) > 0){ */
            outlet_float(x->x_data_outlet, (t_float) serial_byte);
			++count;
        }
//		if( count > 0)
//			post("--- %d", count);
    }
#endif

    if(err < 0)
    { /* if a read error detected */
        if(x->rxerrors == 0) /* post it once */
            post("RXERRORS on serial line\n");
        x->rxerrors = 1; /* remember */
    }
    if (!x->x_hit) clock_delay(x->x_clock, 1);
}

static void comport_float(t_comport *x, t_float f)
{
    unsigned char serial_byte = ((int) f) & 0xFF; /* brutal conv */

    if (write_serial(x,serial_byte) != 1)
    {
        post("Write error, maybe TX-OVERRUNS on serial line");
    }
}

static void comport_list(t_comport *x, t_symbol *s, int argc, t_atom *argv)
{
    unsigned char   temp_array[MAX_LIST];/* arbitrary maximum list length */
    int             i, count;
    int             result;

    count = argc;
    if (argc > MAX_LIST)
    {
        post ("[comport] truncated list of %d elements to %d", argc, count);
        count = MAX_LIST;
    }
    for(i = 0; i < count; i++)
        temp_array[i] = ((unsigned char)atom_getint(argv+i))&0xFF; /* brutal conv */
    result = write_serials(x, temp_array, count);
    if (result < 0)
        post ("[comport] write returned %d, errno is %d", result, errno);
}

static void *comport_new(t_floatarg com_num, t_floatarg fbaud)
{
    t_comport test;
    t_comport *x;
    HANDLE    fd;

/* for UNIX, this is a glob pattern for matching devices  */
#ifdef _WIN32
/*
According to http://msdn2.microsoft.com/en-us/library/aa363858.aspx 
To specify a COM port number greater than 9,
use the following syntax: "\\\\.\\COM10".
This syntax works for all port numbers and hardware
that allows COM port numbers to be specified.
*/
    const char *serial_device_prefix = "\\\\.\\COM";
#else
# ifdef __APPLE__
    const char *serial_device_prefix = "/dev/tty.*";
# else
#  ifdef IRIX
    const char *serial_device_prefix = "/dev/ttyd*";
#  else
    const char *serial_device_prefix = "/dev/tty[SU]*";
#  endif /* IRIX */
# endif /* __APPLE__ */
#endif /* _WIN32 */


/*	 Open the Comport for RD and WR and get a handle */
/* this line should use a real serial device */
    strncpy(test.serial_device_prefix, serial_device_prefix, strlen(serial_device_prefix)+1);
    test.baud = fbaud;
    test.data_bits = 8; /* default 8 data bits */
    test.parity_bit = 0;/* default no parity bit */
    test.stop_bits = 0;/* default 1 stop bit */
    test.ctsrts = 0; /* default no hardware handshaking */
    test.xonxoff = 0; /* default no software handshaking */
    test.hupcl = 1; /* default hangup on close */
    fd = open_serial((unsigned int)com_num, &test);

    /* Now  nothing really bad could happen so we create the class */
    x = (t_comport *)pd_new(comport_class);

    x->comport = test.comport;/* com_num */
    strncpy(x->serial_device_prefix,serial_device_prefix,strlen(serial_device_prefix)+1);
    x->serial_device = test.serial_device; /* we need this so 'help' doesn't crash */

    x->baud = test.baud;
    x->data_bits = test.data_bits;
    x->parity_bit = test.parity_bit;
    x->stop_bits = test.stop_bits;
    x->ctsrts = test.ctsrts;
    x->xonxoff = test.xonxoff;
    x->hupcl = test.hupcl;
    x->comhandle = fd; /* holds the comport handle */

    if(fd == INVALID_HANDLE_VALUE )
    {
        /* postings in open routine */
        post("[comport] invalid handle for %s", x->serial_device_prefix);
    }
    else
    {
#ifdef _WIN32
        memcpy(&(x->dcb_old), &(test.dcb_old), sizeof(DCB)); /*  save the old com config  */
        memcpy(&(x->dcb), &(test.dcb), sizeof(DCB)); /*  for the new com config  */
#else
        /* save the old com and new com config */
        bcopy(&(test.oldcom_termio),&(x->oldcom_termio),sizeof(struct termios));
        bcopy(&(test.com_termio),&(x->com_termio),sizeof(struct termios));
#endif
    }

    x->rxerrors = 0; /* holds the rx line errors */

    x->x_data_outlet = outlet_new(&x->x_obj, &s_float);
    x->x_status_outlet = outlet_new(&x->x_obj, &s_float);

    x->x_hit = 0;
    x->x_deltime = 1;
    x->x_clock = clock_new(x, (t_method)comport_tick);

    clock_delay(x->x_clock, x->x_deltime);

    x->verbose = 0;

    return x;
}


static void comport_free(t_comport *x)
{
    post("[comport] free serial...");
    clock_unset(x->x_clock);
    clock_free(x->x_clock);
    x->comhandle = close_serial(x);
}

/* ---------------- use serial settings ------------- */

static void comport_baud(t_comport *x,t_floatarg f)
{
    if(f == x->baud)
    {
        post("baudrate already %f\n",x->baud);
        return;
    }

    x->baud = set_baudrate(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(set_serial(x) == 0)
    {
        error("[comport] ** ERROR ** could not set baudrate of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
    }
    else if(x->verbose > 0)
        post("set baudrate of %s to %f\n",
#ifdef _WIN32
            &x->serial_device->s_name[4], x->baud);
#else
            x->serial_device->s_name, x->baud);
#endif
}

static void comport_bits(t_comport *x,t_floatarg f)
{
    f = set_bits(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(set_serial(x) == 0)
    {
        error("[comport] ** ERROR ** could not set bits of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
        return;
    }
    else if(x->verbose > 0)
        post("set bits of %s to %f\n",
#ifdef _WIN32
            &x->serial_device->s_name[4], f);
#else
            x->serial_device->s_name, f);
#endif
    x->data_bits = f;
}


static void comport_parity(t_comport *x,t_floatarg f)
{
    f = set_parity(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(set_serial(x) == 0)
    {
        error("[comport] ** ERROR ** could not set extra paritybit of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
        return;
    }
    else if(x->verbose > 0)
        post("[comport] set extra paritybit of %s to %f\n",
#ifdef _WIN32
            &x->serial_device->s_name[4], f);
#else
            x->serial_device->s_name, f);
#endif
    x->parity_bit = f;
}

static void comport_stopbit(t_comport *x,t_floatarg f)
{
    f = set_stopflag(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(set_serial(x) == 0)
    {
        error("[comport] ** ERROR ** could not set extra stopbit of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
        return;
    }
    else if(x->verbose > 0)
        post("[comport] set extra stopbit of %s to %f\n",
#ifdef _WIN32
            &x->serial_device->s_name[4], f);
#else
            x->serial_device->s_name, f);
#endif
    x->stop_bits = f;
}

static void comport_rtscts(t_comport *x,t_floatarg f)
{
    f = set_ctsrts(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(set_serial(x) == 0)
    {
        error("[comport] ** ERROR ** could not set rts_cts of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
        return;
    }
    else if(x->verbose > 0)
        post("[comport] set rts-cts of %s to %f\n",
#ifdef _WIN32
            &x->serial_device->s_name[4], f);
#else
            x->serial_device->s_name, f);
#endif
    x->ctsrts = f;
}

static void comport_dtr(t_comport *x,t_floatarg f)
{
    f = set_dtr(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(f < 0)
    {
        error("[comport] ** ERROR ** could not set dtr of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
    }
    else if(x->verbose > 0)
        post("[comport] set dtr of %s to %f\n",
#ifdef _WIN32
            &x->serial_device->s_name[4], f);
#else
            x->serial_device->s_name, f);
#endif
}

static void comport_rts(t_comport *x,t_floatarg f)
{
    f = set_rts(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(f < 0)
    {
        error("[comport] ** ERROR ** could not set rts of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
    }
    else if(x->verbose > 0)
        post("[comport] set rts of %s to %f\n",
#ifdef _WIN32
            &x->serial_device->s_name[4], f);
#else
            x->serial_device->s_name, f);
#endif
}

static void comport_xonxoff(t_comport *x,t_floatarg f)
{
    f = set_xonxoff(x,f);

    if(x->comhandle == INVALID_HANDLE_VALUE)return;

    if(set_serial(x) == 0)
    {
        error("[comport] ** ERROR ** could not set xonxoff of device %s\n",
#ifdef _WIN32
            &x->serial_device->s_name[4]);
#else
            x->serial_device->s_name);
#endif
        return;
    }
    else if(x->verbose > 0)
        post("[comport] set xonxoff of %s to %f\n",
#ifdef _WIN32
        &x->serial_device->s_name[4], f);
#else
        x->serial_device->s_name, f);
#endif
    x->xonxoff = f;
}

static void comport_hupcl(t_comport *x,t_floatarg f)
{
#ifndef _WIN32
	set_hupcl(x,f);
#endif
}

static void comport_close(t_comport *x)
{
    clock_unset(x->x_clock);
    x->x_hit = 1;
    x->comhandle = close_serial(x);
    x->comport = -1; /* none */
    if (x->x_status_outlet != NULL) outlet_float(x->x_status_outlet, (float)x->comport);
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
    x->serial_device = s;
    if(x->comhandle != INVALID_HANDLE_VALUE)
        comport_close(x);

    x->comhandle = open_serial(USE_DEVICENAME,x);
    clock_delay(x->x_clock, x->x_deltime);
}

static void comport_print(t_comport *x, t_symbol *s, int argc, t_atom *argv)
{
    static char buf[256];
    char        *pch = buf;

    while(argc--)
    {
        atom_string(argv++, buf, 255);
        while(*pch != 0)
        {
            write_serial(x, *pch++);
        }
        if(argc > 0)
        {
            write_serial(x, ' ');
        }
    }
}

static void comport_enum(t_comport *x)
{
#ifdef _WIN32
    HANDLE          fd;
    char            device_name[10];
	unsigned int    i;
    DWORD           dw;
    for(i = 1; i < COMPORT_MAX; i++)
	{
        sprintf(device_name, "%s%d", x->serial_device_prefix, i);
        fd = CreateFile( device_name,
                GENERIC_READ | GENERIC_WRITE,
                0,
                0,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                0);
        dw = 0L;
        if(fd == INVALID_HANDLE_VALUE)
            dw = GetLastError();
        else
            CloseHandle(fd);
        if (dw == 0)post("\t%d - COM%d (free)", i, i);
        else if (dw == ERROR_ACCESS_DENIED)post("\t%d - COM%d (in use)", i, i);
    }
#else
    unsigned int i;
    glob_t         glob_buffer;
    int            fd;
    struct termios test;

/* first look for registered devices in the filesystem */
    switch( glob( x->serial_device_prefix, 0, NULL, &glob_buffer ) )
    {
    case GLOB_NOSPACE:
        error("[comport] out of memory for \"%s\"",x->serial_device_prefix);
        break;
# ifdef GLOB_ABORTED
        case GLOB_ABORTED:
        error("[comport] aborted \"%s\"",x->serial_device_prefix);
        break;
# endif /* GLOB_ABORTED */
# ifdef GLOB_NOMATCH
    case GLOB_NOMATCH:
        error("[comport] no serial devices found for \"%s\"",x->serial_device_prefix);
        break;
# endif /* GLOB_NOMATCH */
    }
    for(i=0; i<glob_buffer.gl_pathc; i++)
    {
/* now try to open the device */
        if((fd = open(glob_buffer.gl_pathv[i], OPENPARAMS)) != INVALID_HANDLE_VALUE)
        {
/* now see if it has attributes */
            if ((tcgetattr(fd, &test)) != -1)
                post("\t%d\t%s", i, glob_buffer.gl_pathv[i]);// this one really exists
                close (fd);
        }
    }
#endif  /* _WIN32 */
}

static void comport_ports(t_comport *x)
{ /* the same as comport_enum except outputs list of available ports on status outlet */
    unsigned int    i, j = 0;
    t_atom          output_atom[2];
#ifdef _WIN32
    HANDLE          fd;
    char            device_name[10];
    DWORD           dw;

    for(i = 1; i < COMPORT_MAX; i++)
    {
        sprintf(device_name, "%s%d", x->serial_device_prefix, i);
        fd = CreateFile( device_name,
                GENERIC_READ | GENERIC_WRITE,
                0,
                0,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                0);
        dw = 0L;
        if(fd == INVALID_HANDLE_VALUE)
            dw = GetLastError();
        else
            CloseHandle(fd);
        if ((dw == 0)||(dw == ERROR_ACCESS_DENIED))
        { /* output index and name as a list */
            SETFLOAT(&output_atom[0], i);
            SETSYMBOL(&output_atom[1], gensym(&device_name[4]));/* strip the slashes and dot */
            outlet_anything( x->x_status_outlet, gensym("ports"), 2, output_atom);
        }
    }
#else
    glob_t          glob_buffer;
    int             fd;
    struct termios  test;

/* first look for registered devices in the filesystem */
    switch( glob( x->serial_device_prefix, 0, NULL, &glob_buffer ) )
    {
        case GLOB_NOSPACE:
            error("[comport] out of memory for \"%s\"",x->serial_device_prefix);
            break;
# ifdef GLOB_ABORTED
        case GLOB_ABORTED:
            error("[comport] aborted \"%s\"",x->serial_device_prefix);
            break;
# endif /* GLOB_ABORTED */
# ifdef GLOB_NOMATCH
        case GLOB_NOMATCH:
            error("[comport] no serial devices found for \"%s\"",x->serial_device_prefix);
            break;
# endif /* GLOB_NOMATCH */
    }
    for(i = 0; (i < glob_buffer.gl_pathc) && (j < COMPORT_MAX); i++)
    {
/* now try to open the device */
        if((fd = open(glob_buffer.gl_pathv[i], OPENPARAMS)) != INVALID_HANDLE_VALUE)
        {
/* now see if it has attributes */
            if ((tcgetattr(fd, &test)) != -1)
            { /* output index and name as a list */
                SETFLOAT(&output_atom[0], i);
                SETSYMBOL(&output_atom[1], gensym(glob_buffer.gl_pathv[i]));
                outlet_anything( x->x_status_outlet, gensym("ports"), 2, output_atom);
            }
            close (fd);
        }
    }
#endif  /* _WIN32 */
}

static void comport_output_print(t_comport *x)
{
    post("[comport]: available serial ports:");
    comport_enum(x);
}


static void comport_output_status(t_comport *x, t_symbol *selector, t_float output_value)
{
    t_atom *output_atom = getbytes(sizeof(t_atom));
    SETFLOAT(output_atom, output_value);
    outlet_anything( x->x_status_outlet, selector, 1, output_atom);
    freebytes(output_atom,sizeof(t_atom));
}

static void comport_output_port_status(t_comport *x)
{
    comport_output_status(x, gensym("port"), (float)x->comport);
}

static void comport_output_dsr_status(t_comport *x)
{
    comport_output_status(x, gensym("dsr"), (float)comport_get_dsr(x));
}

static void comport_output_cts_status(t_comport *x)
{
    comport_output_status(x, gensym("cts"), (float)comport_get_cts(x));
}

static void comport_output_baud_rate(t_comport *x)
{
    comport_output_status(x, gensym("baud"), x->baud);
}

static void comport_output_parity_bit(t_comport *x)
{
    comport_output_status(x, gensym("parity"), x->parity_bit);
}

static void comport_output_stop_bits(t_comport *x)
{
    comport_output_status(x, gensym("stop"), x->stop_bits+1);
}

static void comport_output_data_bits(t_comport *x)
{
    comport_output_status(x, gensym("data"), x->data_bits);
}

static void comport_output_rtscts(t_comport *x)
{
    comport_output_status(x, gensym("rtscts"), x->ctsrts);
}

static void comport_output_xonxoff(t_comport *x)
{
    comport_output_status(x, gensym("xonxoff"), x->xonxoff);
}

static void comport_output_hupcl(t_comport *x)
{
    comport_output_status(x, gensym("hupcl"), x->hupcl);
}

static void comport_output_open_status(t_comport *x)
{
    if(x->comhandle == INVALID_HANDLE_VALUE)
		comport_output_status(x, gensym("open"), 0);
	else
		comport_output_status(x, gensym("open"), 1);
}

static void comport_devices(t_comport *x)
{
	comport_output_print(x);
}

static void comport_info(t_comport *x)
{
	comport_output_open_status(x);
    comport_output_port_status(x);
    comport_output_baud_rate(x);
    comport_output_dsr_status(x);
    comport_output_cts_status(x);
    comport_output_parity_bit(x);
    comport_output_stop_bits(x);
    comport_output_data_bits(x);
    comport_output_rtscts(x);
    comport_output_xonxoff(x);
    comport_output_hupcl(x);
}

/* ---------------- HELPER ------------------------- */
static void comport_verbose(t_comport *x, t_floatarg f)
{
    x->verbose = f;
    if(f > 0) post("[comport] verbose is on: %d", (int) f);
}

static void comport_help(t_comport *x)
{
    post("[comport] serial port %d (baud %f):", x->comport, x->baud);
    if(x->comport >= 0 && x->comport < COMPORT_MAX)
    {
#ifdef WIN32
        post("\tdevicename: %s", &x->serial_device->s_name[4]);
#else
        post("\tdevicename: %s", x->serial_device->s_name);
#endif
    }

    post("  Methods:");
    post("   baud <baud>       ... set baudrate to nearest possible baud\n"
        "   bits <bits>       ... set number of bits (7 or 8)\n"
        "   stopbit <0|1>     ... set off|on stopbit\n"
        "   rtscts <0|1>      ... set rts/cts off|on\n"
        "   parity <0|1>      ... set parity off|on\n"
        "   xonxoff <0|1>     ... set xon/xoff off|on\n"
        "   dtr <0|1>         ... set dtr off|on\n"
        "   rts <0|1>         ... set rts off|on\n"
        "   hupcl <0|1>       ... set hang-up on close off|on\n"
        "   close             ... close device\n"
        "   open <num>        ... open device number num\n"
        "   devicename <d>    ... set device name to d (eg. /dev/ttyS8)\n"
        "   print <list>      ... print list of atoms on serial\n"
        "   pollintervall <t> ... set poll interval to t ticks\n"
        "   verbose <level>   ... for debug set verbosity to level\n"
        "   info              ... output info on status outlet\n"
        "   devices           ... post list of available devices\n"
        "   ports             ... output list of available devices on status outlet\n"
        "   help              ... post this help");
}

/* ---------------- SETUP OBJECTS ------------------ */
void comport_setup(void)
{
    comport_class = class_new(gensym("comport"), (t_newmethod)comport_new,
        (t_method)comport_free, sizeof(t_comport),
        0, A_DEFFLOAT, A_DEFFLOAT, 0);

    class_addfloat(comport_class, (t_method)comport_float);
    class_addlist(comport_class, (t_method)comport_list);
    /*
        class_addbang(comport_class, comport_bang
    */
    class_addmethod(comport_class, (t_method)comport_baud, gensym("baud"),A_FLOAT, 0);

    class_addmethod(comport_class, (t_method)comport_bits, gensym("bits"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_stopbit, gensym("stopbit"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_rtscts, gensym("rtscts"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_dtr, gensym("dtr"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_rts, gensym("rts"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_parity, gensym("parity"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_xonxoff, gensym("xonxoff"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_hupcl, gensym("hupcl"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_close, gensym("close"), 0);
    class_addmethod(comport_class, (t_method)comport_open, gensym("open"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_devicename, gensym("devicename"), A_SYMBOL, 0);
    class_addmethod(comport_class, (t_method)comport_print, gensym("print"), A_GIMME, 0);
    class_addmethod(comport_class, (t_method)comport_pollintervall, gensym("pollintervall"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_verbose, gensym("verbose"), A_FLOAT, 0);
    class_addmethod(comport_class, (t_method)comport_help, gensym("help"), 0);
    class_addmethod(comport_class, (t_method)comport_info, gensym("info"), 0);
    class_addmethod(comport_class, (t_method)comport_devices, gensym("devices"), 0);
    class_addmethod(comport_class, (t_method)comport_ports, gensym("ports"), 0);

#ifndef _WIN32
    null_tv.tv_sec = 0; /* no wait */
    null_tv.tv_usec = 0;
#endif /* NOT _WIN32 */
    post("comport - PD external for unix/windows\n"
        "LGPL 1998-2006,  Winfried Ritsch and others (see LICENCE.txt)\n"
        "Institute for Electronic Music - Graz");
}

