//******************************************************************************
// rblti, Copyright 2005 by Mathieu Bouchard and Heri Andria
// pylti, Copyright 2005 by Michael Neuroth
// a wrapper for ltilib using SWIG

%module rblti
%rename(inplace_add) operator +=;
%rename(inplace_sub) operator -=;
%rename(inplace_mul) operator *=;
%rename(inplace_div) operator /=;
%rename(not_equal)   operator !=;
%include "std_list_ruby.i"
%include "std_string.i"
//%include "std_list.i"
//%include "std_vector.i" 
//%include "std_map.i"

using namespace std;

// sollte nach dem include der std_*.i Dateien stehen, ansonsten gibt swig einen Fehlercode zurueck !
//%feature("autodoc","1")

std::string _getStdString(std::string * pStr);
bool _setStdString(std::string * pStr, const std::string & strValue);

// for the access to c arrays
%include "carrays.i"
%array_functions(float,floatArray)
%array_functions(double,doubleArray)
%array_functions(int,intArray)
%array_class(float,floatArr)
%array_class(double,doubleArr)
%array_class(int,intArr)

//test:
//namespace std {
//    %template(sdmap) map<string,double>;		// TODO: does not work yet ...
//}
//namespace std {
//    %template(vectordouble) vector<double>;
//}

