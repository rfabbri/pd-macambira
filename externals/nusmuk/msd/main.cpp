

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

 Version 0.02 -- 15.04.2005
*/

// include flext header
#include <flext.h>
#include <math.h>

// define constants
#define MSD_VERSION  0.02
#define nb_max_link   4000
#define nb_max_mass   4000
#define Id_length   20

// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif

#define max(a,b) ( ((a) > (b)) ? (a) : (b) ) 
#define min(a,b) ( ((a) < (b)) ? (a) : (b) )

typedef struct _mass {
	t_symbol *Id;
	t_int nbr;
	t_int mobile;
	t_float invM;
	t_float speedX;
	t_float posX;
	t_float posX2;
	t_float forceX;
	t_float out_forceX;
} t_mass;

typedef struct _link {
	t_symbol *Id;
	t_int nbr;
	t_mass *mass1;
	t_mass *mass2;
	t_float K1, D1, D2;
	t_float longx, longueur;
	t_float distance_old;
} t_link;



class msd:
	public flext_base
{
	FLEXT_HEADER_S(msd,flext_base,setup)	//class with setup
 
public:
	// constructor with no arguments
	msd(int argc,t_atom *argv)
	{

		nb_link = 0;
		nb_mass = 0;
		id_mass = 0;
		id_link = 0;

		// --- define inlets and outlets ---
		AddInAnything("bang, reset, etc."); 	// default inlet
		AddOutAnything("infos on masses");	// outlet for integer count
		AddOutAnything("control");		// outlet for bang
	}



protected:

// ---------------------------------------------------------------  RESET 
// ----------------------------------------------------------------------
	void m_reset() 
	{ 
		t_int i;
		t_atom sortie[0];

		for (i=0; i<nb_mass; i++)	{
			delete mass[i];
			}
		for (i=0; i<nb_link; i++)	{
			delete link[i];
			}
		ToOutAnything(1,S_Reset,0,sortie);
		nb_link = 0;
		nb_mass = 0;
		id_mass = 0;
		id_link = 0;
	}

// --------------------------------------------------------------  COMPUTE 
// -----------------------------------------------------------------------

	void m_bang()
	{
		t_float F=0,Fx=0,distance,vitesse, X_new;
		t_int i;
		struct _mass mass_1, mass_2;
	
		for (i=0; i<nb_link; i++)	{
		// compute link forces
			distance = link[i]->mass1->posX-link[i]->mass2->posX;		// L[n] = x1 - x2
			F  = link[i]->K1 * (distance - link[i]->longueur) ;		// F[n] = k1 (L[n] - L[0])
			F += link[i]->D1 * (distance - link[i]->distance_old) ;		// F[n] = F[n] + D1 (L[n] - L[n-1])
			if (distance != 0) 	{
				Fx = F * (link[i]->mass1->posX - link[i]->mass2->posX)/distance; // Fx = F * Lx[n]/L[n]
			}
			link[i]->mass1->forceX -= Fx;					// Fx1[n] = -Fx
			link[i]->mass1->forceX -= link[i]->D2*link[i]->mass1->speedX; 	// Fx1[n] = Fx1[n] - D2 * vx1[n-1]
			link[i]->mass2->forceX += Fx;					// Fx2[n] = Fx
			link[i]->mass2->forceX -= link[i]->D2*link[i]->mass2->speedX; 	// Fx2[n] = Fx2[n] - D2 * vx2[n-1]
			link[i]->distance_old = distance;				// L[n-1] = L[n]
		}

		for (i=0; i<nb_mass; i++)
		// compute new masses position only if mobile = 1
			if (mass[i]->mobile == 1)  		{
				X_new = mass[i]->forceX * mass[i]->invM + 2*mass[i]->posX - mass[i]->posX2; // x[n] =Fx[n]/M+2x[n]-x[n-1] 
				mass[i]->posX2 = mass[i]->posX;				// x[n-2] = x[n-1]
				mass[i]->posX = max(min(X_new,Xmax),Xmin);		// x[n-1] = x[n]	
				mass[i]->speedX = mass[i]->posX - mass[i]->posX2;	// vx[n] = x[n] - x[n-1]
				}

		for (i=0; i<nb_mass; i++)	{
		// clear forces
			mass[i]->out_forceX = mass[i]->forceX;
			mass[i]->forceX = 0;						// Fx[n] = 0
		}
	}

// --------------------------------------------------------------  MASSES
// ----------------------------------------------------------------------
	void m_mass(int argc,t_atom *argv) 
	// add a mass
	// Id, nbr, mobile, invM, speedX, posX, forceX
	{
		t_atom sortie[5], aux[2];
		t_float M;

		mass[nb_mass] = new t_mass;			// new mass
		mass[nb_mass]->Id = GetSymbol(argv[0]);		// ID
		mass[nb_mass]->mobile = GetInt(argv[1]);	// mobile
		if (GetFloat(argv[2])==0)
			M=1;
		else M = GetFloat(argv[2]);			
		mass[nb_mass]->invM = 1/(M);			// 1/M
		mass[nb_mass]->speedX = 0;			// vx[n]
		mass[nb_mass]->posX = GetFloat(argv[3]);	// x[n]
		mass[nb_mass]->posX2 = GetFloat(argv[3]);	// x[n-1]
		mass[nb_mass]->forceX = 0;			// Fx[n]
		mass[nb_mass]->nbr = id_mass;			// id number
		nb_mass++ ;
		id_mass++;
		nb_mass = min ( nb_max_mass -1, nb_mass );
		SetFloat((sortie[0]),id_mass-1);
		SetSymbol((sortie[1]),GetSymbol(argv[0]));
		SetFloat((sortie[2]),mass[nb_mass-1]->mobile);
		SetFloat((sortie[3]),M);
		SetFloat((sortie[4]),mass[nb_mass-1]->posX);
		ToOutAnything(1,S_Mass,5,sortie);
	}

	void m_forceX(int argc,t_atom *argv) 
	{
	// add a force to mass(es) named Id
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		for (i=0; i<nb_mass;i++)
		{
			if (sym == mass[i]->Id)
				mass[i]->forceX += GetFloat(argv[1]);
		}
	}

	void m_posX(int argc,t_atom *argv) 
	{
	// displace mass(es) named Id to a certain position
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		if (GetFloat(argv[1]) < Xmax && GetFloat(argv[1]) > Xmin)
			for (i=0; i<nb_mass;i++)
			{
				if (sym == mass[i]->Id)
					mass[i]->posX = GetAFloat(argv[1]);
			}
	}

	void m_set_mobile(int argc,t_atom *argv) 
	{
	// set mass No to mobile
		t_int i,aux;
		
		aux = GetInt(argv[0]);	
		for (i=0; i<nb_mass;i++)
			{
				if (mass[i]->nbr == aux)
					mass[i]->mobile = 1;
			}
	
	}

	void m_set_fixe(int argc,t_atom *argv) 
	{
	// set mass No to fixed
		t_int i,aux;
		
		aux = GetInt(argv[0]);	
		for (i=0; i<nb_mass;i++)
			{
				if (mass[i]->nbr == aux)
					mass[i]->mobile = 0;
			}
	
	}

	void m_delete_mass(int argc,t_atom *argv) 
	{
	// Delete mass
	t_int i,nb_link_delete=0;
	t_atom sortie[5], aux[nb_link];
	

	// Delete all associated links 
	for (i=0; i<nb_link;i++)	{
		if (link[i]->mass1->nbr == GetAInt(argv[0]) || link[i]->mass2->nbr == GetAInt(argv[0]))	{
			SetFloat((aux[nb_link_delete]),link[i]->nbr);
			nb_link_delete++;
		}
	}

	for (i=0; i<nb_link_delete;i++)
		m_delete_link(1,&aux[i]);
	
	// Delete mass
	for (i=0; i<nb_mass;i++)
		if (mass[i]->nbr == GetAInt(argv[0]))	{
			SetFloat((sortie[0]),mass[i]->nbr);
			SetSymbol((sortie[1]),mass[i]->Id);
			SetFloat((sortie[2]),mass[i]->mobile);
			SetFloat((sortie[3]),1/mass[i]->invM);
			SetFloat((sortie[4]),mass[i]->posX);
			delete mass[i];
			mass[i] = mass[nb_mass-1]; 	// copy last mass instead 
			nb_mass--;
			ToOutAnything(1,S_Mass_deleted,5,sortie);
			break;
		}

	}


	void m_Xmax(int argc,t_atom *argv) 
	{
	// set X max
	Xmax = GetFloat(argv[0]);
	}

	void m_Xmin(int argc,t_atom *argv) 
	{
	// set X min
	Xmin = GetFloat(argv[0]);
	}

// --------------------------------------------------------------  LINKS 
// ---------------------------------------------------------------------

	void m_link(int argc,t_atom *argv) 
	// add a link
	// Id, nbr, *mass1, *mass2, K1, D1
	{
		t_atom sortie[7], aux[2];
		t_int i;

		link[nb_link] = new t_link;			// New pointer
		link[nb_link]->Id = GetSymbol(argv[0]);		// ID
		for (i=0; i<nb_mass;i++)
			if (mass[i]->nbr==GetAInt(argv[1]))	// pointer to mass1
				link[nb_link]->mass1 = mass[i];
			else if(mass[i]->nbr==GetAInt(argv[2]))	// pointer to mass2
				link[nb_link]->mass2 = mass[i];
		link[nb_link]->K1 = GetFloat(argv[3]);		// K1
		link[nb_link]->D1 = GetFloat(argv[4]);		// D1
		link[nb_link]->D2 = GetFloat(argv[5]);		// D2
		link[nb_link]->longx = link[nb_link]->mass1->posX - link[nb_link]->mass2->posX;	// Lx[0]
		link[nb_link]->longueur = link[nb_link]->longx ;// L[0]
		link[nb_link]->nbr = id_link;			// id number
		link[nb_link]->distance_old = link[nb_link]->longueur;	// L[n-1]
		nb_link++;
		id_link++;
		nb_link = min ( nb_max_link -1, nb_link );
		SetFloat((sortie[0]),id_link-1);
		SetSymbol((sortie[1]),link[nb_link-1]->Id);
		SetFloat((sortie[2]),GetInt(argv[1]));
		SetFloat((sortie[3]),GetInt(argv[2]));
		SetFloat((sortie[4]),link[nb_link-1]->K1);
		SetFloat((sortie[5]),link[nb_link-1]->D1);
		SetFloat((sortie[6]),link[nb_link-1]->D2);
		ToOutAnything(1,S_Link,7,sortie);
	}

	void m_ilink(int argc,t_atom *argv) 
	// add interactor link
	// Id, nbr, Id masses1, Id masses2, K1, D1
	{
		t_atom aux[2], arglist[6];
		t_int i,j, imass1[nb_mass], nbmass1=0, imass2[nb_mass], nbmass2=0;
		t_symbol *Id1, *Id2;

		Id1 = GetSymbol(argv[1]);
		Id2 = GetSymbol(argv[2]);
		ToOutAnything(1,S_iLink,0,aux);

		for (i=0;i<nb_mass;i++)	{
			if (Id1 == mass[i]->Id)	{
				imass1[nbmass1]=i;
				nbmass1++;
			}
			if (Id2 == mass[i]->Id)	{
				imass2[nbmass2]=i;
				nbmass2++;
			}
		}
		
		for(i=0;i<nbmass1;i++)
			for(j=0;j<nbmass2;j++)	
				if (imass1[i] != imass2[j])	{
					SetSymbol((arglist[0]),GetSymbol(argv[0]));
					SetFloat((arglist[1]),mass[imass1[i]]->nbr);
					SetFloat((arglist[2]),mass[imass2[j]]->nbr);
					SetFloat((arglist[3]),GetFloat(argv[3]));
					SetFloat((arglist[4]),GetFloat(argv[4]));
					SetFloat((arglist[5]),GetFloat(argv[5]));
					m_link(6,arglist);
				}
	}

	void m_setK(int argc,t_atom *argv) 
	{
	// set rigidity of link(s) named Id
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		for (i=0; i<nb_link;i++)
		{
			if (sym == link[i]->Id)
				link[i]->K1 = GetFloat(argv[1]);
		}
	}

	void m_setD(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		for (i=0; i<nb_link;i++)
		{
			if (sym == link[i]->Id)
				link[i]->D1 = GetFloat(argv[1]);
		}
	}

	void m_setD2(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		for (i=0; i<nb_link;i++)
		{
			if (sym == link[i]->Id)
				link[i]->D2 = GetFloat(argv[1]);
		}
	}

	void m_delete_link(int argc,t_atom *argv) 
	{
	// Delete link
	t_int i;
	t_atom sortie[7];

	for (i=0; i<nb_link;i++)
		if (link[i]->nbr == GetInt(argv[0]))	{
			SetFloat((sortie[0]),link[i]->nbr);
			SetSymbol((sortie[1]),link[i]->Id);
			SetFloat((sortie[2]),link[i]->mass1->nbr);
			SetFloat((sortie[3]),link[i]->mass2->nbr);
			SetFloat((sortie[4]),link[i]->K1);
			SetFloat((sortie[5]),link[i]->D1);
			SetFloat((sortie[6]),link[i]->D2);
			delete link[i];
			link[i]=link[nb_link-1];	// copy last link instead
			nb_link--;
			ToOutAnything(1,S_Link_deleted,7,sortie);
			break;
		}
	}


// --------------------------------------------------------------  GET 
// -------------------------------------------------------------------

	void m_get(int argc,t_atom *argv)
	// get attributes
	{
		t_int i,j;
		t_symbol *auxarg,*auxarg2, *auxtype;
		t_atom sortie[4];
		auxtype = GetSymbol(argv[0]);
		auxarg = GetASymbol(argv[1]);		//auxarg : & symbol, 0 else
		if (argc == 1)
		{
			if (auxtype == S_massesPos)// get all masses positions
				for (i=0; i<nb_mass; i++)
				{		
					SetFloat(sortie[0],mass[i]->posX);
					ToOutAnything(0,S_massesPos,1,sortie);
				}
			else if (auxtype == S_massesForces)// get all masses forces
				for (i=0; i<nb_mass; i++)
				{
					SetFloat(sortie[0],mass[i]->out_forceX);
					ToOutAnything(0,S_massesForces,1,sortie);
				}
			else if (auxtype == S_linksPos)// get all links positions
				for (i=0; i<nb_link; i++)
				{
					SetFloat(sortie[0],link[i]->mass1->posX);
					SetFloat(sortie[1],link[i]->mass2->posX);
					ToOutAnything(0,S_linksPos,2,sortie);
				}
			else 		// get all masses speeds
				for (i=0; i<nb_mass; i++)
				{
					SetFloat(sortie[0],mass[i]->speedX);
					ToOutAnything(0,S_massesSpeeds,1,sortie);
				}
		}
		else if (auxtype == S_massesPos) // get mass positions
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],mass[i]->posX);
							ToOutAnything(0,S_massesPosNo,1,sortie);
						}
			}
			else 		//symbol
			{
				for (j = 1; j<argc; j++)
				{
					auxarg2 = GetSymbol(argv[j]);
					for (i=0;i<nb_mass;i++)	
					{
						if (auxarg2==mass[i]->Id)
						{
							SetFloat(sortie[0],mass[i]->posX);
							ToOutAnything(0,S_massesPosId,1,sortie);
						}
					}
				}
			}
		}
		else if (auxtype == S_massesForces)			 // get mass forces
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],mass[i]->out_forceX);
							ToOutAnything(0,S_massesForcesNo,1,sortie);
						}
			}
			else 		//string
			{
				for (j = 1; j<argc; j++)
				{
					auxarg2 = GetSymbol(argv[j]);
					for (i=0;i<nb_mass;i++)	
					{
						if (auxarg2==mass[i]->Id)
						{
							SetFloat(sortie[0],mass[i]->out_forceX);
							ToOutAnything(0,S_massesForcesId,1,sortie);
						}	
					}
				}
			}
		}
		else if (auxtype == S_linksPos)			 // get links positions
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_link;i++)
						if (link[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],link[i]->mass1->posX);
							SetFloat(sortie[1],link[i]->mass2->posX);
							ToOutAnything(0,S_linksPosNo,2,sortie);
						}
			}
			else 		//symbol
			{
				for (j = 1; j<argc; j++)
				{
					auxarg2 = GetSymbol(argv[j]);
					for (i=0;i<nb_link;i++)	
					{
						if (auxarg2==link[i]->Id)
						{
							SetFloat(sortie[0],link[i]->mass1->posX);
							SetFloat(sortie[1],link[i]->mass2->posX);
							ToOutAnything(0,S_linksPosId,2,sortie);
						}
					}
				}
			}
		}
		else 			 // get mass speeds
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],mass[i]->speedX);
							ToOutAnything(0,S_massesSpeedsNo,1,sortie);
						}
			}
			else 		//symbol
			{
				for (j = 1; j<argc; j++)
				{
					auxarg2 = GetSymbol(argv[j]);
					for (i=0;i<nb_mass;i++)	
					{
						if (auxarg2==mass[i]->Id)
						{
							SETFLOAT(&sortie[0],mass[i]->speedX);
							ToOutAnything(0,S_massesSpeedsId,1,sortie);
						}
					}
				}
			}
		}
		

	}

	void m_mass_dumpl()
	// List of masses positions on first outlet
	{	
		t_atom sortie[nb_mass];
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SetFloat((sortie[i]),mass[i]->posX);
		}
		ToOutAnything(0, S_massesPosL, nb_mass, sortie);
	}

	void m_force_dumpl()
	// List of masses forces on first outlet
	{	
		t_atom sortie[nb_mass];
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SetFloat((sortie[i]),mass[i]->out_forceX);
		}
		ToOutAnything(0, S_massesForcesL, nb_mass, sortie);
	}

	void m_info_dumpl()
	// List of masses and links infos on first outlet
	{	
		t_atom sortie[7];
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SetFloat((sortie[0]),mass[i]->nbr);
			SetSymbol((sortie[1]),mass[i]->Id);
			SetFloat((sortie[2]),mass[i]->mobile);
			SetFloat((sortie[3]),1/mass[i]->invM);
			SetFloat((sortie[4]),mass[i]->posX);
		ToOutAnything(1, S_Mass, 5, sortie);
		}

		for (i=0; i<nb_link; i++)	{
			SetFloat((sortie[0]),link[i]->nbr);
			SetSymbol((sortie[1]),link[i]->Id);
			SetFloat((sortie[2]),link[i]->mass1->nbr);
			SetFloat((sortie[3]),link[i]->mass2->nbr);
			SetFloat((sortie[4]),link[i]->K1);
			SetFloat((sortie[5]),link[i]->D1);
			SetFloat((sortie[6]),link[i]->D2);
		ToOutAnything(1, S_Link, 7, sortie);
		}

	}


