 /******************************************/  
/*  Karplus-Strong drone string model   */
/*  by Perry Cook, 1995-96                */
/*					  */
/*  There exist at least two patents,     */
/*  assigned to Stanford, bearing the     */
/*  names of Karplus and/or Strong.       */
/******************************************/

#include "drone.h"

drone :: drone(MY_FLOAT lowestFreq)
{
  length = (long) (SRATE / lowestFreq + 1);
  loopGain = (MY_FLOAT) 0.999;
  loopFilt = new OneZero();
  delayLine = new DLineA(length);
  envelope = new ADSR();
  noise = new Noise;
  envelope->setAllTimes(2.0,0.5,0.0,0.5);
  this->clear();
}

drone :: ~drone()
{
  delete loopFilt;
  delete delayLine;
  delete envelope;
  delete noise;
}

void drone :: clear()
{
  loopFilt->clear();
  delayLine->clear();
}

void drone :: setFreq(MY_FLOAT frequency)
{
  MY_FLOAT delay;
  delay = (SRATE / frequency);
  delayLine->setDelay(delay - 0.5);
  loopGain = (MY_FLOAT) 0.997 + (frequency * (MY_FLOAT)  0.000002);
  if (loopGain>1.0) loopGain = (MY_FLOAT) 0.99999;
}

void drone :: pluck(MY_FLOAT amplitude)
{
  envelope->keyOn();
}

void drone :: noteOn(MY_FLOAT freq, MY_FLOAT amp)
{
  this->setFreq(freq);
  this->pluck(amp);
#if defined(_debug_)        
  printf("drone : NoteOn: Freq=%lf Amp=%lf\n",freq,amp);
#endif    
}

void drone :: noteOff(MY_FLOAT amp)
{
  loopGain = (MY_FLOAT) 1.0 - amp;
#if defined(_debug_)        
  printf("drone : NoteOff: Amp=%lf\n",amp);
#endif    
}

MY_FLOAT drone :: tick()
{
  /* check this out */
  /* here's the whole inner loop of the instrument!!  */
  lastOutput = delayLine->tick(loopFilt->tick((delayLine->lastOut() * loopGain))
		+ (0.005 * envelope->tick() * noise->tick())); 
  return lastOutput;
}

