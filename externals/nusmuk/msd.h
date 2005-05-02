

/* 
 msd - mass spring damper model for Pure Data or Max/MSP

 Copyright (C) 2005  Nicolas Montgermont
 Written by Nicolas Montgermont for a Master's train in Acoustic,
 Signal processing and Computing Applied to Music (ATIAM, Paris 6) 
 at La Kitchen supervised by Cyrille Henry.

 Based on Pure Data by Miller Puckette and others
 Use FLEXT C++ Layer by Thomas Grill (xovo@gmx.net)
 Based on pmpd by Cyrille Henry 


 Contact : Nicolas Montgermont, montgermont@la-kitchen.fr
	   Cyrille Henry, Cyrille.Henry@la-kitchen.fr

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 Version 0.05 -- 28.04.2005
*/

// include flext header
#include <flext.h>
#include <flmap.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <map>

// define constants
#define MSD_VERSION  0.05


// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#ifdef _MSC_VER
#define NEWARR(type,var,size) type *var = new type[size]
#define DELARR(var) delete[] var
#else
#define NEWARR(type,var,size) type var[size]
#define DELARR(var) ((void)0)
#endif


inline t_float sqr(t_float x) { return x*x; }


template<int N> class Link;

template<int N> 
class LinkList
	: public std::vector<Link<N> *>
{
public:
	void insert(Link<N> *l) 
	{
		for(typename LinkList::iterator it = begin(); it != end(); ++it)
			if(*it == l) return;
		// not found -> add
		push_back(l);
	}
	
	void erase(Link<N> *l)
	{
		for(typename LinkList::iterator it = begin(); it != end(); ++it)
			if(*it == l) { 
				// found
				std::vector<Link<N> *>::erase(it); 
				return; 
			}
	}
};

template<int N>
class Mass {
public:
	t_int nbr;
	const t_symbol *Id;
	bool mobile;
	t_float invM;
	t_float speed[N];
	t_float pos[N];
	t_float pos2[N];
	t_float force[N];
	t_float out_force[N];
	LinkList<N> links;
	
	Mass(t_int n,const t_symbol *id,bool mob,t_float m,t_float p[N])
		: nbr(n),Id(id)
		, mobile(mob)
		, invM(m?1.f/m:1)
	{
		for(int i = 0; i < N; ++i) {
			pos[i] = pos2[i] = p[i];
			force[i] = speed[i] = 0;
		}
	}

	inline void setForce(int n,t_float f) { force[n] = f; }
	
	inline void setForce(t_float f[N]) 
	{ 
		for(int i = 0; i < N; ++i) setForce(i,f[i]); 
	}

	inline void setPos(int n,t_float p) { pos[n] = pos2[n] = p; }
	
	inline void setPos(t_float p[N]) 
	{ 
		for(int i = 0; i < N; ++i) setPos(i,p[i]);
	}
	
	inline void compute(t_float limit[N][2])
	{
		// compute new masses position only if mobile = 1
		if(mobile)  {
			for(int i = 0; i < N; ++i) {
				t_float pold = pos[i];
				t_float pnew = force[i] * invM + 2*pold - pos2[i]; // x[n] =Fx[n]/M+2x[n]-x[n-1]
				if(pnew < limit[i][0]) pnew = limit[i][0]; else if(pnew > limit[i][1]) pnew = limit[i][1];
				speed[i] = (pos[i] = pnew) - (pos2[i] = pold);	// x[n-2] = x[n-1], x[n-1] = x[n],vx[n] = x[n] - x[n-1]
			}
		}
		// clear forces
		for(int i = 0; i < N; ++i) {
			out_force[i] = force[i];
			force[i] = 0;						// Fx[n] = 0
		}
	}
};

template<int N>
class Link {
public:
	t_int nbr;
	const t_symbol *Id;
	Mass<N> *mass1,*mass2;
	t_float K1, D1, D2;
	t_float longueur, long_min, long_max;
	t_float distance_old;
	
	inline t_float compdist() const 
	{
		const Mass<N> *m1 = mass1,*m2 = mass2; // cache locally
		t_float distance;
		if(N == 1) 
			distance = fabs(m1->pos[0]-m2->pos[0]);		// L[n] = |x1 - x2|
		else {
			distance = 0;
			for(int i = 0; i < N; ++i) distance += sqr(m1->pos[i]-m2->pos[i]);
			distance = sqrt(distance);
		}
		return distance;
	}

