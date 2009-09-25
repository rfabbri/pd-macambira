#ifndef COMPOSER_PDCLASSES_H_INCLUDED
#define COMPOSER_PDCLASSES_H_INCLUDED

#include <m_pd.h>

#define MAX_RESULT_SIZE 128

#define TRACK_SELECTOR "#composer::track"
#define SONG_SELECTOR "#composer::song"

class Track;
class Song;

typedef struct _track_proxy
{
    t_object x_obj;
    t_outlet *outlet;
    Track *track;
    t_int editor_open;
    t_symbol *editor_recv;
    t_symbol *editor_send;
} t_track_proxy;

void track_proxy_setup(void);
static t_track_proxy *track_proxy_new(t_symbol *song_name, t_symbol *track_name);
static void track_proxy_free(t_track_proxy *x);
static void track_proxy_save(t_gobj *z, t_binbuf *b);
static void track_proxy_send_result(t_track_proxy *x, int outlet, int editor);
static int track_proxy_getpatterns(t_track_proxy *x);
static int track_proxy_getpatternsize(t_track_proxy *x, t_floatarg pat);
static int track_proxy_setrow(t_track_proxy *x, t_symbol *sel, int argc, t_atom *argv);
static int track_proxy_getrow(t_track_proxy *x, t_floatarg pat, t_floatarg rownum);
static int track_proxy_addpattern(t_track_proxy *x, t_symbol *name, t_floatarg rows, t_floatarg cols);
static int track_proxy_removepattern(t_track_proxy *x, t_floatarg pat);
static int track_proxy_resizepattern(t_track_proxy *x, t_floatarg pat, t_floatarg rows, t_floatarg cols);
static int track_proxy_copypattern(t_track_proxy *x, t_symbol *src, t_symbol *dst);

extern "C" void composer_setup(void);

#endif // COMPOSER_PDCLASSES_H_INCLUDED