// **************************************************************************
// **************************************************************************
// This part is for the c++ wrapper compile phase !
// This code will be copied into the wrapper-code (generated from swig)
%{
#include <string>

// TODO: to be removed, only for tests
std::string _getStdString(std::string * pStr) {return *pStr;}
bool _setStdString(std::string * pStr, const std::string & strValue) {
    if(pStr) *pStr = strValue;
    return !!pStr;
}
// </to-be-removed>

#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_STRING
#undef PACKAGE_VERSION

#include "ltiObject.h"
//#include "ltiTypes.h"
#include "ltiIoHandler.h"
#include "ltiIoObject.h"
#include "ltiMathObject.h"
#include "ltiRGBPixel.h"
#include "ltiPoint.h"
#include "ltiPointList.h"
#include "ltiPolygonPoints.h"
#include "ltiGeometry.h"
#include "ltiGenericVector.h"           // MODIFIED for SWIG !
#include "ltiVector.h"
#include "ltiArray.h"
#include "ltiGenericMatrix.h"           // MODIFIED for SWIG !
#include "ltiMatrix.h"                  // MODIFIED for SWIG !
#include "ltiTree.h"
#include "ltiHistogram.h"
#include "ltiImage.h"
#include "ltiContour.h"
#include "ltiLinearKernels.h"
#include "ltiGradientKernels.h"
#include "ltiHessianKernels.h"
#include "ltiLaplacianKernel.h"
#include "ltiSecondDerivativeKernels.h"

#include "ltiFunctor.h"
// durch SWIG manipulierte typ parameters in functor wieder zurueck benennen
#define _functor functor               // wegen Namenskonflikt mit schon deklarierter Klasse und dem Trick ueber den namespace um die Parameter-Klassen zu generieren (gen_ltilib_class.py)
#define _functor_parameters parameters // sezte den aus dem XML generierten Parameter-Klassen-Namen wieder zurueck auf den urspruenglichen Namen
// notwendig fuer die plain Methoden lti::write(...) und lti::read(...)
namespace lti {typedef lti::functor::parameters functor_parameters;}
#include "ltiModifier.h"
#define _modifier modifier
#define _modifier_parameters parameters
namespace lti {typedef lti::modifier::parameters modifier_parameters;}
#include "ltiFilter.h"
#define _filter filter
#define _filter_parameters parameters
namespace lti {typedef lti::filter::parameters filter_parameters;}
#include "ltiIOFunctor.h"
#define _ioFunctor ioFunctor
#define _ioFunctor_parameters parameters
namespace lti {typedef lti::ioFunctor::parameters ioFunctor_parameters;}
#include "ltiBMPFunctor.h"
#define _ioBMP ioBMP
#define _ioBMP_parameters parameters
#define lti_ioBMP_parameters ioBMP_parameters      // TODO: PATCH !
namespace lti {typedef lti::ioBMP::parameters ioBMP_parameters;}
#include "ltiJPEGFunctor.h"
#define _ioJPEG ioJPEG
#define _ioJPEG_parameters parameters
#define lti_ioJPEG_parameters ioJPEG_parameters      // TODO: PATCH !
namespace lti {typedef lti::ioJPEG::parameters ioJPEG_parameters;}
#include "ltiPNGFunctor.h"
#define _ioPNG ioPNG
#define _ioPNG_parameters parameters
#define lti_ioPNG_parameters ioPNG_parameters     
namespace lti {typedef lti::ioPNG::parameters ioPNG_parameters;}
#include "ltiALLFunctor.h"
#define _ioImage ioImage
#define _ioImage_parameters parameters
#define ioImage_parameters parameters                   // TODO: PATCH !
namespace lti {typedef lti::ioImage::parameters ioImage_parameters;}
#include "ltiViewerBase.h"
#define _viewerBase viewerBase
#define _viewerBase_parameters parameters
namespace lti {typedef lti::viewerBase::parameters viewerBase_parameters;}
#include "ltiExternViewer.h"
#define _externViewer externViewer
#define _externViewer_parameters parameters
namespace lti {typedef lti::externViewer::parameters externViewer_parameters;}
#include "ltiSplitImage.h"
#include "ltiSplitImageTorgI.h"

#include "ltiUsePalette.h"
#define _usePalette usePalette
#define _usePalette_parameters parameters
namespace lti {typedef lti::usePalette::parameters usePalette_parameters;}
#include "ltiTransform.h"
#define _transform transform
#define _transform_parameters parameters
namespace lti {typedef lti::transform::parameters transform_parameters;}
#include "ltiGradientFunctor.h"
#define _gradientFunctor gradientFunctor
#define _gradientFunctor_parameters parameters
#define lti_gradientFunctor_parameters gradientFunctor_parameters      // TODO: PATCH !
namespace lti {typedef lti::gradientFunctor::parameters gradientFunctor_parameters;}
#include "ltiColorContrastGradient.h"
#define _colorContrastGradient colorContrastGradient
#define _colorContrastGradient_parameters parameters
#define lti_colorContrastGradient_parameters colorContrastGradient_parameters      // TODO: PATCH !
namespace lti {typedef lti::colorContrastGradient::parameters colorContrastGradient_parameters;}
#include "ltiEdgeDetector.h"
#define _edgeDetector edgeDetector
#define _edgeDetector_parameters parameters
namespace lti {typedef lti::edgeDetector::parameters edgeDetector_parameters;}
#include "ltiClassicEdgeDetector.h"
#define _classicEdgeDetector classicEdgeDetector
#define _classicEdgeDetector_parameters parameters
namespace lti {typedef lti::classicEdgeDetector::parameters classicEdgeDetector_parameters;}
#include "ltiCannyEdges.h"
#define _cannyEdges cannyEdges
#define _cannyEdges_parameters parameters
namespace lti {typedef lti::cannyEdges::parameters cannyEdges_parameters;}
#include "ltiConvolution.h"
#define _convolution convolution
#define _convolution_parameters parameters
namespace lti {typedef lti::convolution::parameters convolution_parameters;}
#include "ltiSegmentation.h"
#define _segmentation segmentation
#define _segmentation_parameters parameters
namespace lti {typedef lti::segmentation::parameters segmentation_parameters;}
#include "ltiRegionGrowing.h"
#define _regionGrowing regionGrowing
#define _regionGrowing_parameters parameters
namespace lti {typedef lti::regionGrowing::parameters regionGrowing_parameters;}
#include "ltiObjectsFromMask.h"
#define _objectsFromMask objectsFromMask
#define _objectsFromMask_parameters parameters
#define _objectsFromMask_objectStruct objectStruct
namespace lti {
typedef lti::objectsFromMask::objectStruct objectStruct;
typedef lti::objectsFromMask::objectStruct objectsFromMask_objectStruct;
typedef lti::objectsFromMask::parameters objectsFromMask_parameters;
}
////TODO: add better tree support !!!
////#define _tree tree
//#define _tree_objectStruct_node node
//#define _tree tree<objectStruct>
//namespace lti {
//typedef lti::tree<objectStruct>::node tree_objectStruct_node;
//}
#include "ltiPolygonApproximation.h"
#define _polygonApproximation polygonApproximation
#define _polygonApproximation_parameters parameters
namespace lti {typedef lti::polygonApproximation::parameters polygonApproximation_parameters;}
#include "ltiColorQuantization.h"
#define _colorQuantization colorQuantization
#define _colorQuantization_parameters parameters
namespace lti {typedef lti::colorQuantization::parameters colorQuantization_parameters;}
#include "ltiKMColorQuantization.h"
#define _kMColorQuantization kMColorQuantization
#define _kMColorQuantization_parameters parameters
namespace lti {
typedef lti::kMColorQuantization::parameters kMColorQuantization_parameters;
//typedef lti::kMColorQuantization::parameters lti_kMColorQuantization_parameters;
}
typedef lti::kMColorQuantization::parameters lti_kMColorQuantization_parameters;
#include "ltiMeanShiftSegmentation.h"
#define _meanShiftSegmentation meanShiftSegmentation
#define _meanShiftSegmentation_parameters parameters
namespace lti {typedef lti::meanShiftSegmentation::parameters meanShiftSegmentation_parameters;}
#include "ltiKMeansSegmentation.h"
#define _kMeansSegmentation kMeansSegmentation
#define _kMeansSegmentation_parameters parameters
namespace lti {
typedef lti::kMeansSegmentation::parameters kMeansSegmentation_parameters;
//typedef lti::kMeansSegmentation::parameters lti_kMeansSegmentation_parameters;
}
typedef lti::kMeansSegmentation::parameters lti_kMeansSegmentation_parameters;

#include "ltiWhiteningSegmentation.h"
#define _whiteningSegmentation whiteningSegmentation
#define _whiteningSegmentation_parameters parameters
namespace lti {typedef lti::whiteningSegmentation::parameters whiteningSegmentation_parameters;}
#include "ltiCsPresegmentation.h"
#define _csPresegmentation csPresegmentation
#define _csPresegmentation_parameters parameters
namespace lti {typedef lti::csPresegmentation::parameters csPresegmentation_parameters;}
#include "ltiFeatureExtractor.h"
#define _featureExtractor featureExtractor
#define _featureExtractor_parameters parameters
namespace lti {typedef lti::featureExtractor::parameters featureExtractor_parameters;}
#include "ltiGlobalFeatureExtractor.h"
#define _globalFeatureExtractor globalFeatureExtractor
#define _globalFeatureExtractor_parameters parameters
namespace lti {typedef lti::globalFeatureExtractor::parameters globalFeatureExtractor_parameters;}
#include "ltiGeometricFeatures.h"
#define _geometricFeatures geometricFeatures
#define _geometricFeatures_parameters parameters
namespace lti {typedef lti::geometricFeatures::parameters geometricFeatures_parameters;}
#include "ltiChromaticityHistogram.h"
#define _chromaticityHistogram chromaticityHistogram
#define _chromaticityHistogram_parameters parameters
namespace lti {typedef lti::chromaticityHistogram::parameters chromaticityHistogram_parameters;}
#include "ltiLocalFeatureExtractor.h"
#define _localFeatureExtractor localFeatureExtractor
#define _localFeatureExtractor_parameters parameters
namespace lti {typedef lti::localFeatureExtractor::parameters localFeatureExtractor_parameters;}
#include "ltiLocalMoments.h"
#define _localMoments localMoments
#define _localMoments_parameters parameters
namespace lti {typedef lti::localMoments::parameters localMoments_parameters;}
#include "ltiMorphology.h"
#define _morphology morphology
#define _morphology_parameters parameters
namespace lti {typedef lti::morphology::parameters morphology_parameters;}
#include "ltiDilation.h"
#define _dilation dilation
#define _dilation_parameters parameters
namespace lti {typedef lti::dilation::parameters dilation_parameters;}
#include "ltiErosion.h"
#define _erosion erosion
#define _erosion_parameters parameters
namespace lti {typedef lti::erosion::parameters erosion_parameters;}
#include "ltiDistanceTransform.h"
#define _distanceTransform distanceTransform
#define _distanceTransform_parameters parameters
namespace lti {typedef lti::distanceTransform::parameters distanceTransform_parameters;}
#include "ltiSkeleton.h"
#define _skeleton skeleton
#define _skeleton_parameters parameters
namespace lti {typedef lti::skeleton::parameters skeleton_parameters;}
#include "ltiClassifier.h"
#define _classifier classifier
#define _classifier_parameters parameters
#define _classifier_outputTemplate outputTemplate
#define _classifier_outputVector outputVector
namespace lti {
typedef lti::classifier::parameters classifier_parameters;
typedef lti::classifier::outputTemplate classifier_outputTemplate;
typedef lti::classifier::outputVector classifier_outputVector;
}
#include "ltiSupervisedInstanceClassifier.h"
#define _supervisedInstanceClassifier supervisedInstanceClassifier
#define _supervisedInstanceClassifier_parameters parameters
namespace lti {typedef lti::supervisedInstanceClassifier::parameters supervisedInstanceClassifier_parameters;}
/* TODO
#include "ltiDecisionTree.h"
#define _decisionTree decisionTree
#define _decisionTree_parameters parameters
namespace lti {
typedef lti::decisionTree::parameters decisionTree_parameters;
}
*/

#include "ltiSplitImageToHSI.h"
#include "ltiSplitImageToHSV.h"
#include "ltiSplitImageToHLS.h"
#include "ltiSplitImageToRGB.h"
#include "ltiSplitImageToYUV.h"

typedef std::ostream ostream;
typedef std::istream istream;

using namespace lti;

#include "lti_manual.h"

%}
// **************************************************************************
// This part is for the swig parser phase !
// This code will be used by swig to build up the type hierarchy.
// for successful mapping of const ubyte & to simple data types !!!
typedef unsigned      char ubyte;
typedef   signed      char  byte;
typedef unsigned short int uint16;
typedef   signed short int  int16;
typedef unsigned       int uint32;
typedef   signed       int  int32;
/* #ifdef LOSEDOWS
typedef unsigned   __int64 uint64;
typedef signed     __int64  int64;
#else
typedef unsigned long long uint64;
typedef   signed long long  int64;
#endif */
typedef point<int> ipoint;