	Link(t_int n,const t_symbol *id,Mass<N> *m1,Mass<N> *m2,t_float k1,t_float d1,t_float d2,t_float lmin,t_float lmax)
		: nbr(n),Id(id)
		, mass1(m1),mass2(m2)
		, K1(k1),D1(d1),D2(d2)
		, long_min(lmin),long_max(lmax)
	{
		distance_old = longueur = compdist(); // L[n-1]

		mass1->links.insert(this);
		mass2->links.insert(this);
	}
	
	~Link()
	{
		mass1->links.erase(this);
		mass2->links.erase(this);
	}
	
	// compute link forces
	inline void compute() 
	{
		Mass<N> *m1 = mass1,*m2 = mass2; // cache locally
		t_float distance = compdist();
		
		if (distance < long_min || distance > long_max || distance == 0) {
			for(int i = 0; i < N; ++i) {
				m1->force[i] -= D2 * m1->speed[i]; 	//  Fx1[n] = -Fx, Fx1[n] = Fx1[n] - D2 * vx1[n-1]
				m2->force[i] += D2 * m2->speed[i]; 	// Fx2[n] = Fx, Fx2[n] = Fx2[n] - D2 * vx2[n-1]
			}
		}
		else {								// Lmin < L < Lmax
			const t_float F  = (K1 * (distance - longueur) + D1 * (distance - distance_old))/distance ;		// F[n] = k1 (L[n] - L[0])/L[n] + D1 (L[n] - L[n-1])/L[n]
			for(int i = 0; i < N; ++i) {
				const t_float Fn = F * (m1->pos[i] - m2->pos[i]); // Fx = F * Lx[n]/L[n]
				m1->force[i] -= Fn + D2 * m1->speed[i]; 	//  Fx1[n] = -Fx, Fx1[n] = Fx1[n] - D2 * vx1[n-1]
				m2->force[i] += Fn - D2 * m2->speed[i]; 	// Fx2[n] = Fx, Fx2[n] = Fx2[n] - D2 * vx2[n-1]
			}
		}
		
		distance_old = distance;				// L[n-1] = L[n]			
	}
};


template <typename T>
class IndexMap
	: public TablePtrMap<int,T,16>
{
public:
	typedef TablePtrMap<int,T,16> Parent;
	
    virtual ~IndexMap() { reset(); }

	void reset() 
	{ 
		// delete all associated items
		for(typename Parent::iterator it(*this); it; ++it) delete it.data();
		Parent::clear(); 
	}

};

template <typename T>
class IDMap
	: std::map<const t_symbol *,TablePtrMap<T,T,4> *>
{
public:
	// that's the container holding the data items (masses, links) of one ID
	typedef TablePtrMap<T,T,4> Container;
	// that's the map for the key ID (symbol,int) relating to the data items
	typedef std::map<const t_symbol *,Container *> Parent;

	typedef typename Container::iterator iterator;

	IDMap() {}
	
	virtual ~IDMap() { reset(); }
	
	void reset() 
	{
        typename Parent::iterator it;
        for(it = Parent::begin(); it != Parent::end(); ++it) 
            delete it->second;
		Parent::clear();
	}
	
	void insert(T item)
	{
		typename Parent::iterator it = Parent::find(item->Id);
        Container *c;
        if(it == Parent::end())
            Parent::operator[](item->Id) = c = new Container;
        else
            c = it->second;
        c->insert(item,item);
	}
	
	iterator find(const t_symbol *key)
	{
		typename Parent::iterator it = Parent::find(key);
		if(it == Parent::end())
			return iterator();
        else {
            Container *c = it->second;
			return iterator(*c);
        }
	}
	
	void erase(T item)
	{
		typename Parent::iterator it = Parent::find(item->Id);
		if(it != Parent::end()) it->second->remove(item);
	}
};


