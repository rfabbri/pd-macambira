/*
 * Portable timer implementation for Darwin / MacOS X
 *
 * Jon Parise <jparise@cmu.edu>
 *
 * $Id: ptdarwin.c,v 1.8 2005-05-18 04:28:50 millerpuckette Exp $
 */

#include <stdio.h>
#include <sys/time.h>
#include "porttime.h"

#define TRUE 1
#define FALSE 0

static int time_started_flag = FALSE;
static struct timeval time_offset;

PtError Pt_Start(int resolution, PtCallback *callback, void *userData)
{
    struct timezone tz;

    if (callback) printf("error in porttime: callbacks not implemented\n");
    time_started_flag = TRUE;
    gettimeofday(&time_offset, &tz);

    return ptNoError;
}


PtError Pt_Stop(void) // xjs added void
{
    time_started_flag = FALSE;
    return ptNoError;
}


int Pt_Started(void) // xjs added void
{
    return time_started_flag;
}


PtTimestamp Pt_Time(void *time_info) // xjs added void *time_info
{
    long seconds, milliseconds;
    struct timeval now;
    struct timezone tz;

    gettimeofday(&now, &tz);
    seconds = now.tv_sec - time_offset.tv_sec;
    milliseconds = (now.tv_usec - time_offset.tv_usec) / 1000;

    return (seconds * 1000 + milliseconds);
}



