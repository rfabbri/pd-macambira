/*
	$Id: opencv.c 3977 2008-07-04 20:15:08Z matju $

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "../gridflow.h.fcs"
#include <opencv/cv.h>
#include <errno.h>

int ipl_eltype(NumberTypeE e) {
  switch (e) {
    case uint8_e: return IPL_DEPTH_8U;
    // IPL_DEPTH_8S not supported
    // IPL_DEPTH_16U not supported
    case int16_e: return IPL_DEPTH_16S;
    case int32_e: return IPL_DEPTH_32S;
    case float32_e: return IPL_DEPTH_32F;
    case float64_e: return IPL_DEPTH_64F;
    default: RAISE("unsupported type %s",number_type_table[e].name);
  }
}

NumberTypeE gf_ipltype(int e) {
  switch (e) {
    case IPL_DEPTH_8U: return uint8_e;
    // IPL_DEPTH_8S not supported
    // IPL_DEPTH_16U not supported
    case IPL_DEPTH_16S: return int16_e;
    case IPL_DEPTH_32S: return int32_e;
    case IPL_DEPTH_32F: return float32_e;
    case IPL_DEPTH_64F: return float64_e;
    default: RAISE("unsupported IPL type %d",e);
  }
}

int cv_eltype(NumberTypeE e) {
  switch (e) {
    case uint8_e: return CV_8U;
    // CV_8S not supported
    // CV_16U not supported
    case int16_e: return CV_16S;
    case int32_e: return CV_32S;
    case float32_e: return CV_32F;
    case float64_e: return CV_64F;
    default: RAISE("unsupported type %s",number_type_table[e].name);
  }
}

NumberTypeE gf_cveltype(int e) {
  switch (e) {
    case CV_8U: return uint8_e;
    // CV_8S not supported
    // CV_16U not supported
    case CV_16S: return int16_e;
    case CV_32S: return int32_e;
    case CV_32F: return float32_e;
    case CV_64F: return float64_e;
    default: RAISE("unsupported CV type %d",e);
  }
}

enum CvMode {
	cv_mode_auto,
	cv_mode_channels,
	cv_mode_nochannels,
};

CvMode convert (const t_atom2 &x, CvMode *foo) {
	if (x==gensym("auto"))       return cv_mode_auto;
	if (x==gensym("channels"))   return cv_mode_channels;
	if (x==gensym("nochannels")) return cv_mode_nochannels;
	RAISE("invalid CvMode");
}

CvArr *cvGrid(PtrGrid g, CvMode mode, int reqdims=-1) {
	P<Dim> d = g->dim;
	int channels=1;
	int dims=g->dim->n;
	//post("mode=%d",(int)mode);
	if (mode==cv_mode_channels && g->dim->n==0) RAISE("CV: channels dimension required for 'mode channels'");
	if ((mode==cv_mode_auto && g->dim->n>=3) || mode==cv_mode_channels) channels=g->dim->v[--dims];
	if (channels>64) RAISE("CV: too many channels. max 64, got %d",channels);
	//post("channels=%d dims=%d nt=%d",channels,dims,g->nt);
	//post("bits=%d",number_type_table[g->nt].size);
	//if (dims==2) return cvMat(g->dim->v[0],g->dim->v[1],cv_eltype(g->nt),g->data);
	if (reqdims>=0 && reqdims!=dims) RAISE("CV: wrong number of dimensions. expected %d, got %d", reqdims, dims);
	if (dims==2) {
		CvMat *a = cvCreateMatHeader(g->dim->v[0],g->dim->v[1],CV_MAKETYPE(cv_eltype(g->nt),channels));
		cvSetData(a,g->data,g->dim->prod(1)*(number_type_table[g->nt].size/8));
		return a;
	}
	RAISE("unsupported number of dimensions (got %d)",g->dim->n);
	//return 0;
}

IplImage *cvImageGrid(PtrGrid g /*, CvMode mode */) {
	P<Dim> d = g->dim;
	if (d->n!=3) RAISE("expected 3 dimensions, got %s",d->to_s());
	int channels=g->dim->v[2];
	if (channels>64) RAISE("too many channels. max 64, got %d",channels);
	CvSize size = {d->v[1],d->v[0]};
	IplImage *a = cvCreateImageHeader(size,ipl_eltype(g->nt),channels);
	cvSetData(a,g->data,g->dim->prod(1)*(number_type_table[g->nt].size/8));
	return a;
}

