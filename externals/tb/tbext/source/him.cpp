/* Copyright (c) 2004 Tim Blechmann.                                            */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/*                                                                              */
/* him~ is a semi-classicical simulation of an hydrogen atom in a magnetic field*/
/*                                                                              */
/* him~ uses the flext C++ layer for Max/MSP and PD externals.                  */
/* get it at http://www.parasitaere-kapazitaeten.de/PD/ext                      */
/* thanks to Thomas Grill                                                       */
/*                                                                              */
/* him~ is based on code provided in the lecture "physik auf dem computer 1"    */
/* held by joerg main during the winter semester 2003/04 at the university      */
/* stuttgart ... many thanks to him and his assistant ralf habel                */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/* coded while listening to: Elliott Sharp: The Velocity Of Hue                 */
/*                           Fred Frith: Traffic Continues                      */
/*                           Nmperign: Twisted Village                          */
/*                           Frank Lowe: Black Beings                           */
/*                                                                              */



#include <flext.h>
#include <cstdlib>
#include <cmath>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error upgrade your flext version!!!!!!
#endif

#define NUMB_EQ 4

class him: public flext_dsp
{
  FLEXT_HEADER(him,flext_dsp);

public: 
    him(int argc, t_atom *argv);

protected:
    virtual void m_signal (int n, float *const *in, float *const *out);
    t_float *outs;
    
    void set_mu(t_float);
    void set_muv(t_float);
    void set_nu(t_float);
    void set_nuv(t_float);
    void set_etilde(t_float);
    void set_dt(t_float);
    void set_regtime(bool);
    void set_output(t_int);
    void state();
    void reset();

private:
    // contains DGL-System
    t_float deriv(t_float x[],int eq);
    t_float result;

    // 4th order Runge Kutta update of the dynamical variables 
    void   runge_kutta_4(t_float dt);
    int i;
    t_float k1[NUMB_EQ],k2[NUMB_EQ],k3[NUMB_EQ],k4[NUMB_EQ];
    t_float temp1[NUMB_EQ], temp2[NUMB_EQ], temp3[NUMB_EQ];

    //these are our data
    t_float data[4]; //mu, muv, nu, nuv (semi-parabolische koordinaten)
    t_float E;

    //and these our settings
    t_float dt;
    t_int output;   //mu, muv, nu, nuv, x, y 
    bool regtime; //if true "regularisierte zeit"

    //Callbacks
    FLEXT_CALLBACK_1(set_mu,t_float);
    FLEXT_CALLBACK_1(set_muv,t_float);
    FLEXT_CALLBACK_1(set_nu,t_float);
    FLEXT_CALLBACK_1(set_nuv,t_float);
    FLEXT_CALLBACK_1(set_etilde,t_float);
    FLEXT_CALLBACK_1(set_dt,t_float);
    FLEXT_CALLBACK_1(set_regtime,bool);
    FLEXT_CALLBACK_1(set_output,t_int);
    FLEXT_CALLBACK(state);
    FLEXT_CALLBACK(reset);

    //reset mus / nus 
    void reset_nuv()
    {
	data[3]= 0.5*sqrt( - (4*data[1]*data[1]) - 
			   ( data[0]*data[0]*data[1]*data[1]*data[1]*data[1])
			   + (8*E*data[0]) - (8*E*data[2]) - 
			   (data[0]*data[0]*data[0]*data[0]*data[1]*data[1])
			   + 16);
    }

    void reset_muv()
    {
	data[1]= 0.5*sqrt( - (4*data[3]*data[3]) - 
			   ( data[0]*data[0]*data[1]*data[1]*data[1]*data[1])
			   + (8*E*data[0]) - (8*E*data[2]) - 
			   (data[0]*data[0]*data[0]*data[0]*data[1]*data[1])
			   + 16);
    }
    
};


FLEXT_LIB_DSP_V("him~",him)

him::him(int argc, t_atom *argv)
{
    AddInAnything();
    AddOutSignal();
    FLEXT_ADDMETHOD_F(0,"mu",set_mu);
    FLEXT_ADDMETHOD_F(0,"muv",set_muv);
    FLEXT_ADDMETHOD_F(0,"nu",set_nu);
    FLEXT_ADDMETHOD_F(0,"nuv",set_nuv);
    FLEXT_ADDMETHOD_F(0,"e",set_etilde);
    FLEXT_ADDMETHOD_F(0,"dt",set_dt);
    FLEXT_ADDMETHOD_B(0,"regtime",set_regtime);
    FLEXT_ADDMETHOD_I(0,"output",set_output);
    FLEXT_ADDMETHOD_(0,"state",state);
    FLEXT_ADDMETHOD_(0,"reset",reset);
    
    
    //beginning values
    if (argc==1)
	E=atom_getfloat(argv);
    else
	E= -float(rand())/float(RAND_MAX);
    
    reset();

    state();

    //default mode
    regtime=true;
    output=0;
    dt=0.01;
} 

