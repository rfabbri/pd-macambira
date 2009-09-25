#include "Editor.hpp"
#include "Track.hpp"
#include "Song.hpp"

#include <iostream>
#include <string>

#include <m_pd.h>

#include "editor_tk.cpp"
static const int editor_tk_len = sizeof(editor_tk) / sizeof(editor_tk[0]);

using std::cerr;
using std::endl;
using std::string;

static char hexnibble(unsigned int i)
{
    i &= 0xf;
    return (i < 0xa) ? ('0' + i) : ('a' + i - 0xa);
}

static const char* urlencode(char c)
{
    static char buf[4];
    buf[0] = '%';
    buf[1] = hexnibble((c & 0xf0) >> 4);
    buf[2] = hexnibble(c & 0xf);
    buf[3] = '\0';
    return &buf[0];
}

void Editor::uploadCode()
{
    sys_gui("proc Xeval {c {d {}}} {switch $c {begin {set ::Xeval_map {}; for {set i 0} {$i < 256} {incr i} {lappend ::Xeval_map \%[format \%02x $i] [format \%c $i]}; set ::Xeval_data {}} data {append ::Xeval_data [string map $::Xeval_map $d]\\n} end {uplevel #0 $::Xeval_data; unset ::Xeval_map; unset ::Xeval_data}}}\n"); 
    sys_gui("Xeval begin\n");
    for(int i = 0; i < editor_tk_len; i++)
    {
        string s = "Xeval data {";
        string l = editor_tk[i];
        for(int j = 0; j < l.length(); j++)
        {
            if(isalnum(l[j])) s.append(1, l[j]);
            else s += string(urlencode(l[j]));
        }
        s += "}\n";
        sys_gui(const_cast<char*>(s.c_str()));
    }
    sys_gui("Xeval end\n");
}

void Editor::init(t_track_proxy *x)
{
    uploadCode();
    sys_vgui("pd::composer::init %s %s %s %d %s %d\n",
        x->editor_recv->s_name,
        x->track->getSong()->getName().c_str(),
        x->track->getName().c_str(),
        16,
        "NULL",
        1);
}

void Editor::openWindow(t_track_proxy *x)
{
    x->editor_open = 1;
    sys_vgui("pd::composer::openWindow %s\n", x->editor_recv->s_name);
}

void Editor::closeWindow(t_track_proxy *x)
{
    x->editor_open = 0;
    sys_vgui("pd::composer::closeWindow %s\n", x->editor_recv->s_name);
}