%include "ltiObject.h"
%include "ltiIoHandler.h"
%include "ltiIoObject.h"
%include "ltiMathObject.h"
%include "ltiRGBPixel.h"
%include "ltiPoint.h"
namespace lti {
    %template(ipoint) tpoint<int>;
    %template(fpoint) tpoint<float>;
    %template(dpoint) tpoint<double>;
}
%template(list_ipoint) std::list<lti::ipoint>;
%include "ltiPointList.h"
%extend lti::tpointList {
// TODO: add a better (pythonic) support for iterators
void * createIterator()
{
    lti::tpointList<T>::iterator * pIter = new lti::tpointList<T>::iterator;
    (*pIter) = self->begin();
    return (void *) (pIter);
}
void deleteIterator(void *p) 
{
    lti::tpointList<T>::iterator * pIter = (lti::tpointList<T>::iterator *)p;
    delete pIter;
}
bool isEnd(void *p) 
{
    lti::tpointList<T>::iterator * pIter = (lti::tpointList<T>::iterator *)p;
    return *pIter == self->end();
}
tpoint<T> nextElement(void * p) 
{
    lti::tpointList<T>::iterator * pIter = (lti::tpointList<T>::iterator *)p;
    tpoint<T> aPointOut = *(*pIter);
    ++(*pIter);
    return aPointOut;
}
}
namespace lti {
    %template(pointList) tpointList<int>;
}
%include "ltiPolygonPoints.h"
namespace lti {
//    %template(ipolygonPoints) tpolygonPoints<int>;        // PATCH in ltiPolygonPoints.h
}
%include "ltiGeometry.h"
namespace lti {
//TODO:    %template(iintersection) intersection<ipoint>;
}
%include "ltiGenericVector.h"
%extend lti::genericVector {
    // add index support for python (Warning: this is Python-specific!) 
	const T & __getitem__( int index )
	{
		return self->at(index);
	}
	void __setitem__( int index, const T & value )
	{
		(*self)[index] = value;
	}
}
namespace lti {
    %template(dgenericVector)   genericVector<double>;
    %template(fgenericVector)   genericVector<float>;
    %template(igenericVector)   genericVector<int>;
    %template(bgenericVector)   genericVector<ubyte>;
    %template(rgbgenericVector) genericVector<rgbPixel>;
}
%include "ltiVector.h"
namespace lti {
    %template(dvector) vector<double>;
    %template(fvector) vector<float>;
    %template(ivector) vector<int>;
    %template(uvector) vector<ubyte>;
    %template(palette) vector<rgbPixel>;
}
%include "ltiArray.h"
namespace lti {
    %template(iarray) array<int>;
    %template(farray) array<float>;
    %template(darray) array<double>;
    %template(barray) array<ubyte>;
}
%include "ltiGenericMatrix.h"
%extend lti::genericMatrix {
    // add index support for python (Warning: this is Python-specific!) 
	const T & __getitem__( int index )
	{
		return self->at(index);
	}
	void __setitem__( int index, const genericVector<T> & value )
	{
		(*self)[index] = value;
	}
    // TODO: check
    // The original at()-method makes some problems! is it because of 'inline' ?
    const T & at( int row, int col )
    {
        return self->at(row,col);
    }
    void setAt( int row, int col, const T & value )
    {
		(*self)[row][col] = value;
    }
}
namespace lti {
    %template(bgenericMatrix) genericMatrix<ubyte>;
    %template(igenericMatrix) genericMatrix<int>;
    %template(fgenericMatrix) genericMatrix<float>;
    %template(dgenericMatrix) genericMatrix<double>;
    %template(rgbPixelgenericMatrix) genericMatrix<rgbPixel>;
}
%include "ltiMatrix.h"
namespace lti {
    %template(imatrix) matrix<int>;
    %template(fmatrix) matrix<float>;
    %template(dmatrix) matrix<double>;
    %template(bmatrix) matrix<ubyte>;
    %template(rgbPixelmatrix) matrix<rgbPixel>;
}
%include "ltiHistogram.h"
//namespace lti {
//    %template(histogram) thistogram<double>;
//}

