#ifndef COMPOSER_TRACK_H_INCLUDED
#define COMPOSER_TRACK_H_INCLUDED

#include <map>
#include <string>
//#include <vector>

#include <m_pd.h>

using std::string;
//using std::vector;
using std::map;

class Song;
class Pattern;

class Track
{
public:
	static Track *byName(string songName, string trackName);
private:
	string name;
	//vector<Pattern *> patterns;
    map<string,Pattern *> patterns;
	Song *song;
protected:
	Track(Song *_song, string trackName);
public:
	void print();
	void addPattern(int rows, int cols, string name);
	//Pattern *getPattern(int n);
    Pattern *getPattern(const string &p);
	inline unsigned int getPatternCount() {return patterns.size();}
    inline map<string,Pattern *>::iterator patternsBegin() {return patterns.begin();}
    inline map<string,Pattern *>::iterator patternsEnd() {return patterns.end();}
	inline Song *getSong() {return song;}
	inline const string &getName() {return name;}
};

#endif // COMPOSER_TRACK_H_INCLUDED
