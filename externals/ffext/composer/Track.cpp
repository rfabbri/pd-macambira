#include "Song.hpp"
#include "Track.hpp"
#include "Pattern.hpp"

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

Track::Track(Song *_song, string trackName)
: name(trackName), song(_song)
{
}

Track *Track::byName(string songName, string trackName)
{
	Song *song = Song::byName(songName);

	Track *track = song->getTrackByName(trackName);
	if(!track) track = new Track(song, trackName);

	return track;
}

void Track::print()
{
	cerr << "---- Track: " << name << " ----" << endl;

    for(map<string,Pattern *>::iterator i = patterns.begin(); i != patterns.end(); i++)
    {
		cerr << "  Pattern[" << i->first << "]: " << i->second->getName() << endl;
    }

	cerr << "---- End track (" << name << ") ----" << endl;
}

void Track::addPattern(int rows, int cols, string name)
{
    Pattern *pattern = new Pattern(rows, cols, name);
    patterns[name] = pattern;
}

Pattern *Track::getPattern(const string &p)
{
    if(patterns.find(p) != patterns.end())
        return patterns[p];
    else
        return 0;
}

void Track::renamePattern(const string &oldName, const string &newName)
{
    Pattern *pattern = getPattern(oldName);
    if(!pattern) return;
    pattern->setName(newName);
    patterns[newName] = patterns[oldName];
    patterns.erase(oldName);
}
