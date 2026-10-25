#include "SensorCorrente.h"

float lerCorrente() {
  long s = 0, s2 = 0;
  for (int i = 0; i < AMOSTRAS_CORRENTE; i++) {
    int a = analogRead(PIN_ACS712);
    s  += a;
    s2 += (long)a * a;
    delayMicroseconds(100);
  }
  float m   = (float)s / AMOSTRAS_CORRENTE;
  float var = (float)s2 / AMOSTRAS_CORRENTE - m * m;
  if (var < 0) var = 0;   // arredondamento de float com sinal constante daria NaN
  float rms = sqrt(var) * VCC / ADC_MAX / ACS712_SENS;
  return rms < 0.05 ? 0 : rms;
}

bool verificarACS712() {
  int adcVal = analogRead(PIN_ACS712);
  return adcVal > 350 && adcVal < 680;
}