\class CvOp1 : FObject {
	\attr CvMode mode;
	\constructor () {mode = cv_mode_auto;}
	/* has no default \grin 0 handler so far. */
};
\end class {}

\class CvOp2 : CvOp1 {
	PtrGrid r;
	\constructor (Grid *r=0) {this->r = r?r:new Grid(new Dim(),int32_e,true);}
	virtual void func(CvArr *l, CvArr *r, CvArr *o) {/* rien */}
	\grin 0
	\grin 1
};
GRID_INLET(CvOp2,0) {
	SAME_TYPE(in,r);
	if (!in->dim->equal(r->dim)) RAISE("dimension mismatch: left:%s right:%s",in->dim->to_s(),r->dim->to_s());
	in->set_chunk(0);
} GRID_FLOW {
	PtrGrid l = new Grid(in->dim,(T *)data);
	PtrGrid o = new Grid(in->dim,in->nt);
	CvArr *a = cvGrid(l,mode);
	CvArr *b = cvGrid(r,mode);
	CvArr *c = cvGrid(o,mode);
	func(a,b,c);
	cvReleaseMat((CvMat **)&a);
	cvReleaseMat((CvMat **)&b);
	cvReleaseMat((CvMat **)&c);
	out = new GridOutlet(this,0,in->dim,in->nt);
	out->send(o->dim->prod(),(T *)o->data);
} GRID_END
GRID_INPUT2(CvOp2,1,r) {} GRID_END
\end class {}

#define FUNC(CLASS) CLASS(BFObject *bself, MESSAGE):CvOp2(bself,MESSAGE2) {} virtual void func(CvArr *l, CvArr *r, CvArr *o)

\class CvAdd : CvOp2 {FUNC(CvAdd) {cvAdd(l,r,o,0);}};
\end class {install("cv.Add",2,1);}
\class CvSub : CvOp2 {FUNC(CvSub) {cvSub(l,r,o,0);}};
\end class {install("cv.Sub",2,1);}
\class CvMul : CvOp2 {FUNC(CvMul) {cvMul(l,r,o,1);}};
\end class {install("cv.Mul",2,1);}
\class CvDiv : CvOp2 {FUNC(CvDiv) {cvDiv(l,r,o,1);}};
\end class {install("cv.Div",2,1);}
\class CvAnd : CvOp2 {FUNC(CvAnd) {cvAnd(l,r,o,0);}};
\end class {install("cv.And",2,1);}
\class CvOr  : CvOp2 {FUNC(CvOr ) {cvOr( l,r,o,0);}};
\end class {install("cv.Or" ,2,1);}
\class CvXor : CvOp2 {FUNC(CvXor) {cvXor(l,r,o,0);}};
\end class {install("cv.Xor",2,1);}

\class CvInvert : CvOp1 {
	\constructor () {}
	\grin 0
};
GRID_INLET(CvInvert,0) {
	if (in->dim->n!=2) RAISE("should have 2 dimensions");
	if (in->dim->v[0] != in->dim->v[1]) RAISE("matrix should be square");
	in->set_chunk(0);
} GRID_FLOW {
	//post("l=%p, r=%p", &*l, &*r);
	PtrGrid l = new Grid(in->dim,(T *)data);
	PtrGrid o = new Grid(in->dim,in->nt);
	CvArr *a = cvGrid(l,mode);
	CvArr *c = cvGrid(o,mode);
	//post("a=%p, b=%p", a, b);
	cvInvert(a,c);
	cvReleaseMat((CvMat **)&a);
	cvReleaseMat((CvMat **)&c);
	out = new GridOutlet(this,0,in->dim,in->nt);
	out->send(o->dim->prod(),(T *)o->data);
} GRID_END
\end class {install("cv.Invert",1,1);}

