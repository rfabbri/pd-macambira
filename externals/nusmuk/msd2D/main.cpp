

/* 
 msd2D - mass spring damper model for Pure Data or Max/MSP

 Written by Nicolas Montgermont for a Master's train in Acoustic,
 Signal processing and Computing Applied to Music (ATIAM, Paris 6) 
 at La Kitchen supervised by Cyrille Henry.

 Based on Pure Data by Miller Puckette and others
 Use FLEXT C++ Layer by Thomas Grill (xovo@gmx.net)
 Based on pmpd by Cyrille Henry 


 Contact : Nicolas Montgermont, montgermont@la-kitchen.fr
	   Cyrille Henry, Cyrille.Henry@la-kitchen.fr

 This program is free software; you can redistribute it and/or                
 modify it under the terms of the GNU General Public License                  
 as published by the Free Software Foundation; either version 2               
 of the License, or (at your option) any later version.                       
                                                                             
 This program is distributed in the hope that it will be useful,              
 but WITHOUT ANY WARRANTY; without even the implied warranty of               
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                
 GNU General Public License for more details.                           
                                                                              
 You should have received a copy of the GNU General Public License           
 along with this program; if not, write to the Free Software                  
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 Version 0.01, 12.04.2005

*/

// include flext header
#include <flext.h>

#include <math.h>

// define constants
#define MSD2D_VERSION 0.01
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
	t_float speedY;
	t_float posY;
	t_float posY2;
	t_float forceY;
	t_float out_forceY;
	char Id_string[Id_length];
} t_mass;

typedef struct _link {
	t_symbol *Id;
	t_int nbr;
	t_mass *mass1;
	t_mass *mass2;
	t_float K1, D1, D2;
	t_float longx, longy, longueur;
	t_float distance_old;
	char Id_string[Id_length];
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
		ToOutAnything(1,gensym("Reset"),0,sortie);
		nb_link = 0;
		nb_mass = 0;
		id_mass = 0;
		id_link = 0;
	}

// --------------------------------------------------------------  COMPUTE 

	void m_bang()
	{
		t_float F=0,Fx=0,Fy=0,distance,vitesse, X_new, Y_new;
		t_int i;
		struct _mass mass_1, mass_2;
	
		for (i=0; i<nb_link; i++)	{
		// compute link forces
			distance = sqrt(pow(link[i]->mass1->posX-link[i]->mass2->posX,2) + 
				pow(link[i]->mass1->posY-link[i]->mass2->posY,2));
			F  = link[i]->K1 * (distance - link[i]->longueur) ;	// F = k1(x1-x2)
			F += link[i]->D1 * (distance - link[i]->distance_old) ;	// F = F + D1(v1-v2)
			if (distance != 0) 	{
				Fx = F * (link[i]->mass1->posX - link[i]->mass2->posX)/distance;
				Fy = F * (link[i]->mass1->posY - link[i]->mass2->posY)/distance;
			}
			link[i]->mass1->forceX -= Fx;
			link[i]->mass1->forceX -= link[i]->D2*link[i]->mass1->speedX;
			link[i]->mass2->forceX += Fx;
			link[i]->mass2->forceX -= link[i]->D2*link[i]->mass2->speedX;
			link[i]->mass1->forceY -= Fy;
			link[i]->mass1->forceY -= link[i]->D2*link[i]->mass1->speedY;
			link[i]->mass2->forceY += Fy;
			link[i]->mass2->forceY -= link[i]->D2*link[i]->mass2->speedY;
			link[i]->distance_old = distance;
		}

		for (i=0; i<nb_mass; i++)
		// compute new masses position only if mobile = 1
			if (mass[i]->mobile == 1)  		{
				X_new = mass[i]->forceX * mass[i]->invM + 2*mass[i]->posX - mass[i]->posX2;
				mass[i]->posX2 = mass[i]->posX;
				mass[i]->posX = max(min(X_new,Xmax),Xmin);			// x = x + v
				mass[i]->speedX = mass[i]->posX - mass[i]->posX2;	// v = v + F/M
				Y_new = mass[i]->forceY * mass[i]->invM + 2*mass[i]->posY - mass[i]->posY2;
				mass[i]->posY2 = mass[i]->posY;
				mass[i]->posY = max(min(Y_new,Ymax),Ymin);			// x = x + v
				mass[i]->speedY = mass[i]->posY - mass[i]->posY2;	// v = v + F/M
				}

		for (i=0; i<nb_mass; i++)	{
		// clear forces
			mass[i]->out_forceX = mass[i]->forceX;
			mass[i]->forceX = 0;
			mass[i]->out_forceY = mass[i]->forceY;
			mass[i]->forceY = 0;
		}
	}

