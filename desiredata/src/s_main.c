/* Copyright (c) 1997-1999 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Zmölnig added advanced multidevice-support (2001) */

#include "desire.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sstream>

#ifdef __CYGWIN__
#define UNISTD
#endif
#ifdef UNISTD
#include <unistd.h>
#endif
#ifdef MSW
#include <io.h>
#include <windows.h>
#include <winbase.h>
#endif

#ifdef DAZ
#define _MM_DENORM_ZERO_ON 0x0040
#include <xmmintrin.h>
#endif

extern "C" void pd_init();
extern "C" int sys_argparse(int argc, char **argv);
void sys_findprogdir(char *progname);
int sys_startgui();
void sys_init_idle_callbacks();
extern "C" int sys_rcfile();
extern int m_scheduler();
void sys_addhelppath(char *p);
void alsa_adddev(char *name);
extern "C" int sys_parsercfile(char*filename);

#ifdef THREADED_SF
void sys_start_sfthread();
#endif /* THREDED_SF */

char pd_version[] = "DesireData 2009.04.24";
char pd_compiletime[] = __TIME__;
char pd_compiledate[] = __DATE__;

int sys_verbose;
int sys_noloadbang;
int sys_hipriority = -1;    /* -1 = don't care; 0 = no; 1 = yes */
int sys_guisetportnumber = 5400;

int sys_defeatrt;
t_symbol *sys_flags;

char *sys_guicmd;
t_symbol *sys_libdir;
t_namelist *sys_externlist =0;
t_namelist *sys_openlist   =0;
t_namelist *sys_messagelist=0;
static int sys_version;

int sys_nmidiout = 1;
#ifdef MSW
int sys_nmidiin = 0;
#else
int sys_nmidiin = 1;
#endif
int sys_midiindevlist[MAXMIDIINDEV] = {1};
int sys_midioutdevlist[MAXMIDIOUTDEV] = {1};

static int sys_main_srate;
static int sys_main_dacblocksize = DEFDACBLKSIZE;
static int sys_main_advance;

/* jsarlo { */
int sys_externalschedlib;
char *sys_externalschedlibname;
int sys_extraflags;
char *sys_extraflagsstring;
/* } jsarlo */

/* IOhannes { */
/* here the "-1" counts signify that the corresponding vector hasn't been
   specified in command line arguments; sys_open_audio will detect this and fill things in. */
static int sys_nsoundin = -1;  static int  sys_soundindevlist[MAXAUDIOINDEV];
static int sys_nsoundout = -1; static int sys_soundoutdevlist[MAXAUDIOOUTDEV];
static int sys_nchin = -1;     static int        sys_chinlist[MAXAUDIOINDEV];
static int sys_nchout = -1;    static int       sys_choutlist[MAXAUDIOOUTDEV];
/* } IOhannes */

/* jsarlo { */
t_sample *get_sys_soundout()          {return sys_soundout;}
t_sample *get_sys_soundin()           {return sys_soundin;}
int *     get_sys_main_advance()      {return &sys_main_advance;}
double *  get_sys_time_per_dsp_tick() {return &sys_time_per_dsp_tick;} 
int *     get_sys_schedblocksize()    {return &sys_schedblocksize;}
double *  get_sys_time()              {return &sys_time;}
float *   get_sys_dacsr()             {return &sys_dacsr;}
int *     get_sys_sleepgrain()        {return &sys_sleepgrain;}
int *     get_sys_schedadvance()      {return &sys_schedadvance;}
/* } jsarlo */

static void sys_afterargparse();

