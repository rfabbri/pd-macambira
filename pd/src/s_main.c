/* Copyright (c) 1997-1999 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* IOhannes :
 * hacked the code to add advanced multidevice-support
 * 1311:forum::für::umläute:2001
 */

char pd_version[] = "Pd version 0.36 PRELIMINARY TEST 5\n";
char pd_compiletime[] = __TIME__;
char pd_compiledate[] = __DATE__;

#include "m_imp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef UNIX
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#include <windows.h>
#include <winbase.h>
#endif

void pd_init(void);
int sys_argparse(int argc, char **argv);
void sys_findprogdir(char *progname);
int sys_startgui(const char *guipath);
int sys_rcfile(void);
int m_scheduler(int nodacs);
void m_schedsetsr( void);

int sys_debuglevel;
int sys_verbose;
int sys_noloadbang;
int sys_nogui;
char *sys_guicmd;
t_symbol *sys_libdir;
static t_symbol *sys_guidir;
static t_namelist *sys_externlist;
static t_namelist *sys_openlist;
static t_namelist *sys_messagelist;

int sys_nmidiout = 1;
#ifdef NT
int sys_nmidiin = 0;
#define DEFMIDIOUTDEV 0	/* For output, in NT, default to "midi_mapper" */
#else
int sys_nmidiin = 1;
#define DEFMIDIOUTDEV 1	/* in other OSes, default to first MIDI device */
#endif
#define DEFMIDIINDEV 1	/* for NT this isn't used since sys_nmidiin is 0. */
int sys_midiindevlist[MAXMIDIINDEV] = {DEFMIDIINDEV};
int sys_midioutdevlist[MAXMIDIOUTDEV] = {DEFMIDIOUTDEV};

typedef struct _fontinfo
{
    int fi_fontsize;
    int fi_maxwidth;
    int fi_maxheight;
    int fi_hostfontsize;
    int fi_width;
    int fi_height;
} t_fontinfo;

    /* these give the nominal point size and maximum height of the characters
    in the six fonts.  */

static t_fontinfo sys_fontlist[] = {
    {8, 5, 9, 0, 0, 0}, {10, 7, 13, 0, 0, 0}, {12, 9, 16, 0, 0, 0},
    {16, 10, 20, 0, 0, 0}, {24, 15, 25, 0, 0, 0}, {36, 25, 45, 0, 0, 0}};
#define NFONT (sizeof(sys_fontlist)/sizeof(*sys_fontlist))

/* here are the actual font size structs on msp's systems:
NT:
font 8 5 9 8 5 11
font 10 7 13 10 6 13
font 12 9 16 14 8 16
font 16 10 20 16 10 18
font 24 15 25 16 10 18
font 36 25 42 36 22 41

linux:
font 8 5 9 8 5 9
font 10 7 13 12 7 13
font 12 9 16 14 9 15
font 16 10 20 16 10 19
font 24 15 25 24 15 24
font 36 25 42 36 22 41
*/

static t_fontinfo *sys_findfont(int fontsize)
{
    unsigned int i;
    t_fontinfo *fi;
    for (i = 0, fi = sys_fontlist; i < (NFONT-1); i++, fi++)
    	if (fontsize < fi[1].fi_fontsize) return (fi);
    return (sys_fontlist + (NFONT-1));
}

int sys_nearestfontsize(int fontsize)
{
    return (sys_findfont(fontsize)->fi_fontsize);
}

int sys_hostfontsize(int fontsize)
{
    return (sys_findfont(fontsize)->fi_hostfontsize);
}

int sys_fontwidth(int fontsize)
{
    return (sys_findfont(fontsize)->fi_width);
}

int sys_fontheight(int fontsize)
{
    return (sys_findfont(fontsize)->fi_height);
}

int sys_defaultfont;
#ifdef NT
#define DEFAULTFONT 12
#else
#define DEFAULTFONT 10
#endif


static int inchannels = -1, outchannels = -1;
static int srate = 44100;

/* IOhannes { */
#define MAXSOUNDINDEV 4
#define MAXSOUNDOUTDEV 4

int sys_nsoundin = -1;
int sys_nsoundout = -1;
int sys_soundindevlist[MAXSOUNDINDEV] = {-1};
int sys_soundoutdevlist[MAXSOUNDOUTDEV] = {-1};

int sys_nchin = -1;
int sys_nchout = -1;
int sys_chinlist[MAXSOUNDINDEV] = {-1};
int sys_choutlist[MAXSOUNDOUTDEV] = {-1};
/* } IOhannes */

