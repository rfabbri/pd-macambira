/* porttime.h -- portable interface to millisecond timer
 *
 * 27Jun02 XJS - altered type of Pt_Time() (in porttime.h & portmidi.c) so it matches PmTimeProcPtr
 */

/* Should there be a way to choose the source of time here? */

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    ptNoError = 0,
    ptHostError = -10000,
    ptAlreadyStarted,
    ptAlreadyStopped
} PtError;


typedef long PtTimestamp;

typedef int (PtCallback)( PtTimestamp timestamp, void *userData );


PtError Pt_Start(int resolution, PtCallback *callback, void *userData);
PtError Pt_Stop(void); // xjs, added void
int Pt_Started(void);  // xjs, added void
PtTimestamp Pt_Time(void *time_info);  /* xjs - added void *time_info so this f() is a PmTimeProcPtr, defined in portmidi.h */

#ifdef __cplusplus
}
#endif