/* this is called from main() in s_entry.c */
extern "C" int sys_main(int argc, char **argv) {
    int noprefs = 0;
    class_table = new t_hash<t_symbol*,t_class*>(127);
    /* jsarlo { */
    sys_externalschedlib = 0;
    sys_extraflags = 0;
    /* } jsarlo */
#ifdef DEBUG
    fprintf(stderr, "Pd: COMPILED FOR DEBUGGING\n");
#endif
    pd_init(); /* start the message system */
    sys_findprogdir(argv[0]); /* set sys_progname, guipath */
    /* tb: command line flag to defeat preset loading */
    for (int i=0; i<argc; i++) if (!strcmp(argv[i],"-noprefs") || !strcmp(argv[i],"-rcfile")) noprefs = 1;
    if (!noprefs) sys_rcfile(); /* parse the startup file */
    /* initalize idle callbacks before starting the gui */
    sys_init_idle_callbacks();
    /* } tb */
    if (sys_argparse(argc-1, argv+1)) return 1; /* parse cmd line */
    sys_afterargparse(); /* post-argparse settings */
    if (sys_verbose || sys_version) fprintf(stderr, "%s, compiled %s %s\n", pd_version, pd_compiletime, pd_compiledate);
    if (sys_version) return 0; /* if we were just asked our version, exit here. */
    if (sys_startgui()) return 1;      /* start the gui */
    /* tb: { start the soundfiler helper thread */
#ifdef THREADED_SF
    sys_start_sfthread();
#endif /* THREDED_SF */
    /* try to set ftz and daz */
#ifdef DAZ
    _mm_setcsr(_MM_FLUSH_ZERO_ON | _MM_MASK_UNDERFLOW | _mm_getcsr());
    _mm_setcsr(_MM_DENORM_ZERO_ON | _mm_getcsr());
#endif /* DAZ */
    /* } tb */
    /* jsarlo { */
    if (sys_externalschedlib) {
#ifdef MSW
        typedef int (*t_externalschedlibmain)(char *);
        t_externalschedlibmain externalmainfunc;
        HINSTANCE ntdll;
        char *filename;
        asprintf(&filename,"%s.dll", sys_externalschedlibname);
        sys_bashfilename(filename, filename);
        ntdll = LoadLibrary(filename);
        if (!ntdll) {
              post("%s: couldn't load external scheduler lib ", filename);
              free(filename);
              return 0;
        }
	free(filename);
        externalmainfunc = (t_externalschedlibmain)GetProcAddress(ntdll,"main");
        return externalmainfunc(sys_extraflagsstring);
#else
        return 0;
#endif
    }
    /* open audio and MIDI */
    sys_reopen_midi();
    sys_reopen_audio();
    /* run scheduler until it quits */
    int r = m_scheduler();
    fprintf(stderr,"%s",lost_posts.str().data()); // this could be useful if anyone ever called sys_exit
    return r;
}

