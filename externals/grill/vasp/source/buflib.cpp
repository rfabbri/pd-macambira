/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "buflib.h"
#include <stdio.h>

#define LIBTICK 100 // tick time in ms
#define LIBTOL 2  // how many ticks till release

#define REUSE_MAXLOSEREL 0.1  // max. fraction of lost buffer size 
#define REUSE_MAXLOSEABS 10000 // max. lost buffer size

#define LIBMAGIC 12349876L // magic number for s_thing data check


class FreeEntry
{
public:
	FreeEntry(t_symbol *s): sym(s),nxt(NULL) {}

	t_symbol *sym;
	FreeEntry *nxt;
};

class BufEntry
{
public:
	BufEntry(t_symbol *s,I fr,BL zero = true);
	~BufEntry();

	V IncRef();
	V DecRef();

//	UL magic;
	t_symbol *sym;
	I refcnt,tick;
	BufEntry *nxt;

	I alloc,len;
	S *data;
};


static BufEntry *libhead = NULL,*libtail = NULL;
static FreeEntry *freehead = NULL,*freetail = NULL;
static I libcnt = 0,libtick = 0;
static t_clock *libclk = NULL;

#ifdef FLEXT_THREADS
static flext::ThrMutex libmtx;
#endif

static V FreeLibSym(t_symbol *s);




BufEntry::BufEntry(t_symbol *s,I fr,BL zero): 
	sym(s), //magic(LIBMAGIC),
	alloc(fr),len(fr),data(new S[fr]),
	refcnt(0),nxt(NULL) 
{
	if(zero) flext::ZeroMem(data,len*sizeof(*data));
//	ASSERT(!flext_base::GetThing(sym));
//	flext_base::SetThing(sym,this);
}

BufEntry::~BufEntry()
{
	if(sym) FreeLibSym(sym);
	if(data) delete[] data;
}

V BufEntry::IncRef() { ++refcnt; }
V BufEntry::DecRef() { --refcnt; tick = libtick; }

static BufEntry *FindInLib(const t_symbol *s) 
{
	BufEntry *e;
	for(e = libhead; e && e->sym != s; e = e->nxt) (void)0;
	return e?e:NULL;
}

VBuffer *BufLib::Get(const VSymbol &s,I chn,I len,I offs)
{
	BufEntry *e = FindInLib(s.Symbol());
	if(e) 
		return new ImmBuf(e,len,offs);
	else
		return new SysBuf(s,chn,len,offs);
}

V BufLib::IncRef(t_symbol *s) 
{ 
	if(s) {
		BufEntry *e = FindInLib(s);
		if(e) e->IncRef();
	}
}

V BufLib::DecRef(t_symbol *s)
{ 
	if(s) {
		BufEntry *e = FindInLib(s);
		if(e) e->DecRef();
	}
}

static t_symbol *GetLibSym()
{
	if(freehead) {
		// reuse from free-list
		FreeEntry *r = freehead;
		freehead = r->nxt;
		if(!freehead) freetail = NULL;
		t_symbol *s = r->sym;
		delete r;
		return s;
	}
	else {
		// allocate new symbol
		char tmp[20];
	#ifdef __MWERKS__
		std::
	#endif
		sprintf(tmp,"vasp!%04i",libcnt); //! \todo what if libcnt has > 4 digits?
		libcnt++;
		return gensym(tmp);
	}
	
	clock_delay(libclk,LIBTICK);
}

static V FreeLibSym(t_symbol *sym)
{
	FreeEntry *f = new FreeEntry(sym);
	if(!freehead) freehead = f;
	else freetail->nxt = f;
	freetail = f;
}

static V LibTick(V *)
{
#ifdef FLEXT_THREADS
	libmtx.Lock();
#endif

	// collect garbage
	BufEntry *e,*p;
	for(p = NULL,e = libhead; e; ) {
		if(e->refcnt <= 0 && e->tick+LIBTOL < libtick) {
			ASSERT(e->refcnt == 0);

			BufEntry *n = e->nxt;

			if(p) p->nxt = n;
			else libhead = n;

			if(!n) libtail = p;
			else e->nxt = NULL;

			delete e;

			e = n;
		}
		else
			p = e,e = e->nxt;
	}

	++libtick;
	clock_delay(libclk,LIBTICK);

#ifdef FLEXT_THREADS
	libmtx.Unlock();
#endif
}

BufEntry *BufLib::NewImm(I fr,BL zero)
{
	if(!libclk) {
		libclk = (t_clock *)clock_new(NULL,(t_method)LibTick);
		clock_delay(libclk,LIBTICK);
	}

	t_symbol *s = NULL;
//	do {
		s = GetLibSym();
//	} while(s->s_thing);

	BufEntry *entry = new BufEntry(s,fr,zero);

#ifdef FLEXT_THREADS
	libmtx.Lock();
#endif

	if(libtail) libtail->nxt = entry; 
	else libhead = entry;
	libtail = entry;

#ifdef FLEXT_THREADS
	libmtx.Unlock();
#endif

	return entry;
}

static F reuse_maxloserel = (F)REUSE_MAXLOSEREL;
static I reuse_maxloseabs = REUSE_MAXLOSEABS;

BufEntry *BufLib::Resize(BufEntry *e,I fr,BL keep,BL zero)
{ 
	if(e->alloc >= fr && fr >= e->alloc*(1-reuse_maxloserel) && fr >= (e->alloc-reuse_maxloseabs)) {
		// reuse buffer
		e->len = fr;
	}
	else {
		S *nd = new S[fr]; 
		if(keep) {
			I l = fr;
			if(e->len < l) {
				l = e->len;
				if(zero) flext::ZeroMem(nd+l,(fr-l)*sizeof(*nd));
			}
			flext::CopyMem(nd,e->data,l*sizeof(*nd));
		}

		delete[] e->data;
		e->data = nd;
		e->len = e->alloc = fr;
	}
	return e;
}



ImmBuf::ImmBuf(I len):
	VBuffer(0,len),
	entry(BufLib::NewImm(len))
{}

ImmBuf::ImmBuf(BufEntry *e,I len,I offs): 
	VBuffer(0,len,offs),
	entry(e) 
{}

VSymbol ImmBuf::Symbol() const { return entry->sym; }

I ImmBuf::Frames() const { return entry->len; }

V ImmBuf::Frames(I fr,BL keep) { entry = BufLib::Resize(entry,fr,keep); }

S *ImmBuf::Data() { return entry->data; }



