//
// some small helpers
//
// written by olaf.matthes@gmx.de
//

// scale an input f that goes from il to ih to go from ol to oh
__inline long scl(long f, long il, long ih, long ol, long oh)
{
	long or = abs(oh-ol);
	long ir = abs(ih-il);
	float ratio = or/ir;
	long steps = f-il;
	return(steps*ratio+ol);
}