// --------------------------------------------------------------  MASSES

	void m_mass(int argc,t_atom *argv) 
	// add a mass
	// Id, nbr, mobile, invM, speedX, posX, forceX
	{
		t_atom sortie[6], aux[2];
		int M;

		mass[nb_mass] = new t_mass;			// new pointer
		mass[nb_mass]->Id = GetASymbol(argv[0]);	// ID
		mass[nb_mass]->mobile = GetAInt(argv[1]);	// mobile
		if (GetAInt(argv[2])==0)
			M=1;
		else M = GetAInt(argv[2]);			
		mass[nb_mass]->invM = 1/((float)M);		// invM
		mass[nb_mass]->speedX = 0;			// vx[n]
		mass[nb_mass]->posX = GetAInt(argv[3]);		// x[n]
		mass[nb_mass]->posX2 = GetAInt(argv[3]);	// x[n-1]
		mass[nb_mass]->forceX = 0;			// Fx[n]
		mass[nb_mass]->speedY = 0;			// vy[n]
		mass[nb_mass]->posY = GetAInt(argv[4]);		// y[n]
		mass[nb_mass]->posY2 = GetAInt(argv[4]);	// y[n-1]
		mass[nb_mass]->forceY = 0;			// Fy[n]
		mass[nb_mass]->nbr = id_mass;			// id_nbr
		SETSYMBOL(aux,GetASymbol(argv[0]));
		atom_string(aux,mass[nb_mass]->Id_string,Id_length);
		nb_mass++ ;
		id_mass++;
		nb_mass = min ( nb_max_mass -1, nb_mass );
		SETFLOAT(&(sortie[0]),id_mass-1);
		SETSYMBOL(&(sortie[1]),GetASymbol(argv[0]));
		SETFLOAT(&(sortie[2]),mass[nb_mass-1]->mobile);
		SETFLOAT(&(sortie[3]),M);
		SETFLOAT(&(sortie[4]),mass[nb_mass-1]->posX);
		SETFLOAT(&(sortie[5]),mass[nb_mass-1]->posY);
		ToOutAnything(1,gensym("Mass"),6,sortie);
	}

	void m_forceX(int argc,t_atom *argv) 
	{
	// add a force to mass(es) named Id
		t_int i,aux;
		t_atom atom[2];
		char buffer[Id_length];

		SETSYMBOL(atom,GetASymbol(argv[0]));
		atom_string(atom, buffer, Id_length);
		for (i=0; i<nb_mass;i++)
		{
			aux = strcmp(buffer,mass[i]->Id_string);
			if (aux == 0)
				mass[i]->forceX += GetAFloat(argv[1]);
		}
	}

	void m_forceY(int argc,t_atom *argv) 
	{
	// add a force to mass(es) named Id
		t_int i,aux;
		t_atom atom[2];
		char buffer[Id_length];

		SETSYMBOL(atom,GetASymbol(argv[0]));
		atom_string(atom, buffer, Id_length);
		for (i=0; i<nb_mass;i++)
		{
			aux = strcmp(buffer,mass[i]->Id_string);
			if (aux == 0)
				mass[i]->forceY += GetAFloat(argv[1]);
		}
	}

	void m_posX(int argc,t_atom *argv) 
	{
	// displace mass(es) named Id to a certain position
		t_int i,aux;
		t_atom atom[2];
		char buffer[Id_length];

		if (GetAFloat(argv[1]) < Xmax && GetAFloat(argv[1]) > Xmin)
		{
			SETSYMBOL(atom,GetASymbol(argv[0]));
			atom_string(atom, buffer, Id_length);
			for (i=0; i<nb_mass;i++)
			{
				aux = strcmp(buffer,mass[i]->Id_string);
				if (aux == 0)
					mass[i]->posX = GetAFloat(argv[1]);
			}
		}
	}

	void m_posY(int argc,t_atom *argv) 
	{
	// displace mass(es) named Id to a certain position
		t_int i,aux;
		t_atom atom[2];
		char buffer[Id_length];

		if (GetAFloat(argv[1]) < Ymax && GetAFloat(argv[1]) > Ymin)
		{
			SETSYMBOL(atom,GetASymbol(argv[0]));
			atom_string(atom, buffer, Id_length);
			for (i=0; i<nb_mass;i++)
			{
				aux = strcmp(buffer,mass[i]->Id_string);
				if (aux == 0)
					mass[i]->posY = GetAFloat(argv[1]);
			}
		}
	}

	void m_set_mobile(int argc,t_atom *argv) 
	{
	// set mass No to mobile
		t_int i,aux;
		
		aux = GetAInt(argv[0]);	
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
	t_atom sortie[6], aux[nb_link];
	
	for (i=0; i<nb_link;i++)	{
		if (link[i]->mass1->nbr == GetAInt(argv[0]) || link[i]->mass2->nbr == GetAInt(argv[0]))	{
			SETFLOAT(&(aux[nb_link_delete]),link[i]->nbr);
			nb_link_delete++;
		}
	}

	for (i=0; i<nb_link_delete;i++)
		m_delete_link(1,&aux[i]);


	for (i=0; i<nb_mass;i++)
		if (mass[i]->nbr == GetAInt(argv[0]))	{
			SETFLOAT(&(sortie[0]),mass[i]->nbr);
			SETSYMBOL(&(sortie[1]),mass[i]->Id);
			SETFLOAT(&(sortie[2]),mass[i]->mobile);
			SETFLOAT(&(sortie[3]),1/mass[i]->invM);
			SETFLOAT(&(sortie[4]),mass[i]->posX);
			SETFLOAT(&(sortie[5]),mass[i]->posY);
			delete mass[i];
			mass[i] = mass[nb_mass-1];
			nb_mass--;
			ToOutAnything(1,gensym("Mass deleted"),6,sortie);
			break;
		}
	}


	void m_Xmax(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
	Xmax = GetAFloat(argv[0]);
	}

	void m_Ymax(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
	Ymax = GetAFloat(argv[0]);
	}

	void m_Xmin(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
	Xmin = GetAFloat(argv[0]);
	}

	void m_Ymin(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
	Ymin = GetAFloat(argv[0]);
	}

