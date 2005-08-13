#include "Input.h"
#include <iostream.h>


Input::Input ()
{
	fd = 0;
	format = -1;
	verbosity = 1;
	recover=false;
}

Input::~Input ()
{
}

int
Input::Open (const char *pathname)
{
	return -1;
}

int
Input::Close ()
{
	return -1;
}

int
Input::Read (void *buf, unsigned int count)
{
	return -1;
}

long
Input::SeekSet (long offset)
{
	return -1;
}

long
Input::SeekCur (long offset)
{
	return -1;
}

long
Input::SeekEnd (long offset)
{
	return -1;
}
float
Input::get_cachesize ()
{
	return 0.0;
}