%include "ltiImage.h"
%include "ltiContour.h"

// has to be included AFTER the definition of borderPoints !!!
%include "_objectsFromMask_objectStruct.h"
#include "_objectsFromMask_objectStruct.h"

// TODO: add better tree support !
//%include "ltiTree.h"
//namespace lti {
//    //%template(tree_objectStruct) tree<objectsFromMask_objectStruct>;    // does not work because of a syntactical difference to tree<objectStruct>, unforunately is swig not so clever to handel that :-(
//    %template(tree_objectStruct) tree<objectStruct>;    
//}
////#define node tree_objectStruct_node
//%include "_tree_node.h"

%include "ltiLinearKernels.h"
namespace lti {
    %template(ikernel1D) kernel1D<int>;
    %template(fkernel1D) kernel1D<float>;
    %template(dkernel1D) kernel1D<double>;
    %template(bkernel1D) kernel1D<ubyte>;
    %template(ikernel2D) kernel2D<int>;
    %template(fkernel2D) kernel2D<float>;
    %template(dkernel2D) kernel2D<double>;
    %template(bkernel2D) kernel2D<ubyte>;
    %template(isepKernel) sepKernel<int>;
    %template(fsepKernel) sepKernel<float>;
    %template(dsepKernel) sepKernel<double>;
    %template(usepKernel) sepKernel<ubyte>;
}
%include "ltiGradientKernels.h"
namespace lti {
    // TODO %template(igradientKernelX) gradientKernelX<int>;
}
%include "ltiHessianKernels.h"
%include "ltiLaplacianKernel.h"
%include "ltiSecondDerivativeKernels.h"
namespace lti {
    %template(iandoKernelXX) andoKernelXX<int>;
    %template(iandoKernelXY) andoKernelXY<int>;
    %template(iandoKernelYY) andoKernelYY<int>;
    %template(fandoKernelXX) andoKernelXX<float>;
    %template(fandoKernelXY) andoKernelXY<float>;
    %template(fandoKernelYY) andoKernelYY<float>;
}