static void openit(const char *dirname, const char *filename)
{
    char dirbuf[MAXPDSTRING], *nameptr;
    int fd = open_via_path(dirname, filename, "", dirbuf, &nameptr,
    	MAXPDSTRING, 0);
    if (fd)
    {
    	close (fd);
    	glob_evalfile(0, gensym(nameptr), gensym(dirbuf));
    }
    else
    	error("%s: can't open", filename);
}

#define NHOSTFONT 7

/* this is called from the gui process.  The first argument is the cwd, and
succeeding args give the widths and heights of known fonts.  We wait until 
these are known to open files and send messages specified on the command line.
We ask the GUI to specify the "cwd" in case we don't have a local OS to get it
from; for instance we could be some kind of RT embedded system.  However, to
really make this make sense we would have to implement
open(), read(), etc, calls to be served somehow from the GUI too. */

void glob_initfromgui(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    char *cwd = atom_getsymbolarg(0, argc, argv)->s_name;
    t_namelist *nl;
    unsigned int i, j;
    if (argc != 1 + 3 * NHOSTFONT) bug("glob_initfromgui");
    for (i = 0; i < NFONT; i++)
    {
    	int wantheight = sys_fontlist[i].fi_maxheight;
	for (j = 0; j < NHOSTFONT-1; j++)
	{
	    if (atom_getintarg(3 * (j + 1) + 3, argc, argv) > wantheight)
		    break;
	}
	    /* j is now the "real" font index for the desired font index i. */
	sys_fontlist[i].fi_hostfontsize = atom_getintarg(3 * j + 1, argc, argv);
	sys_fontlist[i].fi_width = atom_getintarg(3 * j + 2, argc, argv);
	sys_fontlist[i].fi_height = atom_getintarg(3 * j + 3, argc, argv);
    }
#if 0
    for (i = 0; i < 6; i++)
    	fprintf(stderr, "font %d %d %d %d %d\n",
	    sys_fontlist[i].fi_fontsize,
	    sys_fontlist[i].fi_maxheight,
	    sys_fontlist[i].fi_hostfontsize,
	    sys_fontlist[i].fi_width,
	    sys_fontlist[i].fi_height);
#endif
    	/* load dynamic libraries specified with "-lib" args */
    for  (nl = sys_externlist; nl; nl = nl->nl_next)
    	if (!sys_load_lib(cwd, nl->nl_string))
	    post("%s: can't load library", nl->nl_string);
    namelist_free(sys_externlist);
    sys_externlist = 0;
    	/* open patches specifies with "-open" args */
    for  (nl = sys_openlist; nl; nl = nl->nl_next)
    	openit(cwd, nl->nl_string);
    namelist_free(sys_openlist);
    sys_openlist = 0;
    	/* send messages specified with "-send" args */
    for  (nl = sys_messagelist; nl; nl = nl->nl_next)
    {
    	t_binbuf *b = binbuf_new();
	binbuf_text(b, nl->nl_string, strlen(nl->nl_string));
	binbuf_eval(b, 0, 0, 0);
	binbuf_free(b);
    }
    namelist_free(sys_messagelist);
    sys_messagelist = 0;
}

static void sys_addextrapath(void);

/* this is called from main() in s_entry.c */
int sys_main(int argc, char **argv)
{
#ifdef PD_DEBUG
    fprintf(stderr, "Pd: COMPILED FOR DEBUGGING\n");
#endif
    pd_init();    	    	    	    	/* start the message system */
    sys_findprogdir(argv[0]);	    	    	/* set sys_progname, guipath */
#ifdef __linux__
    sys_rcfile();                               /* parse the startup file */
#endif
    if (sys_argparse(argc, argv)) return (1);	/* parse cmd line */
    sys_addextrapath();
    if (sys_verbose) fprintf(stderr, "%s compiled %s %s\n",
    	pd_version, pd_compiletime, pd_compiledate);
    	    /* open audio and MIDI */
    sys_open_midi(sys_nmidiin, sys_midiindevlist,
    	sys_nmidiout, sys_midioutdevlist);
    sys_open_audio(sys_nsoundin, sys_soundindevlist, sys_nchin, sys_chinlist,
    	sys_nsoundout, sys_soundoutdevlist, sys_nchout, sys_choutlist, srate);

    	    /* tell scheduler the sample rate */ 
    m_schedsetsr();
    if (sys_startgui(sys_guidir->s_name))	/* start the gui */
    	return(1);

    	    /* run scheduler until it quits */
    return (m_scheduler(!(inchannels || outchannels)));
}

