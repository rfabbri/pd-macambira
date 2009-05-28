/* Copyright (c) 1997- Guenter Geiger, Miller Puckette, Larry Troxler,
* Winfried Ritsch, Karl MacMillan, and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */


typedef int16_t t_alsa_sample16;
typedef int32_t t_alsa_sample32;
#define ALSA_SAMPLEWIDTH_16 sizeof(t_alsa_sample16)
#define ALSA_SAMPLEWIDTH_32 sizeof(t_alsa_sample32)
#define ALSA_XFERSIZE16  (signed int)(sizeof(t_alsa_sample16) * sys_dacblocksize)
#define ALSA_XFERSIZE32  (signed int)(sizeof(t_alsa_sample32) * sys_dacblocksize)
#define ALSA_MAXDEV 4
#define ALSA_JITTER 1024
#define ALSA_EXTRABUFFER 2048
#define ALSA_DEFFRAGSIZE 64
#define ALSA_DEFNFRAG 12

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif

struct t_alsa_dev {
    snd_pcm_t *a_handle;
    int a_devno;
    int a_sampwidth;
    int a_channels;
    char **a_addr;
    int a_synced; 
};

struct t_alsa {
    t_alsa_dev dev[ALSA_MAXDEV];
    int ndev;
};
extern t_alsa alsai, alsao;

int alsamm_open_audio(int rate);
void alsamm_close_audio(void);
int alsamm_send_dacs(void);