static const char *(usagemessage[]) = {
"usage: pd [-flags] [file]...","",
"audio configuration flags:",
"-r <n>           -- specify sample rate",
"-audioindev ...  -- audio in devices; e.g., \"1,3\" for first and third",
"-audiooutdev ... -- audio out devices (same)",
"-audiodev ...    -- specify input and output together",
"-inchannels ...  -- audio input channels (by device, like \"2\" or \"16,8\")",
"-outchannels ... -- number of audio out channels (same)",
"-channels ...    -- specify both input and output channels",
"-audiobuf <n>    -- specify size of audio buffer in msec",
"-blocksize <n>   -- specify audio I/O block size in sample frames",
"-dacblocksize <n>-- specify audio dac~block size in samples",
"-sleepgrain <n>  -- specify number of milliseconds to sleep when idle",
"-cb_scheduler    -- use callback-based scheduler (jack and native asio only)",
"-nodac           -- suppress audio output",
"-noadc           -- suppress audio input",
"-noaudio         -- suppress audio input and output (-nosound is synonym)",

#define NOT_HERE "(support not compiled in)"

#ifdef USEAPI_OSS
"-oss             -- use OSS audio API",
"-32bit           ---- allow 32 bit OSS audio (for RME Hammerfall)",
#else
"-oss             -- OSS "NOT_HERE,
"-32bit           ---- allow 32 bit OSS audio "NOT_HERE,
#endif

#ifdef USEAPI_ALSA
"-alsa            -- use ALSA audio API",
"-alsaadd <name>  -- add an ALSA device name to list",
#else
"-alsa            -- use ALSA audio API "NOT_HERE,
"-alsaadd <name>  -- add an ALSA device name to list "NOT_HERE,
#endif

#ifdef USEAPI_JACK
"-jack            -- use JACK audio API",
#else
"-jack            -- use JACK audio API "NOT_HERE,
#endif

#ifdef USEAPI_ASIO
	"-asio_native     -- use native ASIO API",
#else
	"-asio_native     -- use native ASIO API "NOT_HERE,
#endif

#ifdef USEAPI_PORTAUDIO
"-pa              -- use Portaudio API",
"-asio            -- synonym for -pa - use ASIO via Portaudio",
#else
"-pa              -- use Portaudio API "NOT_HERE,
"-asio            -- synonym for -pa - use ASIO via Portaudio "NOT_HERE,
#endif

#ifdef USEAPI_MMIO
"-mmio            -- use MMIO audio API",
#else
"-mmio            -- use MMIO audio API "NOT_HERE,
#endif
#ifdef API_DEFSTRING
"      (default audio API for this platform:  "API_DEFSTRING")","",
#endif

"MIDI configuration flags:",
"-midiindev ...   -- midi in device list; e.g., \"1,3\" for first and third",
"-midioutdev ...  -- midi out device list, same format",
"-mididev ...     -- specify -midioutdev and -midiindev together",
"-nomidiin        -- suppress MIDI input",
"-nomidiout       -- suppress MIDI output",
"-nomidi          -- suppress MIDI input and output",
#ifdef USEAPI_ALSA
"-alsamidi        -- use ALSA midi API",
#else
"-alsamidi        -- use ALSA midi API "NOT_HERE,
#endif

"","other flags:",
"-rcfile <file>   -- read this rcfile instead of default",
"-noprefs         -- suppress loading preferences on startup",
"-path <path>     -- add to file search path",
"-nostdpath       -- don't search standard (\"extra\") directory",
"-stdpath         -- search standard directory (true by default)",
"-helppath <path> -- add to help file search path",
"-open <file>     -- open file(s) on startup",
"-lib <file>      -- load object library(s)",
"-verbose         -- extra printout on startup and when searching for files",
"-version         -- don't run Pd; just print out which version it is",
"-d <n>           -- specify debug level",
"-noloadbang      -- suppress all loadbangs",
"-stdout          -- send printout to standard output (1>) instead of GUI",
"-stderr          -- send printout to standard error  (2>) instead of GUI",
"-guiport <n>     -- set port that the GUI should connect to",
"-send \"msg...\"   -- send a message at startup, after patches are loaded",
#ifdef UNISTD
"-rt or -realtime -- use real-time priority",
"-nrt             -- don't use real-time priority",
#else
"-rt or -realtime -- use real-time priority "NOT_HERE,
"-nrt             -- don't use real-time priority "NOT_HERE,
#endif
};

static void sys_parsedevlist(int *np, int *vecp, int max, char *str) {
    int n = 0;
    while (n < max) {
        if (!*str) break;
        else {
            char *endp;
            vecp[n] = strtol(str, &endp, 10);
            if (endp == str) break;
            n++;
            if (!endp) break;
            str = endp + 1;
        }
    }
    *np = n;
}

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "."
#endif

/* this routine tries to figure out where to find the auxilliary files Pd will need to run.
   This is either done by looking at the command line invokation for Pd, or if that fails,
   by consulting the variable INSTALL_PREFIX.  In MSW, we don't try to use INSTALL_PREFIX. */
