#include "vecblk.h"
//#include <math.h>


///////////////////////////////////////////////////////////////////////////
// VecBlock class
///////////////////////////////////////////////////////////////////////////

VecBlock::VecBlock(BL cx,I msrc,I mdst,I marg,I blarg):
	cplx(cx),asrc(msrc),barg(blarg),aarg(marg*blarg),adst(mdst)
{
	I i,all = asrc+aarg*blarg+adst;
	vecs = new VBuffer *[all];
	for(i = 0; i < all; ++i) vecs[i] = NULL;
}

VecBlock::~VecBlock()
{
	if(vecs) {
		I all = asrc+aarg*barg+adst;
		for(I i = 0; i < all; ++i) 
			if(vecs[i]) delete vecs[i];
		delete[] vecs;
	}
}

Vasp *VecBlock::_DstVasp(I n)
{
	Vasp *ret = new Vasp;
	ret->Frames(Frames());
	for(I i = 0; i < n; ++i) *ret += Vasp::Ref(*_Dst(i));
	return ret;
}

Vasp *VecBlock::_SrcVasp(I n)
{
	Vasp *ret = new Vasp;
	ret->Frames(Frames());
	for(I i = 0; i < n; ++i) *ret += Vasp::Ref(*_Src(i));
	return ret;
}

Vasp *VecBlock::_ResVasp(I n)
{
	return _Dst(0)?_DstVasp(n):_SrcVasp(n);
}



