/*
	One dimensional cellular automata, where each iteration
	adds a new row.

	Each cell has three neighbourns, top left, top and top right,
	as illustrated below:

	123
	 O

	Patterns that lead to a white cell are given as parameter,
	forming the rule of the automata.

	Example: parameter "001-010-011-100"

	Starting from a singe cell, this would lead to:

		O
	       OOO
	      OO  O
	     OO OOOO
	    OO  O   O
	   OO OOOO OOO
	  OO  O    O  O

	etc.
*/

#include <iostream>
#include <string>
#include <vector>

#include "plugin.h"
#include "pixels.h"

using namespace std;

vector<string> sv;

void iterate(const char *arg);
void draw(const _frame &f);

void perform_effect(_frame f, _args a)
{
	arguments ar(a.s);
	if(!ar.count()) {
		printf("Usage: rowca <rule of automata, or command>\n");
		printf("Example rule: 001-010-011-100\n");
		printf("Commands: draw, clear\n");
		return;
	}

	char *t = ar[0];

	if(strstr(t, "0") || strstr(t, "1")) iterate(t); else
	if(strcmp(t, "draw")==0) draw(f); else
	if(strcmp(t, "clear")==0) sv.clear();
}

void iterate(const char *arg)
{
	if(sv.empty())
	{
		sv.push_back("00100");	// start with a single cell
	}

	string s("00"), last=sv.back(), rule(arg);
	int i;

	for(i=0; i<=last.size()-3; i++)
	{
		if( rule.find(last.substr(i, 3)) != rule.npos )
			s.append("1");
		else
			s.append("0");
	}

	s.append("00");

	sv.push_back(s);
}

void draw(const _frame &f)
{
	if(sv.empty()) return;

	int x1=0, y1=0, x2=sv.back().size(), y2=sv.size();
	int i;

	pixels p(f);
	int prevy=-1;

	float sourcex, sourcey;
	string s;

	while(!p.eof())
	{
		if(p.y != prevy)
		{
			sourcey = p.y / (float)p.height();
			s = sv[sourcey * y2];

			// make s the size of x2
			i = (x2-s.size()) / 2;
			s.insert(1, i, '0');
			s.append(i, '0');

			prevy = p.y;
		}

		sourcex = p.x / (float)p.width();

		if( !s.compare(sourcex * x2, 1, "1") )
			p.putrgb(255, 255, 255);
		else
			p.putrgb(0, 0, 0);

		p.next();
	}
	cout << "rows: " << y2 << " size of last row: " << x2 << endl;
}