void sys_findprogdir(char *progname) {
    char *lastslash;
#ifdef UNISTD
    struct stat statbuf;
#endif
    /* find out by what string Pd was invoked; put answer in "sbuf". */
    char *sbuf;
#ifdef MSW
    int len = GetModuleFileName(NULL, sbuf, 0);
    sbuf = (char *)malloc(len+1);
    GetModuleFileName(NULL, sbuf, len);
    sbuf[len] = 0;
    sys_unbashfilename(sbuf,sbuf);
#endif /* MSW */
#ifdef UNISTD
    sbuf = strdup(progname);
#endif
    lastslash = strrchr(sbuf, '/');
    char *sbuf2;
    if (lastslash) {
        /* bash last slash to zero so that sbuf is directory pd was in, e.g., ~/pd/bin */
        *lastslash = 0;
        /* go back to the parent from there, e.g., ~/pd */
        lastslash = strrchr(sbuf, '/');
        if (lastslash) {
            asprintf(&sbuf2, "%.*s", lastslash-sbuf, sbuf);
        } else sbuf2 = strdup("..");
    } else { /* no slashes found.  Try INSTALL_PREFIX. */
        sbuf2 = strdup(INSTALL_PREFIX);
    }
#ifdef MSW
    sys_libdir = gensym(sbuf2);
#else
    sys_libdir = stat(sbuf, &statbuf)>=0 ? symprintf("%s/lib/pd",sbuf2) : gensym(sbuf2);
#endif
    free(sbuf);
    post("sys_libdir = %s",sys_libdir->name);
}

#ifdef MSW
static int sys_mmio = 1;
#else
static int sys_mmio = 0;
#endif

#undef NOT_HERE
#define NOT_HERE fprintf(stderr,"option %s not compiled into this pd\n", *argv)

#define ARG(name,count) (!strcmp(*argv,(name)) && argc>=(count))
#define NEXT(count) argc -= (count), argv += (count); continue;

