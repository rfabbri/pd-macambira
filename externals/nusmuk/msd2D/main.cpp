

/* 
 msd2D - mass spring damper model for Pure Data or Max/MSP

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

 Version 0.04 -- 26.04.2005
*/

// include flext header
#include <flext.h>
#include <math.h>

// define constants
#define MSD2D_VERSION 0.04
#define nb_max_link   4000
#define nb_max_mass   4000
#define Id_length   20

// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif

#define max(a,b) ( ((a) > (b)) ? (a) : (b) ) 
#define min(a,b) ( ((a) < (b)) ? (a) : (b) )

#ifdef _MSC_VER
#define NEWARR(type,var,size) type *var = new type[size]
#define DELARR(var) delete[] var
#else
#define NEWARR(type,var,size) type var[size]
#define DELARR(var) ((void)0)
#endif


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
	t_float speedY;
	t_float posY;
	t_float posY2;
	t_float forceY;
	t_float out_forceY;
} t_mass;

typedef struct _link {
	t_symbol *Id;
	t_int nbr;
	t_mass *mass1;
	t_mass *mass2;
	t_float K1, D1, D2;
	t_float longx, longy, longueur, long_min, long_max;
	t_float distance_old;
} t_link;

class msd2D:
	public flext_base
{
	FLEXT_HEADER_S(msd2D,flext_base,setup)	//class with setup
 
public:
	// constructor with no arguments
	msd2D(int argc,t_atom *argv)
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
//		t_atom sortie;

		for (i=0; i<nb_mass; i++)	{
			delete mass[i];
			}
		for (i=0; i<nb_link; i++)	{
			delete link[i];
			}
		ToOutAnything(1,S_Reset,0,NULL);
		nb_link = 0;
		nb_mass = 0;
		id_mass = 0;
		id_link = 0;
	}

// --------------------------------------------------------------  COMPUTE 
// -----------------------------------------------------------------------

	void m_bang()
	{
		t_float F=0,Fx=0,Fy=0,distance,vitesse, X_new, Y_new;
		t_int i;
		struct _mass mass_1, mass_2;
	
		for (i=0; i<nb_link; i++)	{
		// compute link forces
			distance = sqrt(pow(link[i]->mass1->posX-link[i]->mass2->posX,2) + 
				pow(link[i]->mass1->posY-link[i]->mass2->posY,2));		// L[n] = sqrt( (x1-x2)² +(y1-y2)²)
			if (distance < link[i]->long_min || distance > link[i]->long_max)	{	
				Fx = 0;
				Fy = 0;
			}
			else	{
				F  = link[i]->K1 * (distance - link[i]->longueur) ;		// F[n] = k1 (L[n] - L[0])
				F += link[i]->D1 * (distance - link[i]->distance_old) ;		// F[n] = F[n] + D1 (L[n] - L[n-1])
				if (distance != 0) 	{
					Fx = F * (link[i]->mass1->posX - link[i]->mass2->posX)/distance; // Fx = F * Lx[n]/L[n]
					Fy = F * (link[i]->mass1->posY - link[i]->mass2->posY)/distance; // Fy = F * Ly[n]/L[n]
				}
			}
			link[i]->mass1->forceX -= Fx;					// Fx1[n] = -Fx
			link[i]->mass1->forceX -= link[i]->D2*link[i]->mass1->speedX;	// Fx1[n] = Fx1[n] - D2 * vx1[n-1]
			link[i]->mass2->forceX += Fx;					// Fx2[n] = Fx
			link[i]->mass2->forceX -= link[i]->D2*link[i]->mass2->speedX;	// Fx2[n] = Fx2[n] - D2 * vx2[n-1]
			link[i]->mass1->forceY -= Fy;					// Fy1[n] = -Fy
			link[i]->mass1->forceY -= link[i]->D2*link[i]->mass1->speedY;	// Fy1[n] = Fy1[n] - D2 * vy1[n-1]
			link[i]->mass2->forceY += Fy;					// Fy2[n] = Fy
			link[i]->mass2->forceY -= link[i]->D2*link[i]->mass2->speedY;	// Fy1[n] = Fy1[n] - D2 * vy1[n-1]
			link[i]->distance_old = distance;				// L[n-1] = L[n]
		}

		for (i=0; i<nb_mass; i++)
		// compute new masses position only if mobile = 1
			if (mass[i]->mobile == 1)  		{
				X_new = mass[i]->forceX * mass[i]->invM + 2*mass[i]->posX - mass[i]->posX2; // x[n] =Fx[n]/M+2x[n]-x[n-1] 
				mass[i]->posX2 = mass[i]->posX;				// x[n-2] = x[n-1]
				mass[i]->posX = max(min(X_new,Xmax),Xmin);		// x[n-1] = x[n]
				mass[i]->speedX = mass[i]->posX - mass[i]->posX2;	// vx[n] = x[n] - x[n-1]
				Y_new = mass[i]->forceY * mass[i]->invM + 2*mass[i]->posY - mass[i]->posY2; // x[n] =Fx[n]/M+2x[n]-x[n-1]
				mass[i]->posY2 = mass[i]->posY;				// x[n-2] = x[n-1]
				mass[i]->posY = max(min(Y_new,Ymax),Ymin);		// x[n-1] = x[n]
				mass[i]->speedY = mass[i]->posY - mass[i]->posY2;	// vx[n] = x[n] - x[n-1]
				}

		for (i=0; i<nb_mass; i++)	{
		// clear forces
			mass[i]->out_forceX = mass[i]->forceX;
			mass[i]->forceX = 0;						// Fx[n] = 0
			mass[i]->out_forceY = mass[i]->forceY;
			mass[i]->forceY = 0;						// Fy[n] = 0
		}
	}