template<int N>
class msdN:
	public flext_base
{
	FLEXT_HEADER_S(msdN,flext_base,setup)	//class with setup
 
public:
	// constructor with no arguments
	msdN(int argc,t_atom *argv)
		: id_mass(0),id_link(0)
	{
		for(int i = 0; i < N; ++i) limit[i][0] = -1.e10,limit[i][1] = 1.e10;
	
		// --- define inlets and outlets ---
		AddInAnything("bang, reset, etc."); 	// default inlet
		AddOutAnything("infos on masses");	// outlet for integer count
		AddOutAnything("control");		// outlet for bang
	}

	virtual ~msdN() { clear(); }

protected:

// --------------------------------------------------------------  PROTECTED VARIABLES 
// -----------------------------------------------------------------------------------

	typedef Mass<N> t_mass;
	typedef Link<N> t_link;

	IndexMap<t_link *> link;		// links	
	IDMap<t_link *> linkids;		// links by name
	IndexMap<t_mass *> mass;		// masses
	IDMap<t_mass *> massids;		// masses by name
	
	t_float limit[N][2];			// Limit values
	int id_mass, id_link;

// ---------------------------------------------------------------  RESET 
// ----------------------------------------------------------------------
	void m_reset() 
	{ 
		clear();
		ToOutAnything(1,S_Reset,0,NULL);
	}

// --------------------------------------------------------------  COMPUTE 
// -----------------------------------------------------------------------

	void m_bang()
	{
		// update all links
		for (typename IndexMap<t_link *>::iterator lit(link); lit; ++lit) lit.data()->compute();

		// update all masses
		for (typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit) mit.data()->compute(limit);
	}

// --------------------------------------------------------------  MASSES
// ----------------------------------------------------------------------

	// add a mass
	// Id, nbr, mobile, invM, speedX, posX, forceX
	void m_mass(int argc,t_atom *argv) 
	{
		if(argc != 3+N) {
			error("mass : Id mobile mass X");
			return;
		}
		
		t_float pos[N];
		for(int i = 0; i < N; ++i) pos[i] = GetAFloat(argv[3+i]);

		t_mass *m = new t_mass(
			id_mass, // index
			GetSymbol(argv[0]), // ID
			GetABool(argv[1]), // mobile
			GetAFloat(argv[2]), // mass
			pos // pos
		);
		
		outmass(S_Mass,m);

		massids.insert(m);
		mass.insert(id_mass++,m);
	}

	// add a force to mass(es) named Id or No
	void m_force(int argc,t_atom *argv,int n) 
	{
		if(argc != 2) {
			error("%s - %s Syntax : Id/Nomass value",thisName(),GetString(thisTag()));
			return;
		}

		const t_float f = GetAFloat(argv[1]);

		if(IsSymbol(argv[0])) {
			typename IDMap<t_mass *>::iterator it;
            for(it = massids.find(GetSymbol(argv[0])); it; ++it) {
                t_mass *m = it.data();
				m->setForce(n,f);
            }
		}
		else {
			t_mass *m = mass.find(GetAInt(argv[0]));
			if(m) 
				m->setForce(n,f);
			else
				error("%s - %s : Index not found",thisName(),GetString(thisTag()));
		}
	}

	inline void m_forceX(int argc,t_atom *argv) { m_force(argc,argv,0); }
	inline void m_forceY(int argc,t_atom *argv) { m_force(argc,argv,1); }
	inline void m_forceZ(int argc,t_atom *argv) { m_force(argc,argv,2); }

	// displace mass(es) named Id or No to a certain position
	void m_pos(int argc,t_atom *argv,int n) 
	{
		if(argc != 2) {
			error("%s - %s Syntax : Id/Nomass value",thisName(),GetString(thisTag()));
			return;
		}

		const t_float p = GetAFloat(argv[1]);
		if(p > limit[n][1] || p < limit[n][0])  return;
		
		if(IsSymbol(argv[0])) {
			typename IDMap<t_mass *>::iterator it;
			for(it = massids.find(GetSymbol(argv[0])); it; ++it)
				it.data()->setPos(n,p);
		}
		else {
			t_mass *m = mass.find(GetAInt(argv[0]));
			if(m) 
				m->setPos(n,p);
			else
				error("%s - %s : Index not found",thisName(),GetString(thisTag()));
		}
	}

	inline void m_posX(int argc,t_atom *argv) { m_pos(argc,argv,0); }
	inline void m_posY(int argc,t_atom *argv) { m_pos(argc,argv,1); }
	inline void m_posZ(int argc,t_atom *argv) { m_pos(argc,argv,2); }

	// set mass No to mobile
	void m_set_mobile(int argc,t_atom *argv,bool mob = true) 
	{
		if (argc != 1) {
			error("%s - %s Syntax : Idmass",thisName(),GetString(thisTag()));
			return;
		}
	
		t_mass *m = mass.find(GetAInt(argv[0]));
		if(m) 
			m->mobile = mob;
		else
			error("%s - %s : Index not found",thisName(),GetString(thisTag()));
	}

	// set mass No to fixed
	inline void m_set_fixe(int argc,t_atom *argv) { m_set_mobile(argc,argv,false); }

	// Delete mass
	void m_delete_mass(int argc,t_atom *argv) 
	{
		if (argc != 1) {
			error("%s - %s Syntax : Nomass",thisName(),GetString(thisTag()));
			return;
		}
		
		t_mass *m = mass.remove(GetAInt(argv[0]));
		if(m) {
			// Delete all associated links 
			for(typename std::vector<t_link *>::iterator it = m->links.begin(); it != m->links.end(); ++it)
				deletelink(*it);
			outmass(S_Mass_deleted,m);
			massids.erase(m);
			mass.remove(m->nbr);
			delete m;
		}
		else
			error("%s - %s : Index not found",thisName(),GetString(thisTag()));
	}


	// set X,Y,Z min/max
	void m_limit(int argc,t_atom *argv,int n,int i) 
	{
		if (argc != 1)
			error("%s - %s Syntax : Value",thisName(),GetString(thisTag()));
		else
			limit[n][i] = GetAFloat(argv[0]);
	}

	inline void m_Xmin(int argc,t_atom *argv) { m_limit(argc,argv,0,0); }
	inline void m_Ymin(int argc,t_atom *argv) { m_limit(argc,argv,1,0); }
	inline void m_Zmin(int argc,t_atom *argv) { m_limit(argc,argv,2,0); }

	inline void m_Xmax(int argc,t_atom *argv) { m_limit(argc,argv,0,1); }
	inline void m_Ymax(int argc,t_atom *argv) { m_limit(argc,argv,1,1); }
	inline void m_Zmax(int argc,t_atom *argv) { m_limit(argc,argv,2,1); }

// --------------------------------------------------------------  LINKS 
// ---------------------------------------------------------------------

	// add a link
	// Id, *mass1, *mass2, K1, D1, D2, (Lmin,Lmax)
	void m_link(int argc,t_atom *argv) 
	{
		if (argc < 6 || argc > 8) {
			error("%s - %s Syntax : Id Nomass1 Nomass2 K D1 D2 (Lmin Lmax)",thisName(),GetString(thisTag()));
			return;
		}
		
		t_mass *mass1 = mass.find(GetAInt(argv[1]));
		t_mass *mass2 = mass.find(GetAInt(argv[2]));
 
      	if(!mass1 || !mass2) {
			error("%s - %s : Index not found",thisName(),GetString(thisTag()));
			return;
		}

		t_link *l = new t_link(
			id_link,
			GetSymbol(argv[0]), // ID
			mass1,mass2, // pointer to mass1, mass2
			GetAFloat(argv[3]), // K1
			GetAFloat(argv[4]), // D1
			GetAFloat(argv[5]), // D2
			argc >= 7?GetFloat(argv[6]):0,
			argc >= 8?GetFloat(argv[7]):32768
		);

		linkids.insert(l);
		link.insert(id_link++,l);
		outlink(S_Link,l);
	}

	// add interactor link
	// Id, Id masses1, Id masses2, K1, D1, D2, (Lmin, Lmax)
	void m_ilink(int argc,t_atom *argv) 
	{
		if (argc < 6 || argc > 8) {
			error("%s - %s Syntax : Id Idmass1 Idmass2 K D1 D2 (Lmin Lmax)",thisName(),GetString(thisTag()));
			return;
		}

		typename IDMap<t_mass *>::iterator it1,it2,it;
		it1 = massids.find(GetSymbol(argv[1]));
		it2 = massids.find(GetSymbol(argv[2]));

		for(; it1; ++it1) {
			for(it = it2; it; ++it) {
				t_link *l = new t_link(
					id_link,
					GetSymbol(argv[0]), // ID
					it1.data(),it.data(), // pointer to mass1, mass2
					GetAFloat(argv[3]), // K1
					GetAFloat(argv[4]), // D1
					GetAFloat(argv[5]), // D2
					argc >= 7?GetFloat(argv[6]):0,
					argc >= 8?GetFloat(argv[7]):32768
				);

				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_Link,l);
			}
		}
	}

	// set rigidity of link(s) named Id
	void m_setK(int argc,t_atom *argv) 
	{
		if (argc != 2) {
			error("%s - %s Syntax : IdLink Value",thisName(),GetString(thisTag()));
			return;
		}

		t_float k1 = GetAFloat(argv[1]);
		typename IDMap<t_link *>::iterator it;
		for(it = linkids.find(GetSymbol(argv[0])); it; ++it)
			it.data()->K1 = k1;
	}

	// set damping of link(s) named Id
	void m_setD(int argc,t_atom *argv) 
	{
		if (argc != 2) {
			error("%s - %s Syntax : IdLink Value",thisName(),GetString(thisTag()));
			return;
		}

		t_float d1 = GetAFloat(argv[1]);
		typename IDMap<t_link *>::iterator it;
		for(it = linkids.find(GetSymbol(argv[0])); it; ++it)
			it.data()->D1 = d1;
	}

	// set damping of link(s) named Id
	void m_setD2(int argc,t_atom *argv) 
	{
		if (argc != 2) {
			error("%s - %s Syntax : IdLink Value",thisName(),GetString(thisTag()));
			return;
		}

		t_float d2 = GetAFloat(argv[1]);
		typename IDMap<t_link *>::iterator it;
		for(it = linkids.find(GetSymbol(argv[0])); it; ++it)
			it.data()->D2 = d2;
	}

	// Delete link
	void m_delete_link(int argc,t_atom *argv) 
	{
		if (argc != 1) {
			error("%s - %s Syntax : NoLink",thisName(),GetString(thisTag()));
			return;
		}
		
		t_link *l = link.find(GetAInt(argv[0]));
		if(l)
			deletelink(l);
		else {
			error("%s - %s : Index not found",thisName(),GetString(thisTag()));
			return;
		}
	}