\class CvSVD : CvOp1 {
	\grin 0
	\constructor () {}
};
GRID_INLET(CvSVD,0) {
	if (in->dim->n!=2) RAISE("should have 2 dimensions");
	if (in->dim->v[0] != in->dim->v[1]) RAISE("matrix should be square");
	in->set_chunk(0);
} GRID_FLOW {
	PtrGrid l = new Grid(in->dim,(T *)data);
	PtrGrid o0 = new Grid(in->dim,in->nt);
	PtrGrid o1 = new Grid(in->dim,in->nt);
	PtrGrid o2 = new Grid(in->dim,in->nt);
	CvArr *a = cvGrid(l,mode);
	CvArr *c0 = cvGrid(o0,mode);
	CvArr *c1 = cvGrid(o1,mode);
	CvArr *c2 = cvGrid(o2,mode);
	cvSVD(a,c0,c1,c2);
	cvReleaseMat((CvMat **)&a);
	cvReleaseMat((CvMat **)&c0);
	cvReleaseMat((CvMat **)&c1);
	cvReleaseMat((CvMat **)&c2);
	out = new GridOutlet(this,2,in->dim,in->nt); out->send(o2->dim->prod(),(T *)o2->data);
	out = new GridOutlet(this,1,in->dim,in->nt); out->send(o1->dim->prod(),(T *)o1->data);
	out = new GridOutlet(this,0,in->dim,in->nt); out->send(o0->dim->prod(),(T *)o0->data);
} GRID_END
\end class {install("cv.SVD",1,3);}

\class CvSplit : CvOp1 {
	int channels;
	\constructor (int channels) {
		if (channels<0 || channels>64) RAISE("channels=%d is not in 1..64",channels);
		this->channels = channels;
		bself->noutlets_set(channels);
	}
};
\end class {}

\class CvHaarDetectObjects : FObject {
	\attr double scale_factor; /*=1.1*/
	\attr int min_neighbors;   /*=3*/
	\attr int flags;           /*=0*/
	\constructor () {
		scale_factor=1.1;
		min_neighbors=3;
		flags=0;
		//cascade = cvLoadHaarClassifierCascade("<default_face_cascade>",cvSize(24,24));
		const char *filename = OPENCV_SHARE_PATH "/haarcascades/haarcascade_frontalface_alt2.xml";
		FILE *f = fopen(filename,"r");
		if (!f) RAISE("error opening %s: %s",filename,strerror(errno));
		fclose(f);
		cascade = (CvHaarClassifierCascade *)cvLoad(filename,0,0,0);
		int s = cvGetErrStatus();
		post("cascade=%p, cvGetErrStatus=%d cvErrorStr=%s",cascade,s,cvErrorStr(s));
		//cascade = cvLoadHaarClassifierCascade(OPENCV_SHARE_PATH "/data/haarcascades/haarcascade_frontalface_alt2.xml",cvSize(24,24));
		storage = cvCreateMemStorage(0);
	}
	CvHaarClassifierCascade *cascade;
	CvMemStorage *storage;
	\grin 0
};
GRID_INLET(CvHaarDetectObjects,0) {
	in->set_chunk(0);
} GRID_FLOW {
	PtrGrid l = new Grid(in->dim,(T *)data);
	IplImage *img = cvImageGrid(l);
	CvSeq *ret = cvHaarDetectObjects(img,cascade,storage,scale_factor,min_neighbors,flags);
	int n = ret ? ret->total : 0;
	out = new GridOutlet(this,0,new Dim(n,2,2));
	for (int i=0; i<n; i++) {
		CvRect *r = (CvRect *)cvGetSeqElem(ret,i);
		int32 duh[] = {r->y,r->x,r->y+r->height,r->x+r->width};
		out->send(4,duh);
	}
} GRID_END
\end class {install("cv.HaarDetectObjects",2,1);}

\class CvKalmanWrapper : CvOp1 {
	CvKalman *kal;
	\constructor (int dynam_params, int measure_params, int control_params=0) {
		kal = cvCreateKalman(dynam_params,measure_params,control_params);
	}
	~CvKalmanWrapper () {if (kal) cvReleaseKalman(&kal);}
	\decl void _0_bang ();
	\grin 0
	\grin 1
};

void cvMatSend(const CvMat *self, FObject *obj, int outno) {
	int m = self->rows;
	int n = self->cols;
	int e = CV_MAT_TYPE(cvGetElemType(self));
	int c = CV_MAT_CN(  cvGetElemType(self));
	GridOutlet *out = new GridOutlet(obj,0,new Dim(m,n));
	for (int i=0; i<m; i++) {
		uchar *meuh = cvPtr2D(self,i,0,0);
		switch (e) {
		  case CV_8U:  out->send(c*n,  (uint8 *)meuh); break;
		  case CV_16S: out->send(c*n,  (int16 *)meuh); break;
		  case CV_32S: out->send(c*n,  (int32 *)meuh); break;
		  case CV_32F: out->send(c*n,(float32 *)meuh); break;
		  case CV_64F: out->send(c*n,(float64 *)meuh); break;
		}
	}
}

