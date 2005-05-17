

/* 
 msd - mass spring damper model for Pure Data or Max/MSP

 Copyright (C) 2005  Nicolas Montgermont
 Written by Nicolas Montgermont for a Master's train in Acoustic,
 Signal processing and Computing Applied to Music (ATIAM, Paris 6) 
 at La Kitchen supervised by Cyrille Henry.
 
 Optimized by Thomas Grill for Flext
 Based on Pure Data by Miller Puckette and others
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

 Version 0.07 -- 17.05.2005
*/

// include flext header
#include <flext.h>
#include <flmap.h>
#include <math.h>
#include <string.h>
#include <vector>

// define constants
#define MSD_VERSION  0.07
#define PI  3.1415926535

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
		for(typename LinkList<N>::iterator it = this->begin(); it != this->end(); ++it)
			if(*it == l) return;
		// not found -> add
		push_back(l);
	}
	
	void erase(Link<N> *l)
	{
		for(typename LinkList<N>::iterator it = this->begin(); it != this->end(); ++it)
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
	t_float M,invM;
	t_float speed[N];
	t_float pos[N];
	t_float pos2[N];
	t_float force[N];
	t_float out_force[N];
	LinkList<N> links;
	
	Mass(t_int n,const t_symbol *id,bool mob,t_float m,t_float p[N])
		: nbr(n),Id(id)
		, M(m)
	{
		if(mob) setMobile(); else setFixed();
	
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
	
	inline bool getMobile() const { return invM != 0; }
	
	inline void setMobile() { invM = M?1.f/M:1.; }
	inline void setFixed() { invM = 0; }
	
	inline void compute(t_float limit[N][2])
	{
		for(int i = 0; i < N; ++i) {
			t_float pold = pos[i];
			t_float pnew = force[i] * invM + 2*pold - pos2[i]; // x[n] =Fx[n]/M+2x[n]-x[n-1]
			if(pnew < limit[i][0]) pnew = limit[i][0]; else if(pnew > limit[i][1]) pnew = limit[i][1];
			speed[i] = (pos[i] = pnew) - (pos2[i] = pold);	// x[n-2] = x[n-1], x[n-1] = x[n],vx[n] = x[n] - x[n-1]

			// clear forces
			out_force[i] = force[i];
			force[i] = 0;						// Fx[n] = 0
		}
	}

	static inline t_float dist(const Mass &m1,const Mass &m2) 
	{
		if(N == 1) 
			return fabs(m1.pos[0]-m2.pos[0]);		// L[n] = |x1 - x2|
		else {
			t_float distance = 0;
			for(int i = 0; i < N; ++i) distance += sqr(m1.pos[i]-m2.pos[i]);
			return sqrt(distance);
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
	t_float puissance;
	t_int oriented; //0 : no, 1 : tangential, 2 : normal
	t_float tdirection1[3], tdirection2[3];
	
	Link(t_int n,const t_symbol *id,Mass<N> *m1,Mass<N> *m2,t_float k1,t_float d1, t_int o=0, t_float xa=0, t_float ya=0, t_float za=0,t_float pow=1, t_float lmin = 0,t_float lmax = 1e10)
		: nbr(n),Id(id)
		, mass1(m1),mass2(m2)
		, K1(k1),D1(d1),D2(0),oriented(o),puissance(pow)
		, long_min(lmin),long_max(lmax)
	{
		for (int i=0; i<3; i++)	{
			tdirection1[i] = 0;
			tdirection2[i] = 0;
			}
		if (oriented == 0)
			distance_old = longueur = Mass<N>::dist(*mass1,*mass2); // L[n-1]
		else if (oriented == 1)	{			// TANGENTIAL LINK
			const t_float norme = sqrt(sqr(xa)+sqr(ya)+sqr(za));
			tdirection1[0] = xa/norme;
			tdirection1[1] = ya/norme;
			tdirection1[2] = za/norme;
			distance_old = 0;
			for(int i = 0; i < N; ++i)	
				distance_old += sqr((m1->pos[i]-m2->pos[i])*tdirection1[i]);
			distance_old  = sqrt(distance_old);
			longueur = distance_old;
		}
		else if (oriented == 2)	{			// NORMAL LINK 2D
			if (N >= 2)	{
				const t_float norme = sqrt(sqr(xa)+sqr(ya));
				tdirection1[0]=ya/norme;
				tdirection1[1]=xa/norme;
				distance_old = 0;
				for(int i = 0; i < N; ++i)	
					distance_old += sqr((m1->pos[i]-m2->pos[i])*tdirection1[i]);
				if (N == 3)	{			// NORMAL LINK 3D
					if (xa == 0 && ya==0 && za!= 0)	{	// Special case
						tdirection1[0]=1;
						tdirection1[1]=0;
						tdirection1[2]=0;
						tdirection2[0]=0;
						tdirection2[1]=1;
						tdirection2[2]=0;
					}
					else	{				// Normal case
						const t_float norme2 = sqrt(sqr(xa*ya +za*xa)+sqr(xa*ya+za*ya)+sqr(sqr(xa)+sqr(ya)));
						tdirection2[0] = (xa*za+xa*ya)/norme2;
						tdirection2[1] = (xa*ya+za*ya)/norme2;
						tdirection2[2] = (sqr(xa)+sqr(ya))/norme2;
					}
					distance_old = 0;
					for(int i = 0; i < N; ++i)	
						distance_old += sqr((m1->pos[i]-m2->pos[i])*(tdirection1[i]+tdirection2[i]));
				}
				distance_old  = sqrt(distance_old);
				longueur = distance_old;			
			}
		}	
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
		t_float distance=0;
		t_float F;
		Mass<N> *m1 = mass1,*m2 = mass2; // cache locally
		if (oriented == 0)	
			distance = Mass<N>::dist(*m1,*m2); 
		else if (oriented == 1) {
			for(int i = 0; i < N; ++i)	
				distance += sqr((m1->pos[i]-m2->pos[i])*tdirection1[i]);
			distance = sqrt(distance);
		}
		else if (oriented == 2) {
			for(int i = 0; i < N; ++i)	
				distance += sqr((m1->pos[i]-m2->pos[i])*(tdirection1[i] +tdirection2[i]));
			distance = sqrt(distance);
		}

		if (distance < long_min || distance > long_max || distance == 0) {
//			for(int i = 0; i < N; ++i) {
	//			m1->force[i] -= D2 * m1->speed[i]; 	//  Fx1[n] = -Fx, Fx1[n] = Fx1[n] - D2 * vx1[n-1]
	//			m2->force[i] += D2 * m2->speed[i]; 	// Fx2[n] = Fx, Fx2[n] = Fx2[n] - D2 * vx2[n-1]
	//		}
		}
		else {	// Lmin < L < Lmax
			// F[n] = k1 (L[n] - L[0])/L[n] + D1 (L[n] - L[n-1])/L[n]
			if ((distance - longueur)>0)
				F  = (K1 * pow(distance - longueur,puissance) + D1 * (distance - distance_old))/distance ;
			else
				F  = (-K1 * pow(longueur - distance,puissance) + D1 * (distance - distance_old))/distance ;
			if (oriented == 0)	
				for(int i = 0; i < N; ++i) {
					const t_float Fn = F * (m1->pos[i] - m2->pos[i]); // Fx = F * Lx[n]/L[n]
					m1->force[i] -= Fn + D2 * m1->speed[i]; 	//  Fx1[n] = -Fx, Fx1[n] = Fx1[n] - D2 * vx1[n-1]
					m2->force[i] += Fn - D2 * m2->speed[i]; 	// Fx2[n] = Fx, Fx2[n] = Fx2[n] - D2 * vx2[n-1]
				}
			else if (oriented == 1 || (oriented == 2 && N == 2))
				for(int i = 0; i < N; ++i) {
					const t_float Fn = F * (m1->pos[i] - m2->pos[i])*tdirection1[i]; // Fx = F * Lx[n]/L[n]
					m1->force[i] -= Fn + D2 * m1->speed[i]; 	//  Fx1[n] = -Fx, Fx1[n] = Fx1[n] - D2 * vx1[n-1]
					m2->force[i] += Fn - D2 * m2->speed[i]; 	// Fx2[n] = Fx, Fx2[n] = Fx2[n] - D2 * vx2[n-1]
				}
			else if (oriented == 2 && N == 3)
				for(int i = 0; i < N; ++i) {
					const t_float Fn = F * (m1->pos[i] - m2->pos[i])*(tdirection1[i] +tdirection2[i]); // Fx = F * Lx[n]/L[n]
					m1->force[i] -= Fn + D2 * m1->speed[i]; 	//  Fx1[n] = -Fx, Fx1[n] = Fx1[n] - D2 * vx1[n-1]
					m2->force[i] += Fn - D2 * m2->speed[i]; 	// Fx2[n] = Fx, Fx2[n] = Fx2[n] - D2 * vx2[n-1]
				}
		}
		
		distance_old = distance;				// L[n-1] = L[n]			
	}
};


template <typename T>
inline T bitrev(T k) 
{
	T r = 0;
	for(int i = 0; i < sizeof(k)*8; ++i) r = (r<<1)|(k&1),k >>= 1;
	return r;
}

// use bit-reversed key to pseudo-balance the map tree
template <typename T>
class IndexMap
	: TablePtrMap<unsigned int,T,64>
{
public:
	typedef TablePtrMap<unsigned int,T,64> Parent;
	
    virtual ~IndexMap() { reset(); }

	void reset() 
	{ 
		// delete all associated items
		for(typename Parent::iterator it(*this); it; ++it) delete it.data();
		Parent::clear(); 
	}
	
	inline int size() const { return Parent::size(); }

	inline T insert(unsigned int k,T v) { return Parent::insert(bitrev(k),v); }	

	inline T find(unsigned int k) { return Parent::find(bitrev(k)); }

	inline T remove(unsigned int k) { return Parent::remove(bitrev(k)); }
	
	class iterator
		: public Parent::iterator
	{
	public:
		iterator() {}
		iterator(IndexMap &m): Parent::iterator(m) {}
		inline unsigned int key() const { return bitrev(Parent::key()); }
	};
};

template <typename T>
class IDMap
	: TablePtrMap<const t_symbol *,TablePtrMap<T,T,4> *,4>
{
public:
	// that's the container holding the data items (masses, links) of one ID
	typedef TablePtrMap<T,T,4> Container;
	// that's the map for the key ID (symbol,int) relating to the data items
	typedef TablePtrMap<const t_symbol *,Container *,4> Parent;

	typedef typename Container::iterator iterator;

	IDMap() {}
	
	virtual ~IDMap() { reset(); }
	
	void reset() 
	{
        typename Parent::iterator it(*this);
        for(; it; ++it) delete it.data();
		Parent::clear();
	}
	
	void insert(T item)
	{
		Container *c = Parent::find(item->Id);
        if(!c)
            Parent::insert(item->Id,c = new Container);
        c->insert(item,item);
	}
	
	iterator find(const t_symbol *key)
	{
		Container *c = Parent::find(key);
		if(c)
			return iterator(*c);
        else
			return iterator();
	}
	
	void erase(T item)
	{
		Container *c = Parent::find(item->Id);
		if(c) c->remove(item);
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
	unsigned int id_mass, id_link, mouse_grab, nearest_mass, link_deleted, mass_deleted;

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
			error("mass : Id mobile mass X%s%s",N >= 2?" Y":"",N >= 3?" Z":"");
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
			error("%s - %s Syntax : Id/Nomass",thisName(),GetString(thisTag()));
			return;
		}
		if(IsSymbol(argv[0])) {
			typename IDMap<t_mass *>::iterator it;
			if(mob)
				for(it = massids.find(GetSymbol(argv[0])); it; ++it)
					it.data()->setMobile();
			else
				for(it = massids.find(GetSymbol(argv[0])); it; ++it)
					it.data()->setFixed();
		}
		else {	
			t_mass *m = mass.find(GetAInt(argv[0]));
			if(m) 
				if(mob) m->setMobile(); 
				else m->setFixed();
			else
				error("%s - %s : Index not found",thisName(),GetString(thisTag()));
		}
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
		
		t_mass *m = mass.find(GetAInt(argv[0]));
		if(m) {
			// Delete all associated links 
			for(typename std::vector<t_link *>::iterator it = m->links.begin(); it < m->links.end(); ++it)
				deletelink(*it);
			outmass(S_Mass_deleted,m);
			massids.erase(m);
			mass.remove(m->nbr);
			delete m;
			mass_deleted = 1;
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

	void m_grab_mass(int argc,t_atom *argv) 
	{
	// grab nearest mass X Y
		t_mass **mi;
		t_float aux, distance;
		t_atom aux2[2];
 		bool mobil;

		// if click
		if (GetInt(argv[2])==1 && mass.size()>0)	{

			if (argc != 3)
				error("grabMass : X Y click");
			// first time we grab this mass?Find nearest mass
			if (mouse_grab == 0)	{
				t_mass *m = mass.find(0);
				aux = sqr(m->pos[0]-GetFloat(argv[0])) + sqr(m->pos[1]-GetFloat(argv[1]));
				nearest_mass = 0;
				for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit) {	
					distance = sqr(mit.data()->pos[0]-GetFloat(argv[0])) + sqr(mit.data()->pos[1]-GetFloat(argv[1]));
					if (distance<aux)	{
						aux = distance;
						nearest_mass = mit.data()->nbr;
					}
				}
			}
			
			// Set fixed if mobile
			mobil = mass.find(nearest_mass)->invM;
			SetInt(aux2[0],nearest_mass);
			if (mobil != 0)
				m_set_fixe(1,aux2);

			// Set XY
			SetFloat(aux2[1],GetFloat(argv[0]));
			m_posX(2,aux2);
			SetFloat(aux2[1],GetFloat(argv[1]));
			m_posY(2,aux2);

			// Set mobile
			if(mobil != 0)
				m_set_mobile(1,aux2);		
			
			// Current grabbing on
			mouse_grab = 1;
		}
		else
			// Grabing off
			mouse_grab = 0;
	}

// --------------------------------------------------------------  LINKS 
// ---------------------------------------------------------------------

	// add a link
	// Id, *mass1, *mass2, K1, D1, D2, (Lmin,Lmax)
	void m_link(int argc,t_atom *argv) 
	{
		if (argc < 5 || argc > 8) {
			error("%s - %s Syntax : Id No/Idmass1 No/Idmass2 K D1 (pow Lmin Lmax)",thisName(),GetString(thisTag()));
			return;
		}
		if (IsSymbol(argv[1]) && IsSymbol(argv[2]))	{		// ID & ID
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
						0,0,0,0,
						argc >= 6?GetFloat(argv[5]):1, // power
						argc >= 7?GetFloat(argv[6]):0,
						argc >= 8?GetFloat(argv[7]):1e10
					);
					linkids.insert(l);
					link.insert(id_link++,l);
					outlink(S_iLink,l);
				}
			}
		}
		else if (IsSymbol(argv[1])==0 && IsSymbol(argv[2]))	{	// No & ID
			typename IDMap<t_mass *>::iterator it2,it;
	 		t_mass *mass1 = mass.find(GetAInt(argv[1]));
			it2 = massids.find(GetSymbol(argv[2]));
			for(it = it2; it; ++it) {
				t_link *l = new t_link(
					id_link,
					GetSymbol(argv[0]), // ID
					mass1,it.data(), // pointer to mass1, mass2
					GetAFloat(argv[3]), // K1
					GetAFloat(argv[4]), // D1
					0,0,0,0,
					argc >= 6?GetFloat(argv[5]):1, // power
					argc >= 7?GetFloat(argv[6]):0,
					argc >= 8?GetFloat(argv[7]):1e10
				);
				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_iLink,l);
			}
		}
		else if (IsSymbol(argv[1]) && IsSymbol(argv[2])==0)	{	// ID & No
			typename IDMap<t_mass *>::iterator it1,it;
			it1 = massids.find(GetSymbol(argv[1]));
	 		t_mass *mass2 = mass.find(GetAInt(argv[2]));
			for(it = it1; it; ++it) {
				t_link *l = new t_link(
					id_link,
					GetSymbol(argv[0]), // ID
					it.data(),mass2, // pointer to mass1, mass2
					GetAFloat(argv[3]), // K1
					GetAFloat(argv[4]), // D1
					0,0,0,0,
					argc >= 6?GetFloat(argv[5]):1, // power
					argc >= 7?GetFloat(argv[6]):0,
					argc >= 8?GetFloat(argv[7]):1e10
				);
				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_iLink,l);
			}
		}
		else	{										// No & No
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
				0,0,0,0,
				argc >= 6?GetFloat(argv[5]):1, // power
				argc >= 7?GetFloat(argv[6]):0,	// Lmin
				argc >= 8?GetFloat(argv[7]):1e10// Lmax
			);

			linkids.insert(l);
			link.insert(id_link++,l);
			outlink(S_Link,l);
		}
	}
	// add interactor link
	// Id, Id masses1, Id masses2, K1, D1, D2, (Lmin, Lmax)
	void m_ilink(int argc,t_atom *argv) 
	{
		if (argc < 6 || argc > 8) {
			error("%s - %s Syntax : Id Idmass1 Idmass2 K D1 (pow Lmin Lmax)",thisName(),GetString(thisTag()));
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
					0,0,0,0,
					argc >= 6?GetFloat(argv[5]):1, // power
					argc >= 7?GetFloat(argv[6]):0,
					argc >= 8?GetFloat(argv[7]):1e10
				);

				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_iLink,l);
			}
		}
	}

	// add a tangential link
	// Id, *mass1, *mass2, K1, D1, D2, (Lmin,Lmax)
	void m_tlink(int argc,t_atom *argv) 
	{
		if (argc < 5+N || argc > 8+N) {
			error("%s - %s Syntax : Id Nomass1 Nomass2 K D1 xa%s%s (pow Lmin Lmax)",thisName(),GetString(thisTag()),N >= 2?" ya":"",N >= 3?" za":"");
			return;
		}

		if (IsSymbol(argv[1]) && IsSymbol(argv[2]))	{		// ID & ID
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
						1,					// tangential
						GetAFloat(argv[5]),N >= 2?GetAFloat(argv[6]):0,N >= 3?GetAFloat(argv[7]):0,	// vector
						(N==1 && argc >= 7)?GetFloat(argv[6]):((N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1)), // power
						(N==1 && argc >= 8)?GetFloat(argv[7]):((N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0)),	// Lmin
						(N==1 && argc >= 9)?GetFloat(argv[8]):((N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10))// Lmax

					);
					linkids.insert(l);
					link.insert(id_link++,l);
					outlink(S_iLink,l);
				}
			}
		}
		else if (IsSymbol(argv[1])==0 && IsSymbol(argv[2]))	{	// No & ID
			typename IDMap<t_mass *>::iterator it2,it;
	 		t_mass *mass1 = mass.find(GetAInt(argv[1]));
			it2 = massids.find(GetSymbol(argv[2]));
			for(it = it2; it; ++it) {
				t_link *l = new t_link(
					id_link,
					GetSymbol(argv[0]), // ID
					mass1,it.data(), // pointer to mass1, mass2
					GetAFloat(argv[3]), // K1
					GetAFloat(argv[4]), // D1
					1,					// tangential
					GetAFloat(argv[5]),N >= 2?GetAFloat(argv[6]):0,N >= 3?GetAFloat(argv[7]):0,	// vector
					(N==1 && argc >= 7)?GetFloat(argv[6]):((N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1)), // power
					(N==1 && argc >= 8)?GetFloat(argv[7]):((N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0)),	// Lmin
					(N==1 && argc >= 9)?GetFloat(argv[8]):((N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10))// Lmax
				);
				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_iLink,l);
			}
		}
		else if (IsSymbol(argv[1]) && IsSymbol(argv[2])==0)	{	// ID & No
			typename IDMap<t_mass *>::iterator it1,it;
			it1 = massids.find(GetSymbol(argv[1]));
	 		t_mass *mass2 = mass.find(GetAInt(argv[2]));
			for(it = it1; it; ++it) {
				t_link *l = new t_link(
					id_link,
					GetSymbol(argv[0]), // ID
					it.data(),mass2, // pointer to mass1, mass2
					GetAFloat(argv[3]), // K1
					GetAFloat(argv[4]), // D1
					1,					// tangential
					GetAFloat(argv[5]),N >= 2?GetAFloat(argv[6]):0,N >= 3?GetAFloat(argv[7]):0,	// vector
					(N==1 && argc >= 7)?GetFloat(argv[6]):((N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1)), // power
					(N==1 && argc >= 8)?GetFloat(argv[7]):((N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0)),	// Lmin
					(N==1 && argc >= 9)?GetFloat(argv[8]):((N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10))// Lmax
				);
				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_iLink,l);
			}
		}
		else	{										// No & No
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
				1,					// tangential
				GetAFloat(argv[5]),N >= 2?GetAFloat(argv[6]):0,N >= 3?GetAFloat(argv[7]):0,	// vector
				(N==1 && argc >= 7)?GetFloat(argv[6]):((N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1)), // power
				(N==1 && argc >= 8)?GetFloat(argv[7]):((N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0)),	// Lmin
				(N==1 && argc >= 9)?GetFloat(argv[8]):((N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10))// Lmax
			);
			linkids.insert(l);
			link.insert(id_link++,l);
			outlink(S_tLink,l);
		}
	}

	// add a normal link
	// Id, *mass1, *mass2, K1, D1, D2, (Lmin,Lmax)
	void m_nlink(int argc,t_atom *argv) 
	{
		if (argc < 5+N || argc > 8+N) {
			error("%s - %s Syntax : Id No/Idmass1 No/Idmass2 K D1 xa%s%s (pow Lmin Lmax)",thisName(),GetString(thisTag()),N >= 2?" ya":"",N >= 3?" za":"");
			return;
		}

		if (N==1)	{
			error("%s - %s : No normal Link in 1D",thisName(),GetString(thisTag()));
			return;
		}
		if (IsSymbol(argv[1]) && IsSymbol(argv[2]))	{		// ID & ID
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
						2,					// normal
						GetAFloat(argv[5]),GetAFloat(argv[6]),N >= 3?GetAFloat(argv[7]):0,	// vector
						(N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1),	// pow
						(N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0),	// Lmin
						(N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10)// Lmax
					);
					linkids.insert(l);
					link.insert(id_link++,l);
					outlink(S_nLink,l);
				}
			}
		}
		else if (IsSymbol(argv[1])==0 && IsSymbol(argv[2]))	{	// No & ID
			typename IDMap<t_mass *>::iterator it2,it;
	 		t_mass *mass1 = mass.find(GetAInt(argv[1]));
			it2 = massids.find(GetSymbol(argv[2]));
			for(it = it2; it; ++it) {
				t_link *l = new t_link(
					id_link,
					GetSymbol(argv[0]), // ID
					mass1,it.data(), // pointer to mass1, mass2
					GetAFloat(argv[3]), // K1
					GetAFloat(argv[4]), // D1
					2,					// normal
					GetAFloat(argv[5]),GetAFloat(argv[6]),N >= 3?GetAFloat(argv[7]):0,	// vector
					(N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1),	// pow
					(N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0),	// Lmin
					(N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10)// Lmax
				);
				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_nLink,l);
			}
		}
		else if (IsSymbol(argv[1]) && IsSymbol(argv[2])==0)	{	// ID & No
			typename IDMap<t_mass *>::iterator it1,it;
			it1 = massids.find(GetSymbol(argv[1]));
	 		t_mass *mass2 = mass.find(GetAInt(argv[2]));
			for(it = it1; it; ++it) {
				t_link *l = new t_link(
					id_link,
					GetSymbol(argv[0]), // ID
					it.data(),mass2, // pointer to mass1, mass2
					GetAFloat(argv[3]), // K1
					GetAFloat(argv[4]), // D1
					2,					// normal
					GetAFloat(argv[5]),GetAFloat(argv[6]),N >= 3?GetAFloat(argv[7]):0,	// vector
					(N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1),	// pow
					(N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0),	// Lmin
					(N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10)// Lmax
				);
				linkids.insert(l);
				link.insert(id_link++,l);
				outlink(S_nLink,l);
			}
		}
		else	{										// No & No
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
				2,					// normal
				GetAFloat(argv[5]),GetAFloat(argv[6]),N >= 3?GetAFloat(argv[7]):0,	// vector
				(N==2 && argc >= 8)?GetFloat(argv[7]):((N==3 && argc >= 9)?GetFloat(argv[8]):1),	// pow
				(N==2 && argc >= 9)?GetFloat(argv[8]):((N==3 && argc >= 10)?GetFloat(argv[9]):0),	// Lmin
				(N==2 && argc >= 10)?GetFloat(argv[9]):((N==3 && argc >= 11)?GetFloat(argv[10]):1e10)// Lmax
			);
			linkids.insert(l);
			link.insert(id_link++,l);
			outlink(S_nLink,l);
		}
	}

	// set rigidity of link(s) named Id or number No
	void m_setK(int argc,t_atom *argv) 
	{
		if (argc != 2) {
			error("%s - %s Syntax : Id/NoLink Value",thisName(),GetString(thisTag()));
			return;
		}

		const t_float k1 = GetAFloat(argv[1]);

		if(IsSymbol(argv[0])) {
			typename IDMap<t_link *>::iterator it;
			for(it = linkids.find(GetSymbol(argv[0])); it; ++it)
				it.data()->K1 = k1;
		}
		else	{
			t_link *l = link.find(GetAInt(argv[0]));
			if(l)
				l->K1 = k1;
			else
				error("%s - %s : Index not found",thisName(),GetString(thisTag()));
		}
	}

	// set damping of link(s) named Id or number No
	void m_setD(int argc,t_atom *argv) 
	{
		if (argc != 2) {
			error("%s - %s Syntax : Id/NoLink Value",thisName(),GetString(thisTag()));
			return;
		}

		const t_float d1 = GetAFloat(argv[1]);

		if(IsSymbol(argv[0])) {
			typename IDMap<t_link *>::iterator it;
			for(it = linkids.find(GetSymbol(argv[0])); it; ++it)
				it.data()->D1 = d1;
		}
		else	{
			t_link *l = link.find(GetAInt(argv[0]));
			if(l)
				l->D1 = d1;
			else
				error("%s - %s : Index not found",thisName(),GetString(thisTag()));
		}
	}

	// set initial lenght of link(s) named Id or number No
	void m_setL(int argc,t_atom *argv) 
	{
		if (argc != 2) {
			error("%s - %s Syntax : Id/NoLink Value",thisName(),GetString(thisTag()));
			return;
		}

		const t_float lon = GetAFloat(argv[1]);

		if(IsSymbol(argv[0])) {
			typename IDMap<t_link *>::iterator it;
			for(it = linkids.find(GetSymbol(argv[0])); it; ++it) {
				it.data()->longueur = lon;
				it.data()->distance_old = lon;
			}
		}
		else	{
			t_link *l = link.find(GetAInt(argv[0]));
			if(l) {
				l->longueur = lon;
				l->distance_old = lon;
			}
			else
				error("%s - %s : Index not found",thisName(),GetString(thisTag()));
		}
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
			error("%s - %s Syntax : NtLink",thisName(),GetString(thisTag()));
			return;
		}
		
		t_link *l = link.find(GetAInt(argv[0]));
		if(l)	{
			deletelink(l);
			link_deleted = 1;
		}
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
						for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],mit.data()->pos[i]);
						ToOutAnything(0,S_massesPosId,1+N,sortie);
					}
				}
				else {
					t_mass *m = mass.find(GetAInt(argv[j]));
					if(m) {
						SetInt(sortie[0],m->nbr);
						for(int i = 0; i < N; ++i) SetFloat(sortie[1+i],m->pos[i]);
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
		if (mass_deleted ==0)	{
			int sz = mass.size();
			NEWARR(t_atom,sortie,sz*N);
			t_atom *s = sortie;
			for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
				for(int i = 0; i < N; ++i) SetFloat(s[mit.data()->nbr*N+i],mit.data()->pos[i]);
			ToOutAnything(0, S_massesPosL, sz*N, sortie);
			DELARR(sortie);
		}
		else
			error("%s - %s : Message Forbidden when deletion is used",thisName(),GetString(thisTag()));
	}

	// List of masses x positions on first outlet
	void m_mass_dump_xl()
	{	
		if (mass_deleted ==0)	{
			int sz = mass.size();
			NEWARR(t_atom,sortie,sz);
			t_atom *s = sortie;
			for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
				SetFloat(s[mit.data()->nbr],mit.data()->pos[0]);
			ToOutAnything(0, S_massesPosXL, sz, sortie);
			DELARR(sortie);
		}
		else
			error("%s - %s : Message Forbidden when deletion is used",thisName(),GetString(thisTag()));
	}

	// List of masses y positions on first outlet
	void m_mass_dump_yl()
	{	
		if (mass_deleted ==0)	{
			int sz = mass.size();
			NEWARR(t_atom,sortie,sz);
			t_atom *s = sortie;
			for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
				SetFloat(s[mit.data()->nbr],mit.data()->pos[1]);
			ToOutAnything(0, S_massesPosYL, sz, sortie);
			DELARR(sortie);
		}
		else
			error("%s - %s : Message Forbidden when deletion is used",thisName(),GetString(thisTag()));
	}

	// List of masses z positions on first outlet
	void m_mass_dump_zl()
	{	
		if (mass_deleted ==0)	{
			int sz = mass.size();
			NEWARR(t_atom,sortie,sz);
			t_atom *s = sortie;
			for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
				SetFloat(s[mit.data()->nbr],mit.data()->pos[2]);
			ToOutAnything(0, S_massesPosZL, sz, sortie);
			DELARR(sortie);
		}
		else
			error("%s - %s : Message Forbidden when deletion is used",thisName(),GetString(thisTag()));
	}

	// List of masses forces on first outlet
	void m_force_dumpl()
	{	
		if (mass_deleted ==0)	{
			int sz = mass.size();
			NEWARR(t_atom,sortie,sz*N);
			t_atom *s = sortie;
			for(typename IndexMap<t_mass *>::iterator mit(mass); mit; ++mit)
				for(int i = 0; i < N; ++i) SetFloat(s[mit.data()->nbr*N+i],mit.data()->out_force[i]);
			ToOutAnything(0, S_massesForcesL, sz*N, sortie);
			DELARR(sortie);
		}
		else
			error("%s - %s : Message Forbidden when deletion is used",thisName(),GetString(thisTag()));
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
		// Reset state variables 		
		id_mass = id_link = mouse_grab = mass_deleted = link_deleted = 0;
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
		SetBool((sortie[2]),m->getMobile());
		SetFloat((sortie[3]),m->M);
		for(int i = 0; i < N; ++i) SetFloat((sortie[4+i]),m->pos[i]);
		ToOutAnything(1,s,4+N,sortie);
	}
	
	void outlink(const t_symbol *s,const t_link *l)
	{
		t_atom sortie[15];
		int size=6;
		SetInt((sortie[0]),l->nbr);
		SetSymbol((sortie[1]),l->Id);
		SetInt((sortie[2]),l->mass1->nbr);
		SetInt((sortie[3]),l->mass2->nbr);
		SetFloat((sortie[4]),l->K1);
		SetFloat((sortie[5]),l->D1);

		if (l->oriented == 1 ||(l->oriented == 2 && N ==2))	{
			for (int i=0; i<N; i++)
				SetFloat((sortie[6+i]),l->tdirection1[i]);
//			ToOutAnything(1,s,6+N,sortie);
			size = 6+N;
		}
		else if (l->oriented == 2 && N==3)	{
			for (int i=0; i<N; i++)	{
				SetFloat((sortie[6+i]),l->tdirection1[i]);
				SetFloat((sortie[6+i+N]),l->tdirection2[i]);	
			}			
//			ToOutAnything(1,s,6+2*N,sortie);
			size = 6+2*N;
		}

		if(l->long_max != 1e10)	{
			SetFloat((sortie[size]),l->puissance);
			size++;
			SetFloat((sortie[size]),l->long_min);
			size++;
			SetFloat((sortie[size]),l->long_max);
			size++;
		}
		else if(l->long_min != 0) {
			SetFloat((sortie[size]),l->puissance);
			size++;
			SetFloat((sortie[size]),l->long_min);
			size++;
		}
		else if(l->puissance != 1)	{
			SetFloat((sortie[size]),l->puissance);
			size++;
		}
		ToOutAnything(1,s,size,sortie);
	}
	

	// Static symbols
	const static t_symbol *S_Reset;
	const static t_symbol *S_Mass;
	const static t_symbol *S_Link;
	const static t_symbol *S_iLink;
	const static t_symbol *S_tLink;
	const static t_symbol *S_nLink;
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
	const static t_symbol *S_massesPosXL;
	const static t_symbol *S_massesPosYL;
	const static t_symbol *S_massesPosZL;
	const static t_symbol *S_massesForcesL;

	static void setup(t_classid c)
	{
		S_Reset = MakeSymbol("Reset");
		S_Mass = MakeSymbol("Mass");
		S_Link = MakeSymbol("Link");
		S_iLink = MakeSymbol("iLink");
		S_tLink = MakeSymbol("tLink");
		S_nLink = MakeSymbol("nLink");
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
		S_massesPosXL = MakeSymbol("massesPosXL");
		S_massesPosYL = MakeSymbol("massesPosYL");
		S_massesPosZL = MakeSymbol("massesPosZL");
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
		FLEXT_CADDMETHOD_(c,0,"massesPosL",m_mass_dumpl);
		FLEXT_CADDMETHOD_(c,0,"massesPosXL",m_mass_dump_xl);
		if(N >= 2) {
			FLEXT_CADDMETHOD_(c,0,"forceY",m_forceY);
			FLEXT_CADDMETHOD_(c,0,"posY",m_posY);
			FLEXT_CADDMETHOD_(c,0,"Ymax",m_Ymax);
			FLEXT_CADDMETHOD_(c,0,"Ymin",m_Ymin);
			FLEXT_CADDMETHOD_(c,0,"massesPosYL",m_mass_dump_yl);
			FLEXT_CADDMETHOD_(c,0,"grabMass",m_grab_mass);
		}
		if(N >= 3) {
			FLEXT_CADDMETHOD_(c,0,"forceZ",m_forceZ);
			FLEXT_CADDMETHOD_(c,0,"posZ",m_posZ);
			FLEXT_CADDMETHOD_(c,0,"Zmax",m_Zmax);
			FLEXT_CADDMETHOD_(c,0,"Zmin",m_Zmin);
			FLEXT_CADDMETHOD_(c,0,"massesPosZL",m_mass_dump_zl);
		}
		
		FLEXT_CADDMETHOD_(c,0,"setMobile",m_set_mobile);
		FLEXT_CADDMETHOD_(c,0,"setFixed",m_set_fixe);
		FLEXT_CADDMETHOD_(c,0,"setK",m_setK);
		FLEXT_CADDMETHOD_(c,0,"setD",m_setD);
		FLEXT_CADDMETHOD_(c,0,"setL",m_setL);
		FLEXT_CADDMETHOD_(c,0,"setD2",m_setD2);
		FLEXT_CADDMETHOD_(c,0,"mass",m_mass);
		FLEXT_CADDMETHOD_(c,0,"link",m_link);
		FLEXT_CADDMETHOD_(c,0,"iLink",m_ilink);
		FLEXT_CADDMETHOD_(c,0,"tLink",m_tlink);
		FLEXT_CADDMETHOD_(c,0,"nLink",m_nlink);
		FLEXT_CADDMETHOD_(c,0,"get",m_get);
		FLEXT_CADDMETHOD_(c,0,"deleteLink",m_delete_link);
		FLEXT_CADDMETHOD_(c,0,"deleteMass",m_delete_mass);
		FLEXT_CADDMETHOD_(c,0,"infosL",m_info_dumpl);
		FLEXT_CADDMETHOD_(c,0,"massesForcesL",m_force_dumpl);
	}

	// for every registered method a callback has to be declared
	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK(m_mass_dumpl)
	FLEXT_CALLBACK(m_mass_dump_xl)
	FLEXT_CALLBACK(m_mass_dump_yl)
	FLEXT_CALLBACK(m_mass_dump_zl)
	FLEXT_CALLBACK(m_info_dumpl)
	FLEXT_CALLBACK(m_force_dumpl)
	FLEXT_CALLBACK(m_reset)
	FLEXT_CALLBACK_V(m_set_mobile)
	FLEXT_CALLBACK_V(m_set_fixe)
	FLEXT_CALLBACK_V(m_mass)
	FLEXT_CALLBACK_V(m_link)
	FLEXT_CALLBACK_V(m_ilink)
	FLEXT_CALLBACK_V(m_tlink)
	FLEXT_CALLBACK_V(m_nlink)
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
	FLEXT_CALLBACK_V(m_setL)
	FLEXT_CALLBACK_V(m_setD2)
	FLEXT_CALLBACK_V(m_get)
	FLEXT_CALLBACK_V(m_delete_link)
	FLEXT_CALLBACK_V(m_delete_mass)
	FLEXT_CALLBACK_V(m_grab_mass)
};
// -------------------------------------------------------------- STATIC VARIABLES
// -------------------------------------------------------------------------------

#define MSD(NAME,CLASS,N) \
const t_symbol \
	*msdN<N>::S_Reset,*msdN<N>::S_Mass, \
	*msdN<N>::S_Link,*msdN<N>::S_iLink,*msdN<N>::S_tLink,*msdN<N>::S_nLink, \
	*msdN<N>::S_Mass_deleted,*msdN<N>::S_Link_deleted, \
	*msdN<N>::S_massesPos,*msdN<N>::S_massesPosNo,*msdN<N>::S_massesPosId, \
	*msdN<N>::S_linksPos,*msdN<N>::S_linksPosNo,*msdN<N>::S_linksPosId, \
	*msdN<N>::S_massesForces,*msdN<N>::S_massesForcesNo,*msdN<N>::S_massesForcesId, \
	*msdN<N>::S_massesSpeeds,*msdN<N>::S_massesSpeedsNo,*msdN<N>::S_massesSpeedsId, \
	*msdN<N>::S_massesPosL,*msdN<N>::S_massesPosXL,*msdN<N>::S_massesPosYL, \
	*msdN<N>::S_massesPosZL,*msdN<N>::S_massesForcesL; \
\
typedef msdN<N> CLASS; \
FLEXT_NEW_V(NAME,CLASS)