// --------------------------------------------------------------  GET 
// -------------------------------------------------------------------

	// get attributes
	void m_get(int argc,t_atom *argv)
	{
		if(argc == 0) {
			return;
		}
	
		t_atom sortie[1+2*N];
		const t_symbol *auxtype = GetSymbol(argv[0]);

		if (argc == 1) {
			if (auxtype == S_massesPos)	{	// get all masses positions
				for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit) {	
					SetInt(sortie[0],mit.data()->nbr);
					for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],mit.data()->pos[i]);
					ToOutAnything(0,S_massesPos,1+N,sortie);
				}
			}
			else if (auxtype == S_massesForces)	{ // get all masses forces
				for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit) {	
					SetInt(sortie[0],mit.data()->nbr);	
					for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],mit.data()->out_force[i]);
					ToOutAnything(0,S_massesForces,1+N,sortie);
				}
			}
			else if (auxtype == S_linksPos) {		// get all links positions
				for(typename IndexMap<t_link *>::iterator lit(link); lit; ++lit) {	
					SetInt(sortie[0],lit.data()->nbr);	
					for(int i = 0; i < N; ++i) {
						SetFloat(sortie[1+i],lit.data()->mass1->pos[i]);
						SetFloat(sortie[1+N+i],lit.data()->mass2->pos[i]);
					}
					ToOutAnything(0,S_linksPos,1+2*N,sortie);
				}
			}
			else {					// get all masses speeds
				for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit) {	
					SetInt(sortie[0],mit.data()->nbr);	
					for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],mit.data()->speed[i]);
					ToOutAnything(0,S_massesSpeeds,1+N,sortie);
				}
			}
			return;
		}
		
		// more than 1 args
		if (auxtype == S_massesPos) 	// get mass positions
		{
			for(int j = 1; j<argc; j++) {
				if(IsSymbol(argv[j])) {
					typename IDMap<t_mass *>::iterator mit;
					for(mit = massids.find(GetSymbol(argv[j])); mit; ++mit) {
						SetSymbol(sortie[0],mit.data()->Id);
						for(int i = 0; i < N; ++i) SetFloat(sortie[1],mit.data()->pos[i]);
						ToOutAnything(0,S_massesPosId,1+N,sortie);
					}
				}
				else {
					t_mass *m = mass.find(GetAInt(argv[j]));
					if(m) {
						SetInt(sortie[0],m->nbr);
						for(int i = 0; i < N; ++i) SetFloat(sortie[1],m->pos[i]);
						ToOutAnything(0,S_massesPosNo,1+N,sortie);
					}
//					else
//						error("%s - %s : Index not found",thisName(),GetString(thisTag()));
				}
			}
		}
		else if (auxtype == S_massesForces)	// get mass forces
		{
			for(int j = 1; j<argc; j++) {
				if(IsSymbol(argv[j])) {
					typename IDMap<t_mass *>::iterator mit;
					for(mit = massids.find(GetSymbol(argv[j])); mit; ++mit) {
						SetSymbol(sortie[0],mit.data()->Id);
						for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],mit.data()->out_force[i]);
						ToOutAnything(0,S_massesForcesId,1+N,sortie);
					}
				}
				else {
					t_mass *m = mass.find(GetAInt(argv[j]));
					if(m) {
						SetInt(sortie[0],m->nbr);
						for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],m->out_force[i]);
						ToOutAnything(0,S_massesForcesNo,1+N,sortie);
					}