// TODO: ok: mit SWIG 1.3.21 !!! und SWIG 1.3.24 + VC7
%template(list_ioPoints) std::list<lti::ioPoints>;
%template(list_borderPoints) std::list<lti::borderPoints>;
%template(list_areaPoints) std::list<lti::areaPoints>;

// parameters in functor (eindeutig) umbenennen
#define parameters functor_parameters
%include "_functor_parameters.h"
%include "ltiFunctor.h"
#undef parameters

#define parameters modifier_parameters
%include "_modifier_parameters.h"
%include "ltiModifier.h"
#undef parameters

#define parameters filter_parameters
%include "_filter_parameters.h"
%include "ltiFilter.h"
#undef parameters

#define parameters ioFunctor_parameters
%include "_ioFunctor_parameters.h"
%include "ltiIOFunctor.h"
#undef parameters

#define parameters ioBMP_parameters
%include "_ioBMP_parameters.h"
%include "ltiBMPFunctor.h"
#undef parameters

#define parameters ioJPEG_parameters
%include "_ioJPEG_parameters.h"
%include "ltiJPEGFunctor.h"
#undef parameters

#define parameters ioPNG_parameters
%include "_ioPNG_parameters.h"
%include "ltiPNGFunctor.h"
#undef parameters

#define parameters ioImage_parameters
%include "_ioImage_parameters.h"
%include "ltiALLFunctor.h"
#undef parameters

