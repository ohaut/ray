#ifndef __LED_DIMMERS_H
#define __LED_DIMMERS_H

#define N_DIMMERS 3

#define DIMMER_STEP   0.05
#define DIMMER_PERIOD 0.02

#define ALL_MODE_PROPORTIONAL 0
#define ALL_MODE_MAXOUT 1

class LEDDimmers {
private:
  float _gamma = 1.5;
  float _dimmers[N_DIMMERS];
  float _dimmers_val[N_DIMMERS];
  float _all_prop[N_DIMMERS];
  float _all;

  Ticker update_ticker;

public:
  void  setup(float* boot_values=NULL, int all_mode=ALL_MODE_PROPORTIONAL);
  void  update();
  void  setGamma(float gamma);

  void  setDimmer(int n, float value);
  float getDimmer(int n);

  void  setAll(float value);
  float getAll();

  void  halt();
  void  restart();
};

#endif
