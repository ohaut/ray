#ifndef __LED_DIMMERS_H
#define __LED_DIMMERS_H

#define N_DIMMERS 3

class LEDDimmers {
private:
  float _gamma = 1.5;
  float _dimmers[N_DIMMERS];
public:
  void setup();
  void setGamma(float gamma);
  void setDimmer(int n, float value);  
  float getDimmer(int n);
  void halt();
  void restart();
};

#endif