// --------------------------------------------------------------  MASSES
// ----------------------------------------------------------------------

	void m_mass(int argc,t_atom *argv) 
	// add a mass
	// Id, nbr, mobile, invM, speedX Y, posX Y, forceX Y
	{
		t_atom sortie[6], aux[2];
		t_float M;

		if (argc != 5)
			error("mass : Id mobile mass X Y");

		mass[nb_mass] = new t_mass;			// new mass
		mass[nb_mass]->Id = GetSymbol(argv[0]);		// ID
		mass[nb_mass]->mobile = GetInt(argv[1]);	// mobile
		if (GetFloat(argv[2])==0)
			M=1;
		else M = GetFloat(argv[2]);			
		mass[nb_mass]->invM = 1/(M);			// invM
		mass[nb_mass]->speedX = 0;			// vx[n]
		mass[nb_mass]->posX = GetFloat(argv[3]);	// x[n]
		mass[nb_mass]->posX2 = GetFloat(argv[3]);	// x[n-1]
		mass[nb_mass]->forceX = 0;			// Fx[n]
		mass[nb_mass]->speedY = 0;			// vy[n]
		mass[nb_mass]->posY = GetFloat(argv[4]);	// y[n]
		mass[nb_mass]->posY2 = GetFloat(argv[4]);	// y[n-1]
		mass[nb_mass]->forceY = 0;			// Fy[n]
		mass[nb_mass]->nbr = id_mass;			// id number
		nb_mass++ ;
		id_mass++;
		nb_mass = min ( nb_max_mass -1, nb_mass );
		SetFloat((sortie[0]),id_mass-1);
		SetSymbol((sortie[1]),GetSymbol(argv[0]));
		SetFloat((sortie[2]),mass[nb_mass-1]->mobile);
		SetFloat((sortie[3]),M);
		SetFloat((sortie[4]),mass[nb_mass-1]->posX);
		SetFloat((sortie[5]),mass[nb_mass-1]->posY);
		ToOutAnything(1,S_Mass,6,sortie);
	}

	void m_forceX(int argc,t_atom *argv) 
	{
	// add a force to mass(es) named Id
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		if (argc != 2)
			error("forceX : Idmass value");

		for (i=0; i<nb_mass;i++)
		{
			if (sym == mass[i]->Id)
				mass[i]->forceX += GetFloat(argv[1]);
		}
	}

	void m_forceY(int argc,t_atom *argv) 
	{
	// add a force to mass(es) named Id
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		if (argc != 2)
			error("forceY : Idmass value");

		for (i=0; i<nb_mass;i++)
		{
			if (sym == mass[i]->Id)
				mass[i]->forceY += GetFloat(argv[1]);
		}
	}

	void m_posX(int argc,t_atom *argv) 
	{
	// displace mass(es) named Id to a certain position
		t_int i;
		const t_symbol *sym = GetASymbol(argv[0]);

		if (argc != 2)
			error("posX : Id/Nomass value");

		if (GetFloat(argv[1]) < Xmax && GetFloat(argv[1]) > Xmin)
			if (sym ==0)	
				for (i=0; i<nb_mass;i++)
				{
					if (GetInt(argv[0]) == mass[i]->nbr)	{
						mass[i]->posX = GetFloat(argv[1]);
						break;
					}
				}
			else
				for (i=0; i<nb_mass;i++)
				{
					if (sym == mass[i]->Id)
						mass[i]->posX = GetFloat(argv[1]);
				}
	}

	void m_posY(int argc,t_atom *argv) 
	{
	// displace mass(es) named Id to a certain position
		t_int i;
		const t_symbol *sym = GetASymbol(argv[0]);

		if (argc != 2)
			error("posY : Id/Nomass value");

		if (GetFloat(argv[1]) < Ymax && GetFloat(argv[1]) > Ymin)
			if (sym ==0)	
				for (i=0; i<nb_mass;i++)
				{
					if (GetInt(argv[0]) == mass[i]->nbr)	{
						mass[i]->posY = GetFloat(argv[1]);
						break;
					}
				}
			else
				for (i=0; i<nb_mass;i++)
				{
					if (sym == mass[i]->Id)
						mass[i]->posY = GetFloat(argv[1]);
				}
	}

	void m_set_mobile(int argc,t_atom *argv) 
	{
	// set mass No to mobile
		t_int i,aux;
	
		if (argc != 1)
			error("setMobile : Idmass");
		
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
		
		if (argc != 1)
			error("setFixed : Idmass");
		
		aux = GetAInt(argv[0]);	
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
		t_atom sortie[6];
		NEWARR(t_atom,aux,nb_link);

		if (argc != 1)
			error("deleteMass : Nomass");
	
		// Delete associated links
		for (i=0; i<nb_link;i++)	{
			if (link[i]->mass1->nbr == GetInt(argv[0]) || link[i]->mass2->nbr == GetInt(argv[0]))	{
				SetFloat((aux[nb_link_delete]),link[i]->nbr);
				nb_link_delete++;
			}
		}

		for (i=0; i<nb_link_delete;i++)
			m_delete_link(1,&aux[i]);

		// delete mass
		for (i=0; i<nb_mass;i++)
			if (mass[i]->nbr == GetAInt(argv[0]))	{
				SetFloat((sortie[0]),mass[i]->nbr);
				SetSymbol((sortie[1]),mass[i]->Id);
				SetFloat((sortie[2]),mass[i]->mobile);
				SetFloat((sortie[3]),1/mass[i]->invM);
				SetFloat((sortie[4]),mass[i]->posX);
				SetFloat((sortie[5]),mass[i]->posY);
				delete mass[i];
				mass[i] = mass[nb_mass-1];
				nb_mass--;
				ToOutAnything(1,S_Mass_deleted,6,sortie);
				break;
			}
		DELARR(aux);
	}


	void m_Xmax(int argc,t_atom *argv) 
	{
	// set X max
		if (argc != 1)
			error("Xmax : Value");
		Xmax = GetFloat(argv[0]);
	}

	void m_Xmin(int argc,t_atom *argv) 
	{
	// set X min
		if (argc != 1)
			error("Xmin : Value");
		Xmin = GetFloat(argv[0]);
	}
	void m_Ymax(int argc,t_atom *argv) 
	{
	// set Y max
		if (argc != 1)
			error("Ymax : Value");
		Ymax = GetFloat(argv[0]);
	}

	void m_Ymin(int argc,t_atom *argv) 
	{
	// set Y min
		if (argc != 1)
			error("Ymin : Value");
		Ymin = GetFloat(argv[0]);
	}

