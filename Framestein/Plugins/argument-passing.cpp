//
// example of parsing arguments in a plugin using the arguments-class (defined in pixels.h)
//

#include "pixels.h"

void perform_effect(_frame f, _args a)
{
	arguments ar(a.s);

	//
	// ar.count()	return the number of arguments
	// ar[i]	return i'th argument, starting from zero
	//

	for(int i=0; i<ar.count(); i++)
		printf("argument %d: %s\n", i, ar[i]);
}