static char *(usagemessage[]) = {
"usage: pd [-flags] [file]...\n",
"\naudio configuration flags:\n",
"-r <n>           -- specify sample rate\n",
#if defined(__linux__) || defined(NT)
"-inchannels ...  -- number of audio in channels (by device, like \"2\" or \"16,8\")\n",
"-outchannels ... -- number of audio out channels (by device)\n",
#else
"-inchannels <n>  -- number of audio input channels\n",
"-outchannels <n> -- number of audio output channels\n",
#endif
"-channels ...    -- specify both input and output channels\n",
"-audiobuf <n>    -- specify size of audio buffer in msec\n",
"-blocksize <n>   -- specify audio I/O block size in sample frames\n",
"-sleepgrain <n>  -- specify number of milliseconds to sleep when idle\n",
"-nodac           -- suppress audio output\n",
"-noadc           -- suppress audio input\n",
"-noaudio         -- suppress audio input and output (-nosound is synonym) \n",
"-listdev          -- list audio and MIDI devices\n",

#ifdef __linux__
"-frags <n>       -- specify number of audio fragments (defeats audiobuf)\n",
"-fragsize <n>    -- specify log of fragment size ('blocksize' is better...)\n",
"-stream          -- use stream mode audio (e.g., for es1370 audio cards)\n",
"-32bit     	  -- allow 32 bit OSS audio transfers (for RME Hammerfall)\n",
#endif

#ifdef ALSA99
"-alsa            -- use ALSA audio drivers\n",
"-alsadev <n>     -- specify ALSA I/O device number (counting from 1)\n",
#endif

#ifdef ALSA01
"-alsa            -- use ALSA audio drivers\n",
"-alsadev <n>     -- ALSA device # (counting from 1) or name: default hw:0,0\n",
#endif

#ifdef RME_HAMMERFALL
"-rme             -- use Ritsch's RME 9652 audio driver\n",
#endif
"-audioindev ...  -- sound in device list; e.g., \"2,1\" for second and first\n",
"-audiooutdev ... -- sound out device list, same as above \n",
"-audiodev ...    -- specify both -audioindev and -audiooutdev together\n",

#ifdef NT
"-resync           -- resynchronize audio (default if more than 2 channels)\n",
"-noresync         -- never resynchronize audio I/O (default for stereo)\n",
"-asio             -- use ASIO audio driver (and not the 'MMIO' default)\n",
#endif

"\nMIDI configuration flags:\n",
"-midiindev ...   -- midi in device list; e.g., \"1,3\" for first and third\n",
"-midioutdev ...  -- midi out device list, same format\n",
"-mididev ...     -- specify -midioutdev and -midiindev together\n",
"-nomidiin        -- suppress MIDI input\n",
"-nomidiout       -- suppress MIDI output\n",
"-nomidi          -- suppress MIDI input and output\n",

"\ngeneral flags:\n",
"-path <path>     -- add to file search path\n",
"-open <file>     -- open file(s) on startup\n",
"-lib <file>      -- load object library(s)\n",
"-font <n>        -- specify default font size in points\n",
"-verbose         -- extra printout on startup and when searching for files\n",
"-d <n>           -- specify debug level\n",
"-noloadbang      -- suppress all loadbangs\n",
"-nogui           -- suppress starting the GUI\n",
"-guicmd \"cmd...\" -- substitute another GUI program (e.g., rsh)\n",
"-send \"msg...\"   -- send a message at startup (after patches are loaded)\n",
#ifdef UNIX
"-rt or -realtime -- use real-time priority (needs root privilege)\n",
#endif
};
 
static void sys_parsedevlist(int *np, int *vecp, int max, char *str)
{
    int n = 0;
    while (n < max)
    {
    	if (!*str) break;
	else
	{
    	    char *endp;
	    vecp[n] = strtol(str, &endp, 10);
	    if (endp == str)
	    	break;
	    n++;
	    if (!endp)
	    	break;
	    str = endp + 1;
	}
    }
    *np = n;
}

static int sys_getmultidevchannels(int n, int *devlist)
{
  int sum = 0;
  if (n<0)return(-1);
  if (n==0)return 0;
  while(n--)sum+=*devlist++;
  return sum;
}


    /* this routine tries to figure out where to find the auxilliary files
    Pd will need to run.  This is either done by looking at the command line
    invokation for Pd, or if htat fails, by consulting the variable
    INSTALL_PREFIX.  In NT, we don't try to use INSTALL_PREFIX. */