//					else
//						error("%s - %s : Index not found",thisName(),GetString(thisTag()));
				}
			}
		}
		else if (auxtype == S_linksPos)		// get links positions
		{
			for(int j = 1; j<argc; j++) {
				if(IsSymbol(argv[j])) {
					typename IDMap<t_link *>::iterator lit;
					for(lit = linkids.find(GetSymbol(argv[j])); lit; ++lit) {
						SetSymbol(sortie[0],lit.data()->Id);
						for(int i = 0; i < N; ++i) {
							SetFloat(sortie[1+i],lit.data()->mass1->pos[i]);
							SetFloat(sortie[1+N+i],lit.data()->mass2->pos[i]);
						}
						ToOutAnything(0,S_linksPosId,1+2*N,sortie);
					}
				}
				else {
					t_link *l = link.find(GetAInt(argv[j]));
					if(l) {
						SetInt(sortie[0],l->nbr);
						for(int i = 0; i < N; ++i) {
							SetFloat(sortie[1+i],l->mass1->pos[i]);
							SetFloat(sortie[1+N+i],l->mass2->pos[i]);
						}
						ToOutAnything(0,S_linksPosNo,1+2*N,sortie);
					}
//					else
//						error("%s - %s : Index not found",thisName(),GetString(thisTag()));
				}
			}
		}
		else 			 		// get mass speeds
		{
			for(int j = 1; j<argc; j++) {
				if(IsSymbol(argv[j])) {
					typename IDMap<t_mass *>::iterator mit;
					for(mit = massids.find(GetSymbol(argv[j])); mit; ++mit) {
						SetSymbol(sortie[0],mit.data()->Id);
						for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],mit.data()->speed[i]);
						ToOutAnything(0,S_massesSpeedsId,1+N,sortie);
					}
				}
				else {
					t_mass *m = mass.find(GetAInt(argv[j]));
					if(m) {
						SetInt(sortie[0],m->nbr);
						for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],m->speed[i]);
						ToOutAnything(0,S_massesSpeedsNo,1+N,sortie);
					}