// --------------------------------------------------------------  LINKS 

	void m_link(int argc,t_atom *argv) 
	// add a link
	// Id, nbr, *mass1, *mass2, K1, D1
	{
		t_atom sortie[7], aux[2];
		t_int i;

		link[nb_link] = new t_link;
		link[nb_link]->Id = GetASymbol(argv[0]);
		for (i=0; i<nb_mass;i++)
			if (mass[i]->nbr==GetAInt(argv[1]))
				link[nb_link]->mass1 = mass[i];
			else if(mass[i]->nbr==GetAInt(argv[2]))
				link[nb_link]->mass2 = mass[i];
		link[nb_link]->K1 = GetAFloat(argv[3]);
		link[nb_link]->D1 = GetAFloat(argv[4]);	
		link[nb_link]->D2 = GetAFloat(argv[5]);
		link[nb_link]->longx = link[nb_link]->mass1->posX - link[nb_link]->mass2->posX;
		link[nb_link]->longy = link[nb_link]->mass1->posY - link[nb_link]->mass2->posY;
		link[nb_link]->longueur = sqrt( pow(link[nb_link]->longx,2) + pow(link[nb_link]->longy,2));
		link[nb_link]->nbr = id_link;
		link[nb_link]->distance_old = link[nb_link]->longueur;
		SETSYMBOL(aux,GetASymbol(argv[0]));
		atom_string(aux,(link[nb_link]->Id_string),Id_length);
		nb_link++;
		id_link++;
		nb_link = min ( nb_max_link -1, nb_link );
		SETFLOAT(&(sortie[0]),id_link-1);
		SETSYMBOL(&(sortie[1]),link[nb_link-1]->Id);
		SETFLOAT(&(sortie[2]),GetAInt(argv[1]));
		SETFLOAT(&(sortie[3]),GetAInt(argv[2]));
		SETFLOAT(&(sortie[4]),link[nb_link-1]->K1);
		SETFLOAT(&(sortie[5]),link[nb_link-1]->D1);
		SETFLOAT(&(sortie[6]),link[nb_link-1]->D2);
		ToOutAnything(1,gensym("Link"),7,sortie);
	}

	void m_ilink(int argc,t_atom *argv) 
	// add interactor link
	// Id, nbr, Id masses1, Id masses2, K1, D1
	{
		t_atom aux[2], arglist[6];
		t_int i,j, strvalue, strvalue2, imass1[nb_mass], nbmass1=0, imass2[nb_mass], nbmass2=0;
		char buffer[Id_length], buffer2[Id_length];

		ToOutAnything(1,gensym("iLink"),0,aux);
		SETSYMBOL(aux,GetASymbol(argv[1]));
		atom_string(aux, buffer, Id_length);
		SETSYMBOL(aux,GetASymbol(argv[2]));
		atom_string(aux, buffer2, Id_length);

		for (i=0;i<nb_mass;i++)	{
			strvalue=strcmp(buffer,mass[i]->Id_string);
			strvalue2=strcmp(buffer2,mass[i]->Id_string);
			if (strvalue ==0)	{
				imass1[nbmass1]=i;
				nbmass1++;
			}
			if (strvalue2 ==0)	{
				imass2[nbmass2]=i;
				nbmass2++;
			}
		}
		
		for(i=0;i<nbmass1;i++)
			for(j=0;j<nbmass2;j++)	
				if (imass1[i] != imass2[j])	{
					SETSYMBOL(&(arglist[0]),GetASymbol(argv[0]));
					SETFLOAT(&(arglist[1]),mass[imass1[i]]->nbr);
					SETFLOAT(&(arglist[2]),mass[imass2[j]]->nbr);
					SETFLOAT(&(arglist[3]),GetAInt(argv[3]));
					SETFLOAT(&(arglist[4]),GetAFloat(argv[4]));
					SETFLOAT(&(arglist[5]),GetAFloat(argv[5]));
					m_link(6,arglist);
				}
	}

	void m_setK(int argc,t_atom *argv) 
	{
	// set rigidity of link(s) named Id
		t_int i,aux;
		t_atom atom[2];
		char buffer[Id_length];

		SETSYMBOL(atom,GetASymbol(argv[0]));
		atom_string(atom, buffer, Id_length);
		for (i=0; i<nb_link;i++)
		{
			aux = strcmp(buffer,link[i]->Id_string);
			if (aux == 0)
				link[i]->K1 = GetAFloat(argv[1]);
		}
	}

	void m_setD(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
		t_int i,aux;
		t_atom atom[2];
		char buffer[Id_length];

		SETSYMBOL(atom,GetASymbol(argv[0]));
		atom_string(atom, buffer, Id_length);
		for (i=0; i<nb_link;i++)
		{
			aux = strcmp(buffer,link[i]->Id_string);
			if (aux == 0)
				link[i]->D1 = GetAFloat(argv[1]);
		}
	}

	void m_setD2(int argc,t_atom *argv) 
	{
	// set damping of link(s) named Id
		t_int i,aux;
		t_atom atom[2];
		char buffer[Id_length];

		SETSYMBOL(atom,GetASymbol(argv[0]));
		atom_string(atom, buffer, Id_length);
		for (i=0; i<nb_link;i++)
		{
			aux = strcmp(buffer,link[i]->Id_string);
			if (aux == 0)
				link[i]->D2 = GetAFloat(argv[1]);
		}
	}

	void m_delete_link(int argc,t_atom *argv) 
	{
	// Delete link
	t_int i;
	t_atom sortie[7];

	for (i=0; i<nb_link;i++)
		if (link[i]->nbr == GetAInt(argv[0]))	{
			SETFLOAT(&(sortie[0]),link[i]->nbr);
			SETSYMBOL(&(sortie[1]),link[i]->Id);
			SETFLOAT(&(sortie[2]),link[i]->mass1->nbr);
			SETFLOAT(&(sortie[3]),link[i]->mass2->nbr);
			SETFLOAT(&(sortie[4]),link[i]->K1);
			SETFLOAT(&(sortie[5]),link[i]->D1);
			SETFLOAT(&(sortie[6]),link[i]->D2);
			delete link[i];
			link[i]=link[nb_link-1];
			nb_link--;
			ToOutAnything(1,gensym("Link deleted"),7,sortie);
			break;
		}
	}