t_float him::deriv(t_float x[], int eq)
{
  // set DGL-System here
    if (eq == 0) result =  x[1];
    if (eq == 1) result = 2*E*x[0]-0.25*x[0]*x[2]*x[2]*(2*x[0]*x[0]+x[2]*x[2]);
    if (eq == 2) result =  x[3];
    if (eq == 3) result = 2*E*x[2]-0.25*x[2]*x[0]*x[0]*(2*x[2]*x[2]+x[0]*x[0]);

  return result;
}

void him::runge_kutta_4(t_float dt)                          
{
    for(i=0;i<=NUMB_EQ-1;i++) // iterate over equations 
	{
	    k1[i] = dt * deriv(data,i);
	    temp1[i] = data[i] + 0.5*k1[i];	    	    
	}
    
    for(i=0;i<=NUMB_EQ-1;i++)
	{
	    k2[i] = dt * deriv(temp1,i);
	    temp2[i] = data[i] + 0.5*k2[i];
        }
    
    for(i=0;i<=NUMB_EQ-1;i++)
	{
	    k3[i] = dt * deriv(temp2,i);
	    temp3[i] = data[i] + k3[i];    
	}
    
    for(i=0;i<=NUMB_EQ-1;i++)
	{
	    k4[i] = dt * deriv(temp3,i);
	    data[i] = data[i] + (k1[i] + (2.*(k2[i]+k3[i])) + k4[i])/6.;
        }
    
    reset_muv();
    
}



void him::m_signal(int n, t_float *const *in, t_float *const *out)
{
    outs = out[0];
    
    if (regtime)
	{
	    switch (output)
		{
		case 0:
		    for (int j=0;j!=n;++j)
			{
			runge_kutta_4(dt);
			*(outs+j)=data[0];
			}
		    break;
		    
		case 1:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt);
			    *(outs+j)=data[1];
			}
		    break;
		    
		case 2:
		    for (int j=0;j!=n;++j)
			{
			runge_kutta_4(dt);
			*(outs+j)=data[2];
			}
		    break;
		
		case 3:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt);
			    *(outs+j)=data[3];
			}
		    break;
		    
		case 4:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt);
			    *(outs+j)=data[0]*data[2];
			}
		    break;
		    
		case 5:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt);
			    *(outs+j)=(data[0]*data[0]-data[2]*data[2])*0.5;
			}
		    break;
		}
	}
    else
	{
	    switch (output)
		{	    
		case 0:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt/
					  (2*sqrt(data[0]*data[0]+data[2]*data[2])));
			    *(outs+j)=data[0];
			}
		    break;
		    
		case 1:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt/
					  (2*sqrt(data[0]*data[0]+data[2]*data[2])));
			    *(outs+j)=data[1];
			}
		    break;
		    
		case 2:
		    for (int j=0;j!=n;++j)
		    {
			runge_kutta_4(dt/
				      (2*sqrt(data[0]*data[0]+data[2]*data[2])));
			*(outs+j)=data[2];
		    }
		    break;
		    
		case 3:
		    for (int j=0;j!=n;++j)
		    {
			runge_kutta_4(dt/
				      (2*sqrt(data[0]*data[0]+data[2]*data[2])));
			*(outs+j)=data[3];
		    }
		    break;
		    
		case 4:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt/
					  (2*sqrt(data[0]*data[0]+data[2]*data[2])));
			    *(outs+j)=data[0]*data[2];
			}
		break;
		
		case 5:
		    for (int j=0;j!=n;++j)
			{
			    runge_kutta_4(dt/
					  (2*sqrt(data[0]*data[0]+data[2]*data[2])));
			    *(outs+j)=(data[0]*data[0]-data[2]*data[2])*0.5;
			}
		    break;
		}
	}
}    

void him::set_mu(t_float f)
{
    data[0]=f;
    reset_nuv();
}

void him::set_muv(t_float f)
{
    data[1]=f;
    reset_nuv();
}

void him::set_nu(t_float f)
{
    data[3]=f;
    reset_nuv();
}

void him::set_nuv(t_float f)
{
    data[3]=f;
    reset_muv();
    post("resetting muv!!!");
}

void him::set_etilde(t_float f)
{
    E=f;
    reset_nuv();
}

void him::set_dt(t_float f)
{
    dt=f;
}

void him::set_regtime(bool b)
{
    regtime=b;
}

void him::set_output(t_int i)
{
    output=i;
}

void him::state()
{
    post("mu %f",data[0]);
    post("mus %f",data[1]);
    post("nu %f",data[2]);
    post("nus %f",data[3]);
    post("etilde %f",E);
}

void him::reset()
{
    data[0]=float(rand())/float(RAND_MAX);
    data[1]=float(rand())/float(RAND_MAX);
    data[2]=float(rand())/float(RAND_MAX);
    reset_muv();
    post("randomizing values");
}