// --------------------------------------------------------------  LINKS 
// ---------------------------------------------------------------------

	void m_link(int argc,t_atom *argv) 
	// add a link
	// Id, *mass1, *mass2, K1, D1, D2, (Lmin,Lmax)
	{
		t_atom sortie[7], aux[2];
		t_int i;
        	t_mass *mass1 = NULL;
        	t_mass *mass2 = NULL;
        	
        	if (argc < 6 || argc > 8)
        	    error("link : Id Nomass1 Nomass2 K D1 D2 (Lmin Lmax)");
			
        	// check for existence of link masses:
        	for (i=0; i<nb_mass;i++)
				if (mass[i]->nbr==GetInt(argv[1]))	// pointer to mass1
					// we found mass1
        	        mass1 = mass[i];
				else if (mass[i]->nbr==GetInt(argv[2]))	// pointer to mass2
					// ... and mass2
        	        mass2 = mass[i];

        	if (mass1 and mass2)
        	{
			link[nb_link] = new t_link;			// New pointer
			link[nb_link]->Id = GetSymbol(argv[0]);		// ID
			link[nb_link]->mass1 = mass1; // pointer to mass1
			link[nb_link]->mass2 = mass2; // pointer to mass2
			link[nb_link]->K1 = GetFloat(argv[3]);		// K1
			link[nb_link]->D1 = GetFloat(argv[4]);		// D1
			link[nb_link]->D2 = GetFloat(argv[5]);		// D2
			link[nb_link]->longx = link[nb_link]->mass1->posX - link[nb_link]->mass2->posX;	// Lx[0]
			link[nb_link]->longy = link[nb_link]->mass1->posY - link[nb_link]->mass2->posY;	// Ly[0]
			link[nb_link]->longueur = sqrt(pow(link[nb_link]->longx,2)+pow(link[nb_link]->longy,2)); // L[0]
			link[nb_link]->nbr = id_link;			// id number
			link[nb_link]->distance_old = link[nb_link]->longueur;	// L[n-1]
			switch (argc)	{
				case 6 :	
					link[nb_link]->long_max = 32768;
					link[nb_link]->long_min = 0;
					break;
				case 7 :	
					link[nb_link]->long_min = GetFloat(argv[6]);
					link[nb_link]->long_max = 32768;
					break;
				case 8 :	
					link[nb_link]->long_min = GetFloat(argv[6]);
					link[nb_link]->long_max = GetFloat(argv[7]);
					break;	
			}
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
        	else
            		error("link : Cannot create link: Not all masses for this link have been created yet.");
	}

	void m_ilink(int argc,t_atom *argv) 
	// add interactor link
	// Id, Id masses1, Id masses2, K1, D1, D2, (Lmin, Lmax)
	{
		t_atom aux[2], arglist[8];
		t_int i,j, nbmass1=0, nbmass2=0;
		NEWARR(t_int,imass1,nb_mass);
		NEWARR(t_int,imass2,nb_mass);
		t_symbol *Id1, *Id2;

		if (argc < 6 || argc > 8)
			error("ilink : Id Idmass1 Idmass2 K D1 D2 (Lmin Lmax)");

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
					switch (argc)	{
						case 7 :	
							SetFloat(arglist[6],GetFloat(argv[6]));
							break;
						case 8 :	
							SetFloat(arglist[6],GetFloat(argv[6]));
							SetFloat(arglist[7],GetFloat(argv[7]));
							break;	
					}
					m_link(argc,arglist);
				}
		DELARR(imass1);
		DELARR(imass2);
	}

	void m_setK(int argc,t_atom *argv) 
	{
	// set rigidity of link(s) named Id
		t_int i;
		const t_symbol *sym = GetSymbol(argv[0]);

		if (argc != 2)
			error("setK : IdLink Value");

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

		if (argc != 2)
			error("setD : IdLink Value");

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

		if (argc != 2)
			error("setD2 : IdLink Value");

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

		if (argc != 1)
			error("deleteLink : NoLink");

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
		t_atom sortie[5];
		auxtype = GetSymbol(argv[0]);
		auxarg = GetASymbol(argv[1]);		//auxarg : & symbol, 0 else

		if (argc == 1)
		{
			if (auxtype == S_massesPos)		// get all masses positions
				for (i=0; i<nb_mass; i++)
				{
					SetFloat(sortie[0],mass[i]->nbr);
					SetFloat(sortie[1],mass[i]->posX);
					SetFloat(sortie[2],mass[i]->posY);
					ToOutAnything(0,S_massesPos,3,sortie);
				}
			else if (auxtype == S_massesForces)	// get all masses forces
				for (i=0; i<nb_mass; i++)
				{
					SetFloat(sortie[0],mass[i]->nbr);
					SetFloat(sortie[1],mass[i]->out_forceX);
					SetFloat(sortie[2],mass[i]->out_forceY);
					ToOutAnything(0,S_massesForces,3,sortie);
				}
			else if (auxtype == S_linksPos)		// get all links positions
				for (i=0; i<nb_link; i++)
				{
					SetFloat(sortie[0],link[i]->nbr);
					SetFloat(sortie[1],link[i]->mass1->posX);
					SetFloat(sortie[2],link[i]->mass1->posY);
					SetFloat(sortie[3],link[i]->mass2->posX);
					SetFloat(sortie[4],link[i]->mass2->posY);
					ToOutAnything(0,S_linksPos,5,sortie);
				}
			else 					// get all masses speeds
				for (i=0; i<nb_mass; i++)
				{
					SetFloat(sortie[0],mass[i]->nbr);
					SetFloat(sortie[1],mass[i]->speedX);
					SetFloat(sortie[2],mass[i]->speedY);
					ToOutAnything(0,S_massesSpeeds,3,sortie);
				}
		}
		else if (auxtype == S_massesPos) 		// get mass positions
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],mass[i]->nbr);
							SetFloat(sortie[1],mass[i]->posX);
							SetFloat(sortie[2],mass[i]->posY);
							ToOutAnything(0,S_massesPosNo,3,sortie);
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
							SetSymbol(sortie[0],mass[i]->Id);
							SetFloat(sortie[1],mass[i]->posX);
							SetFloat(sortie[2],mass[i]->posY);
							ToOutAnything(0,S_massesPosId,3,sortie);
						}
					}
				}
			}
		}
		else if (auxtype == S_massesForces)		// get mass forces
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],mass[i]->nbr);
							SetFloat(sortie[1],mass[i]->out_forceX);
							SetFloat(sortie[2],mass[i]->out_forceY);
							ToOutAnything(0,S_massesForcesNo,3,sortie);
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
							SetSymbol(sortie[0],mass[i]->Id);
							SetFloat(sortie[1],mass[i]->out_forceX);
							SetFloat(sortie[2],mass[i]->out_forceY);
							ToOutAnything(0,S_massesForcesId,3,sortie);
						}
					}
				}
			}
		}
		else if (auxtype == S_linksPos)			// get links positions
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_link;i++)
						if (link[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],link[i]->nbr);
							SetFloat(sortie[1],link[i]->mass1->posX);
							SetFloat(sortie[2],link[i]->mass1->posY);
							SetFloat(sortie[3],link[i]->mass2->posX);
							SetFloat(sortie[4],link[i]->mass2->posY);
							ToOutAnything(0,S_linksPosNo,5,sortie);
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
							SetSymbol(sortie[0],link[i]->Id);
							SetFloat(sortie[1],link[i]->mass1->posX);
							SetFloat(sortie[2],link[i]->mass1->posY);
							SetFloat(sortie[3],link[i]->mass2->posX);
							SetFloat(sortie[4],link[i]->mass2->posY);
							ToOutAnything(0,S_linksPosId,5,sortie);
						}
					}
				}
			}
		}
		else 			 			// get mass speeds
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetInt(argv[j]))
						{
							SetFloat(sortie[0],mass[i]->nbr);
							SetFloat(sortie[1],mass[i]->speedX);
							SetFloat(sortie[2],mass[i]->speedY);
							ToOutAnything(0,S_massesSpeedsNo,3,sortie);
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
							SetSymbol(sortie[0],mass[i]->Id);
							SetFloat(sortie[1],mass[i]->speedX);
							SetFloat(sortie[2],mass[i]->speedY);
							ToOutAnything(0,S_massesSpeedsId,3,sortie);
						}
					}
				}
			}
		}
		

	}

	void m_mass_dumpl()
	// List of masses positions on first outlet
	{	
		NEWARR(t_atom,sortie,2*nb_mass);
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SetFloat((sortie[2*i]),mass[i]->posX);
			SetFloat((sortie[2*i+1]),mass[i]->posY);
		}
		ToOutAnything(0, S_massesPosL, 2*nb_mass, sortie);
		DELARR(sortie);
	}

	void m_force_dumpl()
	// List of masses forces on first outlet
	{	
		NEWARR(t_atom,sortie,2*nb_mass);
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SetFloat((sortie[2*i]),mass[i]->out_forceX);
			SetFloat((sortie[2*i+1]),mass[i]->out_forceY);
		}
		ToOutAnything(0, S_massesForcesL, 2*nb_mass, sortie);
		DELARR(sortie);
	}

	void m_info_dumpl()
	// List of infos on masses and links on  first outlet
	{	
		t_atom sortie[7];
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SetFloat((sortie[0]),mass[i]->nbr);
			SetSymbol((sortie[1]),mass[i]->Id);
			SetFloat((sortie[2]),mass[i]->mobile);
			SetFloat((sortie[3]),1/mass[i]->invM);
			SetFloat((sortie[4]),mass[i]->posX);
			SetFloat((sortie[5]),mass[i]->posY);
		ToOutAnything(1, S_Mass, 6, sortie);
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

// --------------------------------------------------------------  GLOBAL VARIABLES 
// --------------------------------------------------------------------------------

	t_link * link[nb_max_link];
	t_mass * mass[nb_max_mass];
	t_float Xmin, Xmax, Ymin, Ymax;
	int nb_link, nb_mass, id_mass, id_link;

// --------------------------------------------------------------  SETUP
// ---------------------------------------------------------------------

private:

	// static symbols
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

		// --- set up methods (class scope) ---

		// register a bang method to the default inlet (0)
		FLEXT_CADDBANG(c,0,m_bang);

		// set up tagged methods for the default inlet (0)
		// the underscore _ after CADDMETHOD indicates that a message tag is used
		// no, variable list or anything and all single arguments are recognized automatically, ...
		FLEXT_CADDMETHOD_(c,0,"reset",m_reset);
		FLEXT_CADDMETHOD_(c,0,"forceX",m_forceX);
		FLEXT_CADDMETHOD_(c,0,"forceY",m_forceY);
		FLEXT_CADDMETHOD_(c,0,"posX",m_posX);
		FLEXT_CADDMETHOD_(c,0,"Xmax",m_Xmax);
		FLEXT_CADDMETHOD_(c,0,"Xmin",m_Xmin);
		FLEXT_CADDMETHOD_(c,0,"Ymax",m_Ymax);
		FLEXT_CADDMETHOD_(c,0,"Ymin",m_Ymin);
		FLEXT_CADDMETHOD_(c,0,"posY",m_posY);
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
		FLEXT_CADDMETHOD_(c,0,"setMobile",m_set_mobile);
		FLEXT_CADDMETHOD_(c,0,"setFixed",m_set_fixe);
	}

	// for every registered method a callback has to be declared
	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK(m_mass_dumpl)
	FLEXT_CALLBACK(m_force_dumpl)
	FLEXT_CALLBACK(m_info_dumpl)
	FLEXT_CALLBACK(m_reset)
	FLEXT_CALLBACK_V(m_mass)
	FLEXT_CALLBACK_V(m_link)
	FLEXT_CALLBACK_V(m_ilink)
	FLEXT_CALLBACK_V(m_set_mobile)
	FLEXT_CALLBACK_V(m_set_fixe)
	FLEXT_CALLBACK_V(m_Xmax)
	FLEXT_CALLBACK_V(m_Xmin)
	FLEXT_CALLBACK_V(m_Ymax)
	FLEXT_CALLBACK_V(m_Ymin)
	FLEXT_CALLBACK_V(m_setK)
	FLEXT_CALLBACK_V(m_setD)
	FLEXT_CALLBACK_V(m_setD2)
	FLEXT_CALLBACK_V(m_forceX)
	FLEXT_CALLBACK_V(m_forceY)
	FLEXT_CALLBACK_V(m_posX)
	FLEXT_CALLBACK_V(m_posY)
	FLEXT_CALLBACK_V(m_get)
	FLEXT_CALLBACK_V(m_delete_link)
	FLEXT_CALLBACK_V(m_delete_mass)
};

	const t_symbol *msd2D::S_Reset = MakeSymbol("Reset");
	const t_symbol *msd2D::S_Mass = MakeSymbol("Mass");
	const t_symbol *msd2D::S_Link = MakeSymbol("Link");
	const t_symbol *msd2D::S_iLink = MakeSymbol("iLink");
	const t_symbol *msd2D::S_Mass_deleted = MakeSymbol("Mass deleted");
	const t_symbol *msd2D::S_Link_deleted = MakeSymbol("Link deleted");
	const t_symbol *msd2D::S_massesPos = MakeSymbol("massesPos");
	const t_symbol *msd2D::S_massesPosNo = MakeSymbol("massesPosNo");
	const t_symbol *msd2D::S_massesPosId = MakeSymbol("massesPosId");
	const t_symbol *msd2D::S_linksPos = MakeSymbol("linksPos");
	const t_symbol *msd2D::S_linksPosNo = MakeSymbol("linksPosNo");
	const t_symbol *msd2D::S_linksPosId = MakeSymbol("linksPosId");
	const t_symbol *msd2D::S_massesForces = MakeSymbol("massesForces");
	const t_symbol *msd2D::S_massesForcesNo = MakeSymbol("massesForcesNo");
	const t_symbol *msd2D::S_massesForcesId = MakeSymbol("massesForcesId");
	const t_symbol *msd2D::S_massesSpeeds = MakeSymbol("massesSpeeds");
	const t_symbol *msd2D::S_massesSpeedsNo = MakeSymbol("massesSpeedsNo");
	const t_symbol *msd2D::S_massesSpeedsId = MakeSymbol("massesSpeedsId");
	const t_symbol *msd2D::S_massesPosL = MakeSymbol("massesPosL");
	const t_symbol *msd2D::S_massesForcesL = MakeSymbol("massesForcesL");

// instantiate the class (constructor has a variable argument list)
FLEXT_NEW_V("msd2D",msd2D)