void sys_findprogdir(char *progname)
{
    char sbuf[MAXPDSTRING], sbuf2[MAXPDSTRING], *sp;
    char *lastslash; 
#ifdef UNIX
    struct stat statbuf;
#endif

    /* find out by what string Pd was invoked; put answer in "sbuf". */
#ifdef NT
    GetModuleFileName(NULL, sbuf2, sizeof(sbuf2));
    sbuf2[MAXPDSTRING-1] = 0;
    sys_unbashfilename(sbuf2, sbuf);
#endif /* NT */
#ifdef UNIX
    strncpy(sbuf, progname, MAXPDSTRING);
    sbuf[MAXPDSTRING-1] = 0;
#endif
    lastslash = strrchr(sbuf, '/');
    if (lastslash)
    {
    	    /* bash last slash to zero so that sbuf is directory pd was in,
	    	e.g., ~/pd/bin */
    	*lastslash = 0; 
	    /* go back to the parent from there, e.g., ~/pd */
    	lastslash = strrchr(sbuf, '/');
    	if (lastslash)
    	{
    	    strncpy(sbuf2, sbuf, lastslash-sbuf);
    	    sbuf2[lastslash-sbuf] = 0;
    	}
    	else strcpy(sbuf2, "..");
    }
    else
    {
    	    /* no slashes found.  Try INSTALL_PREFIX. */
#ifdef INSTALL_PREFIX
    	strcpy(sbuf2, INSTALL_PREFIX);
#else
    	strcpy(sbuf2, ".");
#endif
    }
    	/* now we believe sbuf2 holds the parent directory of the directory
	pd was found in.  We now want to infer the "lib" directory and the
	"gui" directory.  In "simple" UNIX installations, the layout is
	    .../bin/pd
	    .../bin/pd-gui
	    .../doc
	and in "complicated" UNIX installations, it's:
	    .../bin/pd
	    .../lib/pd/bin/pd-gui
	    .../lib/pd/doc
    	To decide which, we stat .../lib/pd; if that exists, we assume it's
	the complicated layout.  In NT, it's the "simple" layout, but
	the gui program is straight wish80:
	    .../bin/pd
	    .../bin/wish80.exe
	    .../doc
	*/
#ifdef UNIX
    strncpy(sbuf, sbuf2, MAXPDSTRING-30);
    sbuf[MAXPDSTRING-30] = 0;
    strcat(sbuf, "/lib/pd");
    if (stat(sbuf, &statbuf) >= 0)
    {
    	    /* complicated layout: lib dir is the one we just stat-ed above */
    	sys_libdir = gensym(sbuf);
	    /* gui lives in .../lib/pd/bin */
    	strncpy(sbuf, sbuf2, MAXPDSTRING-30);
    	sbuf[MAXPDSTRING-30] = 0;
    	strcat(sbuf, "/lib/pd/bin");
    	sys_guidir = gensym(sbuf);
    }
    else
    {
    	    /* simple layout: lib dir is the parent */
    	sys_libdir = gensym(sbuf2);
	    /* gui lives in .../bin */
    	strncpy(sbuf, sbuf2, MAXPDSTRING-30);
    	sbuf[MAXPDSTRING-30] = 0;
    	strcat(sbuf, "/bin");
    	sys_guidir = gensym(sbuf);
    }
#endif
#ifdef NT
    sys_libdir = gensym(sbuf2);
    sys_guidir = &s_;	/* in NT the guipath just depends on the libdir */
#endif
}