int sys_argparse(int argc, char **argv) {
    while ((argc > 0) && **argv == '-') {
        if (ARG("-r",2)) {
	    if (sscanf(argv[1], "%d", &sys_main_srate)<1) goto usage;
            NEXT(2);
        }
	if (ARG("-inchannels",2)) { /* IOhannes */
            sys_parsedevlist(&sys_nchin, sys_chinlist, MAXAUDIOINDEV, argv[1]);
            if (!sys_nchin) goto usage;
            NEXT(2);
        }
        if (ARG("-outchannels",2)) { /* IOhannes */
            sys_parsedevlist(&sys_nchout, sys_choutlist, MAXAUDIOOUTDEV, argv[1]);
            if (!sys_nchout) goto usage;
            NEXT(2);
        }
	if (ARG("-channels",2)) {
            sys_parsedevlist(&sys_nchin, sys_chinlist,MAXAUDIOINDEV, argv[1]);
            sys_parsedevlist(&sys_nchout, sys_choutlist,MAXAUDIOOUTDEV, argv[1]);
            if (!sys_nchout) goto usage;
            NEXT(2);
        }
	if (ARG("-soundbuf",2) || ARG("-audiobuf",2)) {
            sys_main_advance = atoi(argv[1]);
            NEXT(2);
        }
        if (ARG("-blocksize",2)) {sys_setblocksize(atoi(argv[1])); NEXT(2);}
	if (ARG("-dacblocksize",2)) {
		if (sscanf(argv[1], "%d", &sys_main_dacblocksize)<1) goto usage;
		NEXT(2);
	}
        if (ARG("-sleepgrain",2)) {sys_sleepgrain = int(1000 * atof(argv[1])); NEXT(2);}

	/* from tim */
        if (ARG("-cb_scheduler",1)) {sys_setscheduler(1); NEXT(1);}

	/* IOhannes */
	if (ARG("-nodac",1)) {sys_nsoundout= sys_nchout = 0; NEXT(1);}
        if (ARG("-noadc",1)) {sys_nsoundin = sys_nchin =  0; NEXT(1);}
        if (ARG("-nosound",1) || ARG("-noaudio",1)) {
            sys_nsoundin=sys_nsoundout = 0;
            sys_nchin = sys_nchout = 0;
            NEXT(1);
        }

	/* many options for selecting audio api */
        if (ARG("-oss",1)) {
#ifdef USEAPI_OSS
            sys_set_audio_api(API_OSS);
#else
	    NOT_HERE;
#endif
            NEXT(1);
        }
        if (ARG("-32bit",1)) {
#ifdef USEAPI_OSS
            sys_set_audio_api(API_OSS);
            oss_set32bit();
#else
	    NOT_HERE;
#endif
            NEXT(1);
        }
        if (ARG("-alsa",1)) {
#ifdef USEAPI_ALSA
            sys_set_audio_api(API_ALSA);
#else
	    NOT_HERE;
#endif
            NEXT(1);
        }
        if (ARG("-alsaadd",2)) {
#ifdef USEAPI_ALSA
            if (argc>1) alsa_adddev(argv[1]); else goto usage;
#else
	    NOT_HERE;
#endif
            NEXT(2);
        }
        if (ARG("-alsamidi",1)) {
#ifdef USEAPI_ALSA
          sys_set_midi_api(API_ALSA);
#else
	    NOT_HERE;
#endif
            NEXT(1);
        }
        if (ARG("-jack",1)) {
#ifdef USEAPI_JACK
            sys_set_audio_api(API_JACK);
#else
	    NOT_HERE;
#endif
            NEXT(1);
        }
        if (ARG("-pa",1) || ARG("-portaudio",1) || ARG("-asio",1)) {
#ifdef USEAPI_PORTAUDIO
            sys_set_audio_api(API_PORTAUDIO);
#else
	    NOT_HERE;
#endif
            sys_mmio = 0;
            argc--; argv++;
        }
    	if (ARG("-asio_native",1)) {
#ifdef USEAPI_ASIO
    	    sys_set_audio_api(API_ASIO);
#else
	    NOT_HERE;
#endif
	    sys_mmio = 0;
    	    argc--; argv++;
    	}
        if (ARG("-mmio",1)) {
#ifdef USEAPI_MMIO
            sys_set_audio_api(API_MMIO);
#else
	    NOT_HERE;
#endif
            sys_mmio = 1;
            NEXT(1);
        }

        if (ARG("-nomidiin", 1)) {sys_nmidiin = 0;  NEXT(1);}
        if (ARG("-nomidiout",1)) {sys_nmidiout = 0; NEXT(1);}
        if (ARG("-nomidi",1))    {sys_nmidiin = sys_nmidiout = 0; NEXT(1);}
        if (ARG("-midiindev",2)) {
            sys_parsedevlist(&sys_nmidiin, sys_midiindevlist, MAXMIDIINDEV, argv[1]);
            if (!sys_nmidiin) goto usage;
            NEXT(2);
        }
        if (ARG("-midioutdev",2)) {
            sys_parsedevlist(&sys_nmidiout, sys_midioutdevlist, MAXMIDIOUTDEV, argv[1]);
            if (!sys_nmidiout) goto usage;
            NEXT(2);
        }
        if (ARG("-mididev",2)) {
            sys_parsedevlist(&sys_nmidiin, sys_midiindevlist, MAXMIDIINDEV, argv[1]);
            sys_parsedevlist(&sys_nmidiout, sys_midioutdevlist, MAXMIDIOUTDEV, argv[1]);
            if (!sys_nmidiout) goto usage;
            NEXT(2);
        }
        if (ARG("-nostdpath",1)) {sys_usestdpath = 0; NEXT(1);}
        if (ARG("-stdpath",1)) {sys_usestdpath = 1; NEXT(1);}
        if (ARG("-path",2))   {sys_searchpath = namelist_append_files(sys_searchpath,argv[1]); NEXT(2);}
        if (ARG("-helppath",2)) {sys_helppath = namelist_append_files(sys_helppath,  argv[1]); NEXT(2);}
        if (ARG("-open",2))     {sys_openlist = namelist_append_files(sys_openlist,  argv[1]); NEXT(2);}
        if (ARG("-lib",2))    {sys_externlist = namelist_append_files(sys_externlist,argv[1]); NEXT(2);}
        if (ARG("-font",2)) {
	    fprintf(stderr,"Warning: -font ignored by DesireData; use .ddrc instead\n");
            //sys_defaultfont = sys_nearestfontsize(atoi(argv[1]));
            NEXT(2);
        }
    	if (ARG("-typeface",2)) { /* tim */
	    fprintf(stderr,"Warning: -typeface ignored by DesireData; use .ddrc instead\n");
	    NEXT(2);
	}
    	if (ARG("-noprefs",1)) {NEXT(1);} /* tim: skip flag, we already parsed it */
	/* jmz: read an alternative rcfile { */
    	if (ARG("-rcfile",2)) {sys_parsercfile(argv[1]); NEXT(2);} /* recursively */
	/* } jmz */
        if (ARG("-verbose",1)) {sys_verbose++; NEXT(1);}
        if (ARG("-version",1)) {sys_version = 1; NEXT(1);}
        if (ARG("-noloadbang",1)) {sys_noloadbang = 1; NEXT(1);}
        if (ARG("-nogui",1)) {
		fprintf(stderr,"Warning: -nogui is obsolete: nowadays it does just like -stderr instead\n");
		sys_printtofh = stderr; NEXT(1);}
        if (ARG("-guiport",2)) {
		if (sscanf(argv[1], "%d", &sys_guisetportnumber)<1) goto usage;
		NEXT(2);
	}
        if (ARG("-stdout",1)) {sys_printtofh = stdout; NEXT(1);}
        if (ARG("-stderr",1)) {sys_printtofh = stderr; NEXT(1);}
        if (ARG("-guicmd",2)) {fprintf(stderr,"Warning: -guicmd ignored"); NEXT(2);}
        if (ARG("-send",2)) {sys_messagelist = namelist_append(sys_messagelist,argv[1],1); NEXT(2);}
        /* jsarlo { */
        if (ARG("-schedlib",2)) {
            sys_externalschedlib = 1;
            sys_externalschedlibname = strdup(argv[1]);
            NEXT(2);
        }
        if (ARG("-extraflags",2)) {
            sys_extraflags = 1;
            sys_extraflagsstring = strdup(argv[1]);
            NEXT(2);
        }
        /* } jsarlo */
#ifdef UNISTD
        if (ARG("-rt",1) || ARG("-realtime",1)) {sys_hipriority = 1; NEXT(1);}
        if (ARG("-nrt",1))                      {sys_hipriority = 0; NEXT(1);}
#endif
        if (ARG("-soundindev",2) || ARG("-audioindev",2)) { /* IOhannes */
          sys_parsedevlist(&sys_nsoundin, sys_soundindevlist, MAXAUDIOINDEV, argv[1]);
          if (!sys_nsoundin) goto usage;
          NEXT(2);
        }
        if (ARG("-soundoutdev",2) || ARG("-audiooutdev",2)) { /* IOhannes */
          sys_parsedevlist(&sys_nsoundout, sys_soundoutdevlist, MAXAUDIOOUTDEV, argv[1]);
          if (!sys_nsoundout) goto usage;
          NEXT(2);
        }
        if (ARG("-sounddev",2) || ARG("-audiodev",2)) {
          sys_parsedevlist(&sys_nsoundin,  sys_soundindevlist,  MAXAUDIOINDEV,  argv[1]);
          sys_parsedevlist(&sys_nsoundout, sys_soundoutdevlist, MAXAUDIOOUTDEV, argv[1]);
          if (!sys_nsoundout) goto usage;
          NEXT(2);
        }
        usage:
	    if (argc) fprintf(stderr, "Can't handle option '%s'.\n",*argv);
            for (size_t i=0; i < sizeof(usagemessage)/sizeof(*usagemessage); i++)
                fprintf(stderr, "%s\n", usagemessage[i]);
            return 1;
    }
    for (; argc > 0; argc--, argv++) sys_openlist = namelist_append_files(sys_openlist, *argv);
    return 0;
}