#define parameters viewerBase_parameters
%include "_viewerBase_parameters.h"
%include "ltiViewerBase.h"
#undef parameters

#define parameters externViewer_parameters
%include "_externViewer_parameters.h"
%include "ltiExternViewer.h"
#undef parameters

%include "ltiSplitImage.h"
%include "ltiSplitImageTorgI.h"

#define parameters usePalette_parameters
%include "_usePalette_parameters.h"
%include "ltiUsePalette.h"
#undef parameters

#define parameters transform_parameters
%include "_transform_parameters.h"
%include "ltiTransform.h"
#undef parameters

#define parameters gradientFunctor_parameters
%include "_gradientFunctor_parameters.h"
%include "ltiGradientFunctor.h"
#undef parameters

#define parameters colorContrastGradient_parameters
%include "_colorContrastGradient_parameters.h"
%include "ltiColorContrastGradient.h"
#undef parameters

#define parameters edgeDetector_parameters
%include "_edgeDetector_parameters.h"
%include "ltiEdgeDetector.h"
#undef parameters

#define parameters classicEdgeDetector_parameters
%include "_classicEdgeDetector_parameters.h"
%include "ltiClassicEdgeDetector.h"
#undef parameters

#define parameters cannyEdges_parameters
%include "_cannyEdges_parameters.h"
%include "ltiCannyEdges.h"
#undef parameters

// TODO: problems with private class accumulator !!! --> can we solve this with generated header file out of the XML-output ?
#define parameters convolution_parameters
%include "_convolution_parameters.h"
%include "ltiConvolution.h"
#undef parameters

#define parameters segmentation_parameters
%include "_segmentation_parameters.h"
%include "ltiSegmentation.h"
#undef parameters

#define parameters regionGrowing_parameters
%include "_regionGrowing_parameters.h"
%include "ltiRegionGrowing.h"
#undef parameters

////#define objectStruct objectsFromMask_objectStruct
//%include "_objectsFromMask_objectStruct.h"
////%include "ltiObjectsFromMask.h"
////#undef objectStruct

#define parameters objectsFromMask_parameters
%include "_objectsFromMask_parameters.h"
%include "ltiObjectsFromMask.h"
#undef parameters

#define parameters polygonApproximation_parameters
%include "_polygonApproximation_parameters.h"
%include "ltiPolygonApproximation.h"
#undef parameters

#define parameters colorQuantization_parameters
%include "_colorQuantization_parameters.h"
%include "ltiColorQuantization.h"
#undef parameters

#define parameters kMColorQuantization_parameters
%include "_kMColorQuantization_parameters.h"
%include "ltiKMColorQuantization.h"
#undef parameters