// --------------------------------------------------------------  GET 

	void m_get(int argc,t_atom *argv)
	// get attributes
	{
		t_int i,j, auxstring1, auxstring2, auxstring3, aux;
		t_symbol *auxarg;
		t_atom sortie[4],atom[2];
		char buffer[Id_length],masses[]="massesPos",buffer2[Id_length], forces[] = "massesForces", links[] = "linksPos";

		SETSYMBOL(atom,GetASymbol(argv[0]));
		atom_string(atom, buffer, 20);
		auxstring1 = strcmp(buffer,masses);	//auxstring1 : 0 masses, 1 else
		auxstring2 = strcmp(buffer,forces);	//auxstring2 : 0 forces, 1 else
		auxstring3 = strcmp(buffer,links);	//auxstring2 : 0 links, 1 else
		auxarg = GetASymbol(argv[1]);		//auxarg : & symbol, 0 else
		if (argc == 1)
		{
			if (auxstring1 == 0)// get all masses positions
				for (i=0; i<nb_mass; i++)
				{
					SETFLOAT(&sortie[0],mass[i]->posX);
					SETFLOAT(&sortie[1],mass[i]->posY);
					ToOutAnything(0,gensym("massesPos"),2,sortie);
				}
			else if (auxstring2 == 0)// get all masses forces
				for (i=0; i<nb_mass; i++)
				{
					SETFLOAT(&sortie[0],mass[i]->out_forceX);
					SETFLOAT(&sortie[1],mass[i]->out_forceY);
					ToOutAnything(0,gensym("massesForces"),2,sortie);
				}
			else if (auxstring3 == 0)// get all links positions
				for (i=0; i<nb_link; i++)
				{
					SETFLOAT(&sortie[0],link[i]->mass1->posX);
					SETFLOAT(&sortie[1],link[i]->mass1->posY);
					SETFLOAT(&sortie[2],link[i]->mass2->posX);
					SETFLOAT(&sortie[3],link[i]->mass2->posY);
					ToOutAnything(0,gensym("linksPos"),4,sortie);
				}
			else 		// get all masses speeds
				for (i=0; i<nb_mass; i++)
				{
					SETFLOAT(&sortie[0],mass[i]->speedX);
					SETFLOAT(&sortie[1],mass[i]->speedY);
					ToOutAnything(0,gensym("massesSpeeds"),2,sortie);
				}
		}
		else if (auxstring1 == 0) // get mass positions
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetAInt(argv[j]))
						{
							SETFLOAT(&sortie[0],mass[i]->posX);
							SETFLOAT(&sortie[1],mass[i]->posY);
							ToOutAnything(0,gensym("massesPosNo"),2,sortie);
						}
			}
			else 		//string
			{
				for (j = 1; j<argc; j++)
				{
					SETSYMBOL(&atom[1],GetASymbol(argv[j]));
					atom_string(&atom[1], buffer2, Id_length);
					for (i=0;i<nb_mass;i++)	
					{
						
						aux = strcmp(buffer2,mass[i]->Id_string);
						if (aux==0)
						{
							SETFLOAT(&sortie[0],mass[i]->posX);
							SETFLOAT(&sortie[1],mass[i]->posY);
							ToOutAnything(0,gensym("massesPosId"),2,sortie);
						}
					}
				}
			}
		}
		else if (auxstring2 == 0)			 // get mass forces
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_mass;i++)
						if (mass[i]->nbr==GetAInt(argv[j]))
						{
							SETFLOAT(&sortie[0],mass[i]->out_forceX);
							SETFLOAT(&sortie[1],mass[i]->out_forceY);
							ToOutAnything(0,gensym("massesForcesNo"),2,sortie);
						}
			}
			else 		//string
			{
				for (j = 1; j<argc; j++)
				{
					SETSYMBOL(&atom[1],GetASymbol(argv[j]));
					atom_string(&atom[1], buffer2, Id_length);
					for (i=0;i<nb_mass;i++)	
					{
		
						aux = strcmp(buffer2,mass[i]->Id_string);
						if (aux==0)
						{
							SETFLOAT(&sortie[0],mass[i]->out_forceX);
							SETFLOAT(&sortie[1],mass[i]->out_forceY);
							ToOutAnything(0,gensym("massesForcesId"),2,sortie);
						}
					}
				}
			}
		}
		else if (auxstring3 == 0)			 // get links positions
		{
			if (auxarg == 0) // No
			{
				for (j = 1; j<argc; j++)
					for (i=0;i<nb_link;i++)
						if (link[i]->nbr==GetAInt(argv[j]))
						{
							SETFLOAT(&sortie[0],link[i]->mass1->posX);
							SETFLOAT(&sortie[1],link[i]->mass1->posY);
							SETFLOAT(&sortie[2],link[i]->mass2->posX);
							SETFLOAT(&sortie[3],link[i]->mass2->posY);
							ToOutAnything(0,gensym("linksPosNo"),4,sortie);
						}
			}
			else 		//string
			{
				for (j = 1; j<argc; j++)
				{
					SETSYMBOL(&atom[1],GetASymbol(argv[j]));
					atom_string(&atom[1], buffer2, Id_length);
					for (i=0;i<nb_link;i++)	
					{
		
						aux = strcmp(buffer2,link[i]->Id_string);
						if (aux==0)
						{
							SETFLOAT(&sortie[0],link[i]->mass1->posX);
							SETFLOAT(&sortie[1],link[i]->mass1->posY);
							SETFLOAT(&sortie[2],link[i]->mass2->posX);
							SETFLOAT(&sortie[3],link[i]->mass2->posY);
							ToOutAnything(0,gensym("linksPosId"),4,sortie);
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
						if (mass[i]->nbr==GetAInt(argv[j]))
						{
							SETFLOAT(&sortie[0],mass[i]->speedX);
							SETFLOAT(&sortie[1],mass[i]->speedY);
							ToOutAnything(0,gensym("massesSpeedsNo"),2,sortie);
						}
			}
			else 		//string
			{
				for (j = 1; j<argc; j++)
				{
					SETSYMBOL(&atom[1],GetASymbol(argv[j]));
					atom_string(&atom[1], buffer2, Id_length);
					for (i=0;i<nb_mass;i++)	
					{
		
						aux = strcmp(buffer2,mass[i]->Id_string);
						if (aux==0)
						{
							SETFLOAT(&sortie[0],mass[i]->speedX);
							SETFLOAT(&sortie[1],mass[i]->speedY);
							ToOutAnything(0,gensym("massesSpeedsId"),2,sortie);
						}
					}
				}
			}
		}
		

	}

	void m_mass_dumpl()
	// List of masses positions on first outlet
	{	
		t_atom sortie[2*nb_mass];
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SETFLOAT(&(sortie[2*i]),mass[i]->posX);
			SETFLOAT(&(sortie[2*i+1]),mass[i]->posY);
		}
		ToOutAnything(0, gensym("massesPosL"), 2*nb_mass, sortie);
	}

	void m_force_dumpl()
	// List of masses positions on first outlet
	{	
		t_atom sortie[2*nb_mass];
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SETFLOAT(&(sortie[2*i]),mass[i]->out_forceX);
			SETFLOAT(&(sortie[2*i+1]),mass[i]->out_forceY);
		}
		ToOutAnything(0, gensym("massesForcesL"), 2*nb_mass, sortie);
	}

	void m_link_dumpl()
	// List of masses positions on first outlet
	{	
		t_atom sortie[2*nb_link];
		t_int i;
	
		for (i=0; i<nb_link; i++)	{
			SETFLOAT(&(sortie[2*i]),link[i]->mass1->nbr);
			SETFLOAT(&(sortie[2*i+1]),link[i]->mass2->nbr);
		}
		ToOutAnything(0, gensym("linksMassesL"), 2*nb_link, sortie);
	}

	void m_info_dumpl()
	// List of masses positions on first outlet
	{	
		t_atom sortie[7];
		t_int i;
	
		for (i=0; i<nb_mass; i++)	{
			SETFLOAT(&(sortie[0]),mass[i]->nbr);
			SETSYMBOL(&(sortie[1]),mass[i]->Id);
			SETFLOAT(&(sortie[2]),mass[i]->mobile);
			SETFLOAT(&(sortie[3]),1/mass[i]->invM);
			SETFLOAT(&(sortie[4]),mass[i]->posX);
			SETFLOAT(&(sortie[5]),mass[i]->posY);
		ToOutAnything(1, gensym("Mass"), 6, sortie);
		}

		for (i=0; i<nb_link; i++)	{
			SETFLOAT(&(sortie[0]),link[i]->nbr);
			SETSYMBOL(&(sortie[1]),link[i]->Id);
			SETFLOAT(&(sortie[2]),link[i]->mass1->nbr);
			SETFLOAT(&(sortie[3]),link[i]->mass2->nbr);
			SETFLOAT(&(sortie[4]),link[i]->K1);
			SETFLOAT(&(sortie[5]),link[i]->D1);
			SETFLOAT(&(sortie[6]),link[i]->D2);
		ToOutAnything(1, gensym("Link"), 7, sortie);
		}

	}

// --------------------------------------------------------------  GLOBAL VARIABLES 

	t_link * link[nb_max_link];
	t_mass * mass[nb_max_mass];
	t_float Xmin, Xmax, Ymin, Ymax;
	int nb_link, nb_mass, id_mass, id_link;

// --------------------------------------------------------------  SETUP

private:

	static void setup(t_classid c)
	{
		// --- set up meth(i=0; i<nb_link;i++)ods (class scope) ---

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
		FLEXT_CADDMETHOD_(c,0,"linksMassesL",m_link_dumpl);
		FLEXT_CADDMETHOD_(c,0,"massesForcesL",m_force_dumpl);
		FLEXT_CADDMETHOD_(c,0,"setMobile",m_set_mobile);
		FLEXT_CADDMETHOD_(c,0,"setFixed",m_set_fixe);
	}

	// for every registered method a callback has to be declared
	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK(m_mass_dumpl)
	FLEXT_CALLBACK(m_force_dumpl)
	FLEXT_CALLBACK(m_link_dumpl)
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

// instantiate the class (constructor has a variable argument list)
FLEXT_NEW_V("msd2D",msd2D)


