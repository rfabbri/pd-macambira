/******************************************/  
/*  Karplus-Strong Sitar1 string model   */
/*  by Perry Cook, 1995-96                */
/*                                        */
/*  There exist at least two patents,     */
/*  assigned to Stanford, bearing the     */
/*  names of Karplus and/or Strong.       */
/******************************************/

#if !defined(__sitar_h)
#define __sitar_h

#include "Instrmnt.h" 
#include "DLineA.h"
#include "OneZero.h"
#include "ADSR.h" 
#include "Noise.h" 
#include "Object.h" 

class sitar : public Instrmnt
{
protected:  
  DLineA *delayLine;
  OneZero *loopFilt;
  ADSR *envelope;
  Noise *noise;
  long length;
  MY_FLOAT loopGain;
  MY_FLOAT amPluck;
  MY_FLOAT delay;
  MY_FLOAT delayTarg;
public:
  sitar(MY_FLOAT lowestFreq);
  ~sitar();
  void clear();
  virtual void setFreq(MY_FLOAT frequency);
  void pluck(MY_FLOAT amplitude);
  virtual void noteOn(MY_FLOAT freq, MY_FLOAT amp);
  virtual void noteOff(MY_FLOAT amp);
  virtual MY_FLOAT tick();
};

#endif