#define parameters meanShiftSegmentation_parameters
%include "_meanShiftSegmentation_parameters.h"
%include "ltiMeanShiftSegmentation.h"
#undef parameters

#define parameters kMeansSegmentation_parameters
%include "_kMeansSegmentation_parameters.h"
%include "ltiKMeansSegmentation.h"
#undef parameters

%extend lti::_kMeansSegmentation::_kMeansSegmentation_parameters {
// TODO: is there a better way to support complex attributes ?
// a helper method to set complex attributes of a parameters-class
void setQuantParameters(const lti::_kMColorQuantization::kMColorQuantization_parameters & value) 
{
    self->quantParameters = value;
} 
};

#define parameters whiteningSegmentation_parameters
%include "_whiteningSegmentation_parameters.h"
%include "ltiWhiteningSegmentation.h"
#undef parameters

#define parameters csPresegmentation_parameters
%include "_csPresegmentation_parameters.h"
%include "ltiCsPresegmentation.h"
#undef parameters

#define parameters featureExtractor_parameters
%include "_featureExtractor_parameters.h"
%include "ltiFeatureExtractor.h"
#undef parameters

#define parameters globalFeatureExtractor_parameters
%include "_globalFeatureExtractor_parameters.h"
%include "ltiGlobalFeatureExtractor.h"
#undef parameters

#define parameters localFeatureExtractor_parameters
%include "_localFeatureExtractor_parameters.h"
%include "ltiLocalFeatureExtractor.h"
#undef parameters

#define parameters localMoments_parameters
%include "_localMoments_parameters.h"
%include "ltiLocalMoments.h"
#undef parameters

#define parameters geometricFeatures_parameters
%include "_geometricFeatures_parameters.h"
%include "ltiGeometricFeatures.h"
#undef parameters

#define parameters chromaticityHistogram_parameters
%include "_chromaticityHistogram_parameters.h"
%include "ltiChromaticityHistogram.h"
#undef parameters

#define parameters morphology_parameters
%include "_morphology_parameters.h"
%include "ltiMorphology.h"
#undef parameters

#define parameters dilation_parameters
%include "_dilation_parameters.h"
%include "ltiDilation.h"
#undef parameters

#define parameters erosion_parameters
%include "_erosion_parameters.h"
%include "ltiErosion.h"
#undef parameters

#define parameters distanceTransform_parameters
%rename (distanceTransform_sedMask) lti::distanceTransform::sedMask;
%include "_distanceTransform_parameters.h"
%include "ltiDistanceTransform.h"
#undef parameters

#define parameters skeleton_parameters
%include "_skeleton_parameters.h"
%include "ltiSkeleton.h"
#undef parameters

#define parameters classifier_parameters
#define outputTemplate classifier_outputTemplate
#define outputVector classifier_outputVector
%include "_classifier_outputVector.h"
%include "_classifier_outputTemplate.h"
%include "_classifier_parameters.h"
%include "ltiClassifier.h"
#undef parameters

#define parameters supervisedInstanceClassifier_parameters
//%include "_supervisedInstanceClassifier_parameters.h"
%include "ltiSupervisedInstanceClassifier.h"
#undef parameters

//#define parameters decisionTree_parameters
//%include "_decisionTree_parameters.h"
//%include "ltiDecisionTree.h"
//#undef parameters

%include "ltiSplitImageToHSI.h"
%extend lti::splitImageToHSI {
    // transfer the HSI value as a rgbPixel (TODO: maybe better as a Python tuple? How?)
    lti::rgbPixel apply( const rgbPixel &pixel )
    {
        ubyte H, S, I;
        self->apply( pixel, H, S, I );
        return lti::rgbPixel( H, S, I );
    }
/* TODO --> does not work !
    int[3] apply( const rgbPixel &pixel )
    {
        int ret[3];
        ubyte H, S, I;
        self->apply( pixel, H, S, I );
        ret[0] = H;
        ret[1] = S;
        ret[2] = I;
        return ret;
    }
*/    
}
%include "ltiSplitImageToHSV.h"
%include "ltiSplitImageToHLS.h"
%include "ltiSplitImageToRGB.h"
%include "ltiSplitImageToYUV.h"

// **************************************************************************

%include "lti_manual.h"


