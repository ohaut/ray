#include <Arduino.h>
#include "LEDDimmers.h"


#if defined(ESP8266)

const int ANALOG_RANGE=1000;

int DIMMER_PIN[] = {14, 5, 15};

#elif defined(ESP32)

int DIMMER_PIN[] = {8, 10, 7};

#define PWM_FREQ 20000
#define PWM_RESOLUTION 10
const int ANALOG_RANGE = (1 << PWM_RESOLUTION) - 1;

#endif

#define BOOT_VALUE ANALOG_RANGE
#define BOOT_VALUE_F 1.0

void ticker_update(LEDDimmers *cls) {
  cls->update();
}

void LEDDimmers::setup(float* boot_values, int all_mode) {

#if defined(ESP8266)
  analogWriteRange(ANALOG_RANGE);
#endif
  float _min = 1.0;
  float _max = 0.0;

  for (int i=0; i<N_DIMMERS; i++) {

    float boot_value = boot_values?boot_values[i]:1.0;
    if (boot_value > _max) _max = boot_value;
    if (boot_value > 0.0 && boot_value < _min) _min = boot_value;

    float analog_val = pow(boot_value,
                           _gamma)*float(ANALOG_RANGE);

    #if defined(ESP8266)
    pinMode(DIMMER_PIN[i], OUTPUT);
    analogWrite(DIMMER_PIN[i], analog_val);
    #elif defined(ESP32)
    // use a 10mA drive strength for the dimmer pins
    gpio_set_drive_capability((gpio_num_t)DIMMER_PIN[i], GPIO_DRIVE_CAP_0);

    ledcSetup(i, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(DIMMER_PIN[i], i);
    ledcWrite(i, analog_val);
    #endif

   _dimmers[i] = boot_value;
   _dimmers_val[i] = boot_value;
  }

  /* Calculate the proportional values for the "all" channel based on the mode */
  float proportion;
  if (all_mode == ALL_MODE_PROPORTIONAL) {
      proportion = 1.0 / _max;
      _all = _max;
  } else {
      proportion = 1.0 / _min;
      _all = _min;
  }
  bool all_0 = true;

  // if all dimmers are set to 0 on boot, the proportional calculation won't work
  // TODO: we need a better way of handling this, i.e. having a "switched off" flag.
  for (int i=0; i<N_DIMMERS; i++) {
    if (_dimmers[i] > 0.0) {
      all_0 = false;
      break;
    }
  }
  
  for (int i=0; i<N_DIMMERS; i++) {
    if (all_0) {
      _all_prop[i] = 1.0;
    } else {
      _all_prop[i] = _dimmers[i] * proportion;
    }
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

const float min_step = 1.0/float(ANALOG_RANGE);

void LEDDimmers::update() {

  for (int n=0; n<N_DIMMERS; n++) {
    float step = (_dimmers[n]-_dimmers_val[n])*DIMMER_STEP;
     if (_dimmers_val[n]!=_dimmers[n]) {
        float fdiff = fabs(_dimmers_val[n] - _dimmers[n]);
        if (fdiff<=min_step)
          _dimmers_val[n] = _dimmers[n];
        else
          _dimmers_val[n] += step;
        #if defined(ESP8266)
        analogWrite(DIMMER_PIN[n],
                    pow(_dimmers_val[n], _gamma)*float(ANALOG_RANGE));
        #elif defined(ESP32)
        int val = pow(_dimmers_val[n], _gamma)*float(ANALOG_RANGE);
        ledcWrite(n, val);
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

void LEDDimmers::setAll(float value) {
  _all = value;
  for (int i=0; i<N_DIMMERS; i++) {
    setDimmer(i, _all_prop[i] * value);
  }
}

float LEDDimmers::getAll() {
  return _all;
}