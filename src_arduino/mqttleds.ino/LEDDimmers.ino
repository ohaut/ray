#include <Arduino.h>
#include "LEDDimmers.h"


#define ANALOG_RANGE 1000
#define BOOT_VALUE 1000
#define BOOT_VALUE_F 1.0

int DIMMER_PIN[] = {14, 5, 15};

void LEDDimmers::setup() {
  analogWriteRange(ANALOG_RANGE);
  for (int i=0; i<N_DIMMERS; i++) {
    
    analogWrite(DIMMER_PIN[i],
                BOOT_VALUE);
    pinMode(DIMMER_PIN[i], OUTPUT);
   _dimmers[i] = BOOT_VALUE_F;  
  }
}

void LEDDimmers::halt()
{
  for (int i=0; i<N_DIMMERS; i++) {
    analogWrite(DIMMER_PIN[i], 0);   
  }
}

void LEDDimmers::restart() {
  for (int i=0; i<N_DIMMERS; i++) {
    setDimmer(i, _dimmers[i]);
  }
}

void LEDDimmers::setGamma(float gamma) {
  _gamma = gamma;
}

void LEDDimmers::setDimmer(int n, float value) {

   if (!(n>=0 && n<N_DIMMERS)) 
      return;

   if (value<0.0) value = 0.0;
   if (value>1.0) value = 1.0;

   analogWrite(DIMMER_PIN[n],
               pow(value, _gamma)*float(ANALOG_RANGE));

   _dimmers[n] = value;  
}

float LEDDimmers::getDimmer(int n) {
  if (n>=0 && n<N_DIMMERS)
    return _dimmers[n];

  return -1;
}