\def void _0_bang () {
	const CvMat *r = cvKalmanPredict(kal,0);
	cvMatSend(r,this,0);
}

GRID_INLET(CvKalmanWrapper,0) {
	in->set_chunk(0);
} GRID_FLOW {
	PtrGrid l = new Grid(in->dim,(T *)data);
	CvMat *a = (CvMat *)cvGrid(l,mode,2);
	const CvMat *r = cvKalmanPredict(kal,a);
	cvMatSend(r,this,0);
} GRID_END

GRID_INLET(CvKalmanWrapper,1) {
	in->set_chunk(0);
} GRID_FLOW {
	PtrGrid l = new Grid(in->dim,(T *)data);
	CvMat *a = (CvMat *)cvGrid(l,mode,2);
	const CvMat* r = cvKalmanCorrect(kal,a);
	cvMatSend(r,this,0);
} GRID_END
\end class {install("cv.Kalman",2,1);}

//\class CvEllipse : FObject {
//	\grin 0
//};
//GRID_INLET(CvEllipse,0) {
//	in->set_chunk(0);
//} GRID_FLOW {
//} GRID_END
//\end class {install("cv.Ellipse",1,1);}

/*
void cvEllipse( CvArr* img, CvPoint center, CvSize axes, double angle,
                double start_angle, double end_angle, CvScalar color,
                int thickness=1, int line_type=8, int shift=0 );
CvSeq* cvApproxPoly( const void* src_seq, int header_size, CvMemStorage* storage,
                     int method, double parameter, int parameter2=0 );
void cvCalcOpticalFlowHS( const CvArr* prev, const CvArr* curr, int use_previous,
                          CvArr* velx, CvArr* vely, double lambda,
                          CvTermCriteria criteria );
void cvCalcOpticalFlowLK( const CvArr* prev, const CvArr* curr, CvSize win_size,
                          CvArr* velx, CvArr* vely );
void cvCalcOpticalFlowBM( const CvArr* prev, const CvArr* curr, CvSize block_size,
                          CvSize shift_size, CvSize max_range, int use_previous,
                          CvArr* velx, CvArr* vely );
void cvCalcOpticalFlowPyrLK( const CvArr* prev, const CvArr* curr, CvArr* prev_pyr, CvArr* curr_pyr,
                             const CvPoint2D32f* prev_features, CvPoint2D32f* curr_features,
                             int count, CvSize win_size, int level, char* status,
                             float* track_error, CvTermCriteria criteria, int flags );
void cvCalcBackProject( IplImage** image, CvArr* back_project, const CvHistogram* hist );
void cvCalcHist( IplImage** image, CvHistogram* hist, int accumulate=0, const CvArr* mask=NULL );
CvHistogram* cvCreateHist( int dims, int* sizes, int type, float** ranges=NULL, int uniform=1 );
void cvSnakeImage( const IplImage* image, CvPoint* points, int length,
                   float* alpha, float* beta, float* gamma, int coeff_usage,
                   CvSize win, CvTermCriteria criteria, int calc_gradient=1 );
int cvMeanShift( const CvArr* prob_image, CvRect window, CvTermCriteria criteria, CvConnectedComp* comp );
int  cvCamShift( const CvArr* prob_image, CvRect window, CvTermCriteria criteria, CvConnectedComp* comp, CvBox2D* box=NULL );
*/

/* **************************************************************** */

static int erreur_handleur (int status, const char* func_name, const char* err_msg, const char* file_name, int line, void *userdata) {
	cvSetErrStatus(CV_StsOk);
	// we might be looking for trouble because we don't know whether OpenCV is throw-proof.
	RAISE("OpenCV error: status='%s' func_name=%s err_msg=\"%s\" file_name=%s line=%d",cvErrorStr(status),func_name,err_msg,file_name,line);
	// if this breaks OpenCV, then we will have to use post() or a custom hybrid of post() and RAISE() that does not cause a
	// longjmp when any OpenCV functions are on the stack.
	return 0;
}

void startup_opencv() {
	/* CvErrorCallback z = */ cvRedirectError(erreur_handleur);
	\startall
}