//					else
//						error("%s - %s : Index not found",thisName(),GetString(thisTag()));
				}
			}
		}
	}

	// List of masses positions on first outlet
	void m_mass_dumpl()
	{	
		int sz = mass.size();
		NEWARR(t_atom,sortie,sz*N);
		t_atom *s = sortie;
		for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
			for(int i = 0; i < N; ++i) SetFloat(*(s++),mit.data()->pos[i]);
		ToOutAnything(0, S_massesPosL, sz*N, sortie);
		DELARR(sortie);
	}

	// List of masses forces on first outlet
	void m_force_dumpl()
	{	
		int sz = mass.size();
		NEWARR(t_atom,sortie,sz*N);
		t_atom *s = sortie;
		for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
			for(int i = 0; i < N; ++i) SetFloat(*(s++),mit.data()->out_force[i]);
		ToOutAnything(0, S_massesForcesL, sz*N, sortie);
		DELARR(sortie);
	}

	// List of masses and links infos on second outlet
	void m_info_dumpl()
	{	
		for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
			outmass(S_Mass,mit.data());

		for(typename IndexMap<t_link *>::iterator lit(link); lit; ++lit)
			outlink(S_Link,lit.data());
	}


// --------------------------------------------------------------  SETUP
// ---------------------------------------------------------------------

