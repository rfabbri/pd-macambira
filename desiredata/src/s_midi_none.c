/* This is for compiling pd without any midi support. by matju, 2006.11.21 */

#include "desire.h"

void sys_do_open_midi(int nmidiin, int *midiinvec, int nmidiout, int *midioutvec) {}
void sys_close_midi(void) {}
void sys_putmidimess(int portno, int a, int b, int c) {}
void sys_putmidibyte(int portno, int byte) {}
void sys_poll_midi(void) {}
void midi_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int maxndev, int devdescsize)
{
    *nindevs = 0;
    *noutdevs = 0;
}