int sys_getblksize() {return sys_dacblocksize;}

/* stuff to do, once, after calling sys_argparse() -- which may itself
   be called more than once (first from "settings, second from .pdrc, then
   from command-line arguments */
static void sys_afterargparse() {
    char *sbuf;
    t_audiodevs audio_in, audio_out;
    int nchindev, nchoutdev, rate, dacblksize, advance, scheduler;
    int nmidiindev = 0, midiindev[MAXMIDIINDEV];
    int nmidioutdev = 0, midioutdev[MAXMIDIOUTDEV];
    /* add "extra" library to path */
    asprintf(&sbuf,"%s/extra",sys_libdir->name);
    sys_setextrapath(sbuf);
    free(sbuf);
    asprintf(&sbuf,"%s/doc/5.reference",sys_libdir->name);
    sys_helppath = namelist_append_files(sys_helppath, sbuf);
    free(sbuf);
    /* correct to make audio and MIDI device lists zero based.  On MMIO, however, "1" really means the second device
       the first one is "mapper" which is was not included when the command args were set up, so we leave it that way for compatibility. */
    if (!sys_mmio) {
        for (int i=0; i<sys_nsoundin ; i++) sys_soundindevlist[i]--;
        for (int i=0; i<sys_nsoundout; i++) sys_soundoutdevlist[i]--;
    }
    for (int i=0; i<sys_nmidiin;  i++) sys_midiindevlist[i]--;
    for (int i=0; i<sys_nmidiout; i++) sys_midioutdevlist[i]--;
    /* get the current audio parameters.  These are set by the preferences mechanism (sys_loadpreferences()) or
       else are the default.  Overwrite them with any results of argument parsing, and store them again. */
    sys_get_audio_params(&audio_in, &audio_out, &rate, &dacblksize, &advance, &scheduler);
    nchindev  =  sys_nchin>=0 ?  sys_nchin :  audio_in.ndev;
    nchoutdev = sys_nchout>=0 ? sys_nchout : audio_out.ndev;
    if (sys_nchin >=0) {for (int i=0; i< nchindev; i++)  audio_in.chdev[i] = sys_chinlist[i];}
    if (sys_nchout>=0) {for (int i=0; i<nchoutdev; i++) audio_out.chdev[i] = sys_choutlist[i];}
    if (sys_nsoundin >=0) {audio_in.ndev  = sys_nsoundin; for (int i=0; i< audio_in.ndev; i++)  audio_in.dev[i] = sys_soundindevlist[i];}
    if (sys_nsoundout>=0) {audio_out.ndev = sys_nsoundout;for (int i=0; i<audio_out.ndev; i++) audio_out.dev[i] = sys_soundoutdevlist[i];}
    if (sys_nmidiin >=0)  {nmidiindev     = sys_nmidiin;  for (int i=0; i<    nmidiindev; i++)     midiindev[i] = sys_midiindevlist[i];}
    if (sys_nmidiout>=0)  {nmidioutdev    = sys_nmidiout; for (int i=0; i<   nmidioutdev; i++)    midioutdev[i] = sys_midioutdevlist[i];}
    if (sys_main_advance) advance = sys_main_advance;
    if (sys_main_srate)      rate = sys_main_srate;
    if (sys_main_dacblocksize) dacblksize = sys_main_dacblocksize;
    sys_open_audio(audio_in.ndev,  audio_in.dev,  nchindev,  audio_in.chdev,
		  audio_out.ndev, audio_out.dev, nchoutdev, audio_out.chdev, rate, dacblksize, advance, scheduler, 0);
    sys_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev, 0);
}
