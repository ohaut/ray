#include <Arduino.h>
#include "LEDDimmers.h"


#if defined(ESP8266)

const int ANALOG_RANGE=1000;

int DIMMER_PIN[] = {14, 5, 15};


#elif defined(ESP32)

int DIMMER_PIN[] = {LED_BUILTIN, 12, 14};

#define PWM_FREQ 5000
#define PWM_RESOLUTION 13
const int ANALOG_RANGE = 8191;

#endif

#define BOOT_VALUE ANALOG_RANGE
#define BOOT_VALUE_F 1.0

void ticker_update(LEDDimmers *cls) {
  cls->update();
}

void LEDDimmers::setup(float* boot_values) {

#if defined(ESP8266)
  analogWriteRange(ANALOG_RANGE);
#endif

  for (int i=0; i<N_DIMMERS; i++) {
    #ifdef ESP32
    ledcSetup(i, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(DIMMER_PIN[i], i);
    #endif

    float boot_value = boot_values?boot_values[i]:1.0;
    float analog_val = pow(boot_value,
                           _gamma)*float(ANALOG_RANGE);

    #if defined(ESP8266)
    pinMode(DIMMER_PIN[i], OUTPUT);
    analogWrite(DIMMER_PIN[i], analog_val);
    #elif defined(ESP32)
    ledcWrite(i, analog_val);
    #endif
   _dimmers[i] = boot_value;
   _dimmers_val[i] = boot_value;
  }
  update_ticker.attach(DIMMER_PERIOD, ticker_update, this);
}

void LEDDimmers::halt()
{
/* ESP8266 PWM needs to be stopped during updates, etc.. */
#if defined(ESP8266)
  for (int i=0; i<N_DIMMERS; i++) {
    analogWrite(DIMMER_PIN[i], 0);
  }
#endif
  update_ticker.detach();
}

void LEDDimmers::restart() {
  for (int i=0; i<N_DIMMERS; i++) {
    setDimmer(i, _dimmers[i]);
  }
  update_ticker.attach(DIMMER_PERIOD, ticker_update, this);
}

void LEDDimmers::setGamma(float gamma) {
  _gamma = gamma;
}

void LEDDimmers::update() {

  for (int n=0; n<N_DIMMERS; n++) {
    float step = (_dimmers[n]-_dimmers_val[n])*DIMMER_STEP;
     if (_dimmers_val[n]!=_dimmers[n]) {
        if (fabs(_dimmers_val[n] - _dimmers[n])<=step)
          _dimmers_val[n] = _dimmers[n];
        else
          _dimmers_val[n] += step;
        #if defined(ESP8266)
        analogWrite(DIMMER_PIN[n],
                    pow(_dimmers_val[n], _gamma)*float(ANALOG_RANGE));
        #elif defined(ESP32)
        ledcWrite(n, pow(_dimmers_val[n], _gamma)*float(ANALOG_RANGE));
        #endif
     }
  }
}

void LEDDimmers::setDimmer(int n, float value) {

   if (!(n>=0 && n<N_DIMMERS))
      return;

   if (value<0.0) value = 0.0;
   if (value>1.0) value = 1.0;

   _dimmers[n] = value;
}

float LEDDimmers::getDimmer(int n) {
  if (n>=0 && n<N_DIMMERS)
    return _dimmers[n];

  return -1;
}
