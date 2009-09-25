#ifndef COMPOSER_TRACK_H_INCLUDED
#define COMPOSER_TRACK_H_INCLUDED

#include <string>
#include <vector>

#include <m_pd.h>

using std::string;
using std::vector;

class Song;
class Pattern;

class Track
{
public:
	static Track *byName(string songName, string trackName);
private:
	string name;
	vector<Pattern *> patterns;
	Song *song;
protected:
	Track(Song *_song, string trackName);
public:
	void print();
	void addPattern(int rows, int cols, string name);
	Pattern *getPattern(int n);
	inline unsigned int getPatternCount() {return patterns.size();}
	inline Song *getSong() {return song;}
	inline const string &getName() {return name;}
};

#endif // COMPOSER_TRACK_H_INCLUDED
