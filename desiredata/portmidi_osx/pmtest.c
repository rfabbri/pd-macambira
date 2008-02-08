#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "portmidi.h"
#include "porttime.h"
#include "pminternal.h"

#define LATENCY 0
#define NUM_ECHOES 10

int
main()
{
    int i = 0;
    int n = 0;
    PmStream *midi_in;
    PmStream *midi_out;
    PmError err;
    char line[80];
    PmEvent buffer[NUM_ECHOES];
    int transpose;
    int delay;
    int status, data1, data2;
    int statusprefix;


    Pm_Initialize(); // xjs
    
    /* always start the timer before you start midi */
    Pt_Start(1, 0, 0); /* start a timer with millisecond accuracy */

    printf("%d midi ports found...\n", Pm_CountDevices()); // xjs
    for (i = 0; i < Pm_CountDevices(); i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        printf("%d: %s, %s", i, info->interf, info->name);
        if (info->input) printf(" (input)");
        if (info->output) printf(" (output)");
        printf("\n");
    }
    
    /* OPEN INPUT DEVICE */

    printf("Type input number: ");
    while (n != 1) {
        n = scanf("%d", &i);
        gets(line);
    }

    err = Pm_OpenInput(&midi_in, i, NULL, 100, NULL, NULL, NULL);
    if (err) {
        printf("could not open midi device: %s\n", Pm_GetErrorText(err));
        exit(1);
    }
    printf("Midi Input opened.\n");

    /* OPEN OUTPUT DEVICE */

    printf("Type output number: ");
    n = 0;
    while (n != 1) {
        n = scanf("%d", &i);
        gets(line);
    }

    err = Pm_OpenOutput(&midi_out, i, NULL, 0, NULL, NULL, LATENCY);
    if (err) {
        printf("could not open midi device: %s\n", Pm_GetErrorText(err));
        exit(1);
    }
    printf("Midi Output opened with %d ms latency.\n", LATENCY);



    /* Get input from user for parameters */
    printf("Type number of milliseconds for echoes: ");
    n = 0;
    while (n != 1) {
        n = scanf("%d", &delay);
        gets(line);
    }

    printf("Type number of semitones to transpose up: ");
    n = 0;
    while (n != 1) {
        n = scanf("%d", &transpose);
        gets(line);
    }

    printf("delay %d, tranpose %d\n", delay, transpose); // xjs


    /* loop, echoing input back transposed with multiple taps */

    printf("Press C2 on the keyboard (2 octaves below middle C) to quit.\nWaiting for MIDI input...\n");

    do {
        err = Pm_Read(midi_in, buffer, 1);
        if (err == 0) continue;           /* no bytes read. */

        /* print a hash mark for each event read. */
        printf("#");
        fflush(stdout);

        status = Pm_MessageStatus(buffer[0].message);
        data1  = Pm_MessageData1(buffer[0].message);
        data2  = Pm_MessageData2(buffer[0].message);
        statusprefix = status >> 4;

        /* ignore messages other than key-down and key-up */
        if ((statusprefix != 0x9) && (statusprefix != 0x8)) continue;

        printf("\nReceived key message = %X %X %X, at time %ld\n", status, data1, data2, buffer[0].timestamp);
        fflush(stdout);

        /* immediately send the echoes to PortMIDI (note that only the echoes are to be transposed) */
        for (i = 1; i < NUM_ECHOES; i++) {
            buffer[i].message = Pm_Message(status, data1 + transpose, data2 >> i);
            buffer[i].timestamp = buffer[0].timestamp + (i * delay);
        }
        Pm_Write(midi_out, buffer, NUM_ECHOES);
    } while (data1 != 36); /* quit when C2 is pressed */

    printf("Key C2 pressed.  Exiting...\n");
    fflush(stdout);

    Pt_Stop(); // xjs

    /* Give the echoes time to finish before quitting. */
    sleep(((NUM_ECHOES * delay) / 1000) + 1);

    Pm_Close(midi_in);
    Pm_Close(midi_out);

    Pm_Terminate(); // xjs

    printf("Done.\n");
    return 0;
}


    