int sys_argparse(int argc, char **argv)
{
    char sbuf[MAXPDSTRING];
#ifdef NT
    int resync = -1;
#endif
    argc--; argv++;
    while ((argc > 0) && **argv == '-')
    {
    	if (!strcmp(*argv, "-r") && argc > 1 &&
    	    sscanf(argv[1], "%d", &srate) >= 1)
    	{
    	    argc -= 2;
    	    argv += 2;
    	}
    	else if (!strcmp(*argv, "-inchannels"))
	  { /* IOhannes */
	  sys_parsedevlist(&sys_nchin, sys_chinlist, MAXSOUNDINDEV, argv[1]);
	  inchannels=sys_getmultidevchannels(sys_nchin, sys_chinlist);

	  if (!sys_nchin)
	      goto usage;

	  argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-outchannels"))
	  { /* IOhannes */
	  sys_parsedevlist(&sys_nchout, sys_choutlist,MAXSOUNDOUTDEV, argv[1]);
	  outchannels=sys_getmultidevchannels(sys_nchout, sys_choutlist);

	  if (!sys_nchout)
	    goto usage;

	  argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-channels"))
	{
	    sys_parsedevlist(&sys_nchin, sys_chinlist,MAXSOUNDINDEV,
	    	argv[1]);
	    inchannels = sys_getmultidevchannels(sys_nchin, sys_chinlist);
	    sys_parsedevlist(&sys_nchout, sys_choutlist,MAXSOUNDOUTDEV,
	    	argv[1]);
	    outchannels = sys_getmultidevchannels(sys_nchout, sys_choutlist);

	    if (!sys_nchout)
	      goto usage;

	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-soundbuf") || !strcmp(*argv, "-audiobuf"))
    	{
    	    sys_audiobuf(atoi(argv[1]));
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-blocksize"))
    	{
    	    sys_setblocksize(atoi(argv[1]));
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-sleepgrain"))
    	{
    	    sys_sleepgrain = 1000 * atoi(argv[1]);
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-nodac"))
	  { /* IOhannes */
	  sys_nsoundout=0;
	  sys_nchout = 0;
	  outchannels  =0;
	  argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-noadc"))
	  { /* IOhannes */
	  sys_nsoundin=0;
	  sys_nchin = 0;
	  inchannels  =0;
	  argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-nosound") || !strcmp(*argv, "-noaudio"))
	  { /* IOhannes */
	  sys_nsoundin=sys_nsoundout = 0;
	  sys_nchin = sys_nchout = 0;
	  inchannels  =outchannels    =0;
	  argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-nomidiin"))
    	{
    	    sys_nmidiin = 0;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-nomidiout"))
    	{
    	    sys_nmidiout = 0;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-nomidi"))
    	{
    	    sys_nmidiin = sys_nmidiout = 0;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-midiindev"))
    	{
    	    sys_parsedevlist(&sys_nmidiin, sys_midiindevlist, MAXMIDIINDEV,
	    	argv[1]);
    	    if (!sys_nmidiin)
	    	goto usage;
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-midioutdev"))
    	{
    	    sys_parsedevlist(&sys_nmidiout, sys_midioutdevlist, MAXMIDIOUTDEV,
	    	argv[1]);
    	    if (!sys_nmidiout)
	    	goto usage;
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-mididev"))
    	{
    	    sys_parsedevlist(&sys_nmidiin, sys_midiindevlist, MAXMIDIINDEV,
	    	argv[1]);
    	    sys_parsedevlist(&sys_nmidiout, sys_midioutdevlist, MAXMIDIOUTDEV,
	    	argv[1]);
    	    if (!sys_nmidiout)
	    	goto usage;
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-path"))
    	{
    	    sys_addpath(argv[1]);
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-open") && argc > 1)
    	{
    	    sys_openlist = namelist_append(sys_openlist, argv[1]);
    	    argc -= 2; argv += 2;
    	}
        else if (!strcmp(*argv, "-lib") && argc > 1)
        {
    	    sys_externlist = namelist_append(sys_externlist, argv[1]);
    	    argc -= 2; argv += 2;
        }
    	else if (!strcmp(*argv, "-font") && argc > 1)
    	{
    	    sys_defaultfont = sys_nearestfontsize(atoi(argv[1]));
    	    argc -= 2;
    	    argv += 2;
    	}
    	else if (!strcmp(*argv, "-verbose"))
    	{
    	    sys_verbose = 1;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-d") && argc > 1 &&
    	    sscanf(argv[1], "%d", &sys_debuglevel) >= 1)
    	{
    	    argc -= 2;
    	    argv += 2;
    	}
    	else if (!strcmp(*argv, "-noloadbang"))
    	{
    	    sys_noloadbang = 1;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-nogui"))
    	{
    	    sys_nogui = 1;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-guicmd") && argc > 1)
    	{
    	    sys_guicmd = argv[1];
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-send") && argc > 1)
    	{
    	    sys_messagelist = namelist_append(sys_messagelist, argv[1]);
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-listdev"))
    	{
	    sys_listdevs();
    	    argc--; argv++;
    	}
#ifdef UNIX
    	else if (!strcmp(*argv, "-rt"))
    	{
    	    sys_hipriority = 1;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-realtime"))
    	{
    	    sys_hipriority = 1;
    	    argc--; argv++;
    	}
#endif
#ifdef __linux__
    	else if (!strcmp(*argv, "-frags"))
    	{
    	    linux_setfrags(atoi(argv[1]));
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-fragsize"))
    	{
	    post("pd: -fragsize argument is obsolete; use '-blocksize %d'\n",
	    	(1 << atoi(argv[1])));
    	    sys_setblocksize(1 << atoi(argv[1]));
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-stream"))
    	{
    	    linux_streammode();
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-32bit"))
    	{
    	    linux_32bit();
    	    argc--; argv++;
    	}
#ifdef ALSA01
    	else if (!strcmp(*argv, "-alsa"))
    	{
    	    linux_set_sound_api(API_ALSA);
    	    argc--; argv++;
    	}
	else if (!strcmp(*argv, "-alsadev"))
	{
	    if (argv[1][0] >= '1' && argv[1][0] <= '9')
	    {
	    	char buf[80];
		sprintf(buf, "hw:%d,0", atoi(argv[1]) - 1);
		linux_alsa_devname(buf);
	    }
	    else linux_alsa_devname(argv[1]);
    	    linux_set_sound_api(API_ALSA);
	    argc -= 2; argv +=2;
	}
#endif
#ifdef ALSA99
    	else if (!strcmp(*argv, "-alsa"))
    	{
    	    linux_set_sound_api(API_ALSA);
    	    argc--; argv++;
    	}
	else if (!strcmp(*argv, "-alsadev"))
	{
	    linux_alsa_devno(atoi(argv[1]));
    	    linux_set_sound_api(API_ALSA);
	    argc -= 2; argv +=2;
	}
#endif
#ifdef RME_HAMMERFALL
    	else if (!strcmp(*argv, "-rme"))
    	{
    	    linux_set_sound_api(API_RME);
    	    argc--; argv++;
    	}
#endif
#endif
    	else if (!strcmp(*argv, "-soundindev") ||
	    !strcmp(*argv, "-audioindev"))
	  { /* IOhannes */
	  sys_parsedevlist(&sys_nsoundin, sys_soundindevlist,
	      MAXSOUNDINDEV, argv[1]);
	  if (!sys_nsoundin)
	    goto usage;
	  argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-soundoutdev") ||
	    !strcmp(*argv, "-audiooutdev"))
	  { /* IOhannes */
	  sys_parsedevlist(&sys_nsoundout, sys_soundoutdevlist,
	      MAXSOUNDOUTDEV, argv[1]);
	  if (!sys_nsoundout)
	    goto usage;
    	    argc -= 2; argv += 2;
    	}
    	else if (!strcmp(*argv, "-sounddev") || !strcmp(*argv, "-audiodev"))
	{
	  sys_parsedevlist(&sys_nsoundin, sys_soundindevlist,
	      MAXSOUNDINDEV, argv[1]);
	  sys_parsedevlist(&sys_nsoundout, sys_soundoutdevlist,
	      MAXSOUNDOUTDEV, argv[1]);
	  if (!sys_nsoundout)
	    goto usage;
    	    argc -= 2; argv += 2;
    	}
#ifdef NT
    	else if (!strcmp(*argv, "-asio"))
	{
    	    nt_set_sound_api(API_PORTAUDIO);
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-noresync"))
	{
    	    resync = 0;
    	    argc--; argv++;
    	}
    	else if (!strcmp(*argv, "-resync"))
	{
    	    resync = 1;
    	    argc--; argv++;
    	}

#endif /* NT */
    	else
    	{
	    unsigned int i;
    	usage:
	    for (i = 0; i < sizeof(usagemessage)/sizeof(*usagemessage); i++)
	    	fprintf(stderr, "%s", usagemessage[i]);
    	    return (1);
    	}
    }
#ifdef NT
    	/* resynchronization is on by default for mulltichannel, otherwise
	    off. */
    if (resync == -1)
	resync =  (inchannels > 2 || outchannels > 2);
    if (!resync)
    	nt_noresync();
#endif
    if (!sys_defaultfont) sys_defaultfont = DEFAULTFONT;
    for (; argc > 0; argc--, argv++) 
    	sys_openlist = namelist_append(sys_openlist, *argv);


    return (0);
}

int sys_getblksize(void)
{
    return (DACBLKSIZE);
}

static void sys_addextrapath(void)
{
    char sbuf[MAXPDSTRING];
	    /* add "extra" library to path */
    strncpy(sbuf, sys_libdir->s_name, MAXPDSTRING-30);
    sbuf[MAXPDSTRING-30] = 0;
    strcat(sbuf, "/extra");
    sys_addpath(sbuf);
}

