/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

/*! \file vasp_imm.cpp
	\brief Definitions for immediate vasps
*/


#include "classes.h"
#include "util.h"
#include "buflib.h"
#include "oploop.h"


/*! \class vasp_imm
	\remark \b vasp.imm
	\brief Get vasp immediate.
	\since 0.0.6
	\param inlet.1 vasp - is stored and output triggered
	\param inlet.1 bang - triggers output
	\param inlet.1 set - vasp to be stored 
	\param inlet.1 frames - minimum frame length
	\param inlet.2 int - minimum frame length
	\retval outlet vasp! - vasp immediate

*/
class vasp_imm:
	public vasp_op
{
	FLEXT_HEADER(vasp_imm,vasp_op)

public:
	vasp_imm(I argc,t_atom *argv):
		frms(0)
	{
		if(argc >= 1 && CanbeInt(argv[0]))
			m_frames(GetAInt(argv[0]));
		else if(argc)
			post("%s - Frame count argument invalid -> ignored",thisName());

		AddInAnything();
		AddInInt();
		AddOutAnything();
		SetupInOut();

		FLEXT_ADDMETHOD_(0,"frames",m_frames);
		FLEXT_ADDMETHOD(1,m_frames);
	}

	V m_frames(I n) { frms = n; }

	virtual V m_bang() 
	{ 
		if(!ref.Ok() || !ref.Check()) {
/*
			if(!frms) 
				post("%s - No length defined!",thisName());
			else 
*/
			{
				ImmBuf ibuf(frms);
				Vasp ret(frms,Vasp::Ref(ibuf));
				ToOutVasp(0,ret);
			}
		}
		else if(ref.Vectors() > 1) 
			post("%s - More than one vector in vasp!",thisName());
		else {
			VBuffer *buf = ref.Buffer(0);
			I len = buf->Length(),chns = buf->Channels();
			if(frms > len) len = frms; 
			
			ImmBuf imm(len);

			S *dst = imm.Pointer();
			const S *src = buf->Pointer();
			register int i;
			_D_LOOP(i,len) *(dst++) = *src,src += chns; _E_LOOP

			Vasp ret(len,Vasp::Ref(imm));
			ToOutVasp(0,ret);
		}
	}

	virtual V m_help() { post("%s - Get immediate vasp vectors",thisName()); }

protected:

	I frms;
	
private:
	FLEXT_CALLBACK_I(m_frames)
};

FLEXT_LIB_V("vasp, vasp.imm vasp.!",vasp_imm)