// --------------------------------------------------------------  PROTECTED VARIABLES 
// -----------------------------------------------------------------------------------

	t_link * link[nb_max_link];		// Pointer table on links
	t_mass * mass[nb_max_mass];		// Pointer table on masses
	t_float Xmin, Xmax;			// Limit values
	int nb_link, nb_mass, id_mass, id_link;

// --------------------------------------------------------------  SETUP
// ---------------------------------------------------------------------

private:

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

	void static setup(t_classid c)
	{

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
	FLEXT_CALLBACK_V(m_setK)
	FLEXT_CALLBACK_V(m_setD)
	FLEXT_CALLBACK_V(m_setD2)
	FLEXT_CALLBACK_V(m_forceX)
	FLEXT_CALLBACK_V(m_posX)
	FLEXT_CALLBACK_V(m_get)
	FLEXT_CALLBACK_V(m_delete_link)
	FLEXT_CALLBACK_V(m_delete_mass)
};
// -------------------------------------------------------------- STATIC VARIABLES
// -------------------------------------------------------------------------------

	const t_symbol *msd::S_Reset = MakeSymbol("Reset");
	const t_symbol *msd::S_Mass = MakeSymbol("Mass");
	const t_symbol *msd::S_Link = MakeSymbol("Link");
	const t_symbol *msd::S_iLink = MakeSymbol("iLink");
	const t_symbol *msd::S_Mass_deleted = MakeSymbol("Mass deleted");
	const t_symbol *msd::S_Link_deleted = MakeSymbol("Link deleted");
	const t_symbol *msd::S_massesPos = MakeSymbol("massesPos");
	const t_symbol *msd::S_massesPosNo = MakeSymbol("massesPosNo");
	const t_symbol *msd::S_massesPosId = MakeSymbol("massesPosId");
	const t_symbol *msd::S_linksPos = MakeSymbol("linksPos");
	const t_symbol *msd::S_linksPosNo = MakeSymbol("linksPosNo");
	const t_symbol *msd::S_linksPosId = MakeSymbol("linksPosId");
	const t_symbol *msd::S_massesForces = MakeSymbol("massesForces");
	const t_symbol *msd::S_massesForcesNo = MakeSymbol("massesForcesNo");
	const t_symbol *msd::S_massesForcesId = MakeSymbol("massesForcesId");
	const t_symbol *msd::S_massesSpeeds = MakeSymbol("massesSpeeds");
	const t_symbol *msd::S_massesSpeedsNo = MakeSymbol("massesSpeedsNo");
	const t_symbol *msd::S_massesSpeedsId = MakeSymbol("massesSpeedsId");
	const t_symbol *msd::S_massesPosL = MakeSymbol("massesPosL");
	const t_symbol *msd::S_massesForcesL = MakeSymbol("massesForcesL");

// instantiate the class (constructor has a variable argument list)
FLEXT_NEW_V("msd",msd)


