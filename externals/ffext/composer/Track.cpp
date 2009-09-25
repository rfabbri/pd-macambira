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

	for(unsigned int i = 0; i < patterns.size(); i++)
	{
		cerr << "  Pattern[" << patterns[i]->getName() << "]: " << patterns[i]->getName() << endl;
	}

	cerr << "---- End track (" << name << ") ----" << endl;
}

void Track::addPattern(int rows, int cols, string name)
{
    Pattern *pattern = new Pattern(rows, cols, name);
	patterns.push_back(pattern);
}

Pattern *Track::getPattern(int n)
{
	return patterns[n];
}