private:

	void clear()
	{
		linkids.reset();
		link.reset();

		massids.reset();
		mass.reset();
		
		id_mass = id_link = 0;
	}
	
	void deletelink(t_link *l)
	{
		outlink(S_Link_deleted,l);
		linkids.erase(l);
		link.remove(l->nbr);
		delete l;
	}
	
	void outmass(const t_symbol *s,const t_mass *m)
	{
		t_atom sortie[4+N];
		SetInt((sortie[0]),m->nbr);
		SetSymbol((sortie[1]),m->Id);
		SetBool((sortie[2]),m->mobile);
		SetFloat((sortie[3]),1.f/m->invM);
		for(int i = 0; i < N; ++i) SetFloat((sortie[4+i]),m->pos[i]);
		ToOutAnything(1,s,4+N,sortie);
	}
	
	void outlink(const t_symbol *s,const t_link *l)
	{
		t_atom sortie[7];
		SetInt((sortie[0]),l->nbr);
		SetSymbol((sortie[1]),l->Id);
		SetInt((sortie[2]),l->mass1->nbr);
		SetInt((sortie[3]),l->mass2->nbr);
		SetFloat((sortie[4]),l->K1);
		SetFloat((sortie[5]),l->D1);
		SetFloat((sortie[6]),l->D2);
		ToOutAnything(1,s,7,sortie);
	}
	

	// Static symbols
	const static t_symbol *S_Reset;
	const static t_symbol *S_Mass;
	const static t_symbol *S_Link;
	const static t_symbol *S_iLink;
	const static t_symbol *S_Mass_deleted;
	const static t_symbol *S_Link_deleted;
	const static t_symbol *S_massesPos;
	const static t_symbol *S_massesPosNo;
	const static t_symbol *S_massesPosId;
	const static t_symbol *S_linksPos;
	const static t_symbol *S_linksPosNo;
	const static t_symbol *S_linksPosId;
	const static t_symbol *S_massesForces;
	const static t_symbol *S_massesForcesNo;
	const static t_symbol *S_massesForcesId;
	const static t_symbol *S_massesSpeeds;
	const static t_symbol *S_massesSpeedsNo;
	const static t_symbol *S_massesSpeedsId;
	const static t_symbol *S_massesPosL;
	const static t_symbol *S_massesForcesL;

	static void setup(t_classid c)
	{
		S_Reset = MakeSymbol("Reset");
		S_Mass = MakeSymbol("Mass");
		S_Link = MakeSymbol("Link");
		S_iLink = MakeSymbol("iLink");
		S_Mass_deleted = MakeSymbol("Mass deleted");
		S_Link_deleted = MakeSymbol("Link deleted");
		S_massesPos = MakeSymbol("massesPos");
		S_massesPosNo = MakeSymbol("massesPosNo");
		S_massesPosId = MakeSymbol("massesPosId");
		S_linksPos = MakeSymbol("linksPos");
		S_linksPosNo = MakeSymbol("linksPosNo");
		S_linksPosId = MakeSymbol("linksPosId");
		S_massesForces = MakeSymbol("massesForces");
		S_massesForcesNo = MakeSymbol("massesForcesNo");
		S_massesForcesId = MakeSymbol("massesForcesId");
		S_massesSpeeds = MakeSymbol("massesSpeeds");
		S_massesSpeedsNo = MakeSymbol("massesSpeedsNo");
		S_massesSpeedsId = MakeSymbol("massesSpeedsId");
		S_massesPosL = MakeSymbol("massesPosL");
		S_massesForcesL = MakeSymbol("massesForcesL");

		// --- set up methods (class scope) ---

		// register a bang method to the default inlet (0)
		FLEXT_CADDBANG(c,0,m_bang);

		// set up tagged methods for the default inlet (0)
		// the underscore _ after CADDMETHOD indicates that a message tag is used
		// no, variable list or anything and all single arguments are recognized automatically, ...
		FLEXT_CADDMETHOD_(c,0,"reset",m_reset);

		FLEXT_CADDMETHOD_(c,0,"forceX",m_forceX);
		FLEXT_CADDMETHOD_(c,0,"posX",m_posX);
		FLEXT_CADDMETHOD_(c,0,"Xmax",m_Xmax);
		FLEXT_CADDMETHOD_(c,0,"Xmin",m_Xmin);
		if(N >= 2) {
			FLEXT_CADDMETHOD_(c,0,"forceY",m_forceY);
			FLEXT_CADDMETHOD_(c,0,"posY",m_posY);
			FLEXT_CADDMETHOD_(c,0,"Ymax",m_Ymax);
			FLEXT_CADDMETHOD_(c,0,"Ymin",m_Ymin);
		}
		if(N >= 3) {
			FLEXT_CADDMETHOD_(c,0,"forceZ",m_forceZ);
			FLEXT_CADDMETHOD_(c,0,"posZ",m_posZ);
			FLEXT_CADDMETHOD_(c,0,"Zmax",m_Zmax);
			FLEXT_CADDMETHOD_(c,0,"Zmin",m_Zmin);
		}
		
		FLEXT_CADDMETHOD_(c,0,"setMobile",m_set_mobile);
		FLEXT_CADDMETHOD_(c,0,"setFixed",m_set_fixe);
		FLEXT_CADDMETHOD_(c,0,"setK",m_setK);
		FLEXT_CADDMETHOD_(c,0,"setD",m_setD);
		FLEXT_CADDMETHOD_(c,0,"setD2",m_setD2);
		FLEXT_CADDMETHOD_(c,0,"mass",m_mass);
		FLEXT_CADDMETHOD_(c,0,"link",m_link);
		FLEXT_CADDMETHOD_(c,0,"iLink",m_ilink);
		FLEXT_CADDMETHOD_(c,0,"get",m_get);
		FLEXT_CADDMETHOD_(c,0,"deleteLink",m_delete_link);
		FLEXT_CADDMETHOD_(c,0,"deleteMass",m_delete_mass);
		FLEXT_CADDMETHOD_(c,0,"massesPosL",m_mass_dumpl);
		FLEXT_CADDMETHOD_(c,0,"infosL",m_info_dumpl);
		FLEXT_CADDMETHOD_(c,0,"massesForcesL",m_force_dumpl);
	}

	// for every registered method a callback has to be declared
	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK(m_mass_dumpl)
	FLEXT_CALLBACK(m_info_dumpl)
	FLEXT_CALLBACK(m_force_dumpl)
	FLEXT_CALLBACK(m_reset)
	FLEXT_CALLBACK_V(m_set_mobile)
	FLEXT_CALLBACK_V(m_set_fixe)
	FLEXT_CALLBACK_V(m_mass)
	FLEXT_CALLBACK_V(m_link)
	FLEXT_CALLBACK_V(m_ilink)
	FLEXT_CALLBACK_V(m_Xmax)
	FLEXT_CALLBACK_V(m_Xmin)
	FLEXT_CALLBACK_V(m_forceX)
	FLEXT_CALLBACK_V(m_posX)
	FLEXT_CALLBACK_V(m_Ymax)
	FLEXT_CALLBACK_V(m_Ymin)
	FLEXT_CALLBACK_V(m_forceY)
	FLEXT_CALLBACK_V(m_posY)
	FLEXT_CALLBACK_V(m_Zmax)
	FLEXT_CALLBACK_V(m_Zmin)
	FLEXT_CALLBACK_V(m_forceZ)
	FLEXT_CALLBACK_V(m_posZ)
	FLEXT_CALLBACK_V(m_setK)
	FLEXT_CALLBACK_V(m_setD)
	FLEXT_CALLBACK_V(m_setD2)
	FLEXT_CALLBACK_V(m_get)
	FLEXT_CALLBACK_V(m_delete_link)
	FLEXT_CALLBACK_V(m_delete_mass)
};
// -------------------------------------------------------------- STATIC VARIABLES
// -------------------------------------------------------------------------------

#define MSD(NAME,CLASS,N) \
const t_symbol \
	*msdN<N>::S_Reset,*msdN<N>::S_Mass, \
	*msdN<N>::S_Link,*msdN<N>::S_iLink, \
	*msdN<N>::S_Mass_deleted,*msdN<N>::S_Link_deleted, \
	*msdN<N>::S_massesPos,*msdN<N>::S_massesPosNo,*msdN<N>::S_massesPosId, \
	*msdN<N>::S_linksPos,*msdN<N>::S_linksPosNo,*msdN<N>::S_linksPosId, \
	*msdN<N>::S_massesForces,*msdN<N>::S_massesForcesNo,*msdN<N>::S_massesForcesId, \
	*msdN<N>::S_massesSpeeds,*msdN<N>::S_massesSpeedsNo,*msdN<N>::S_massesSpeedsId, \
	*msdN<N>::S_massesPosL,*msdN<N>::S_massesForcesL; \
\
typedef msdN<N> CLASS; \
FLEXT_NEW_V(NAME,CLASS)
