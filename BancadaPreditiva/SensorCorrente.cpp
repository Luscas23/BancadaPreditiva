#include "SensorCorrente.h"
#include "Configuracao.h"
#include "config.h"

float lerCorrente() {
  long s = 0, s2 = 0;
  for (int i = 0; i < cfg.acsAmostras; i++) {
    int a = analogRead(cfg.pinAcs);
    s  += a;
    s2 += (long)a * a;
    delayMicroseconds(cfg.acsIntervaloUs);
  }
  float m   = (float)s / cfg.acsAmostras;
  float var = (float)s2 / cfg.acsAmostras - m * m;
  if (var < 0) var = 0;   // arredondamento de float com sinal constante daria NaN
  float rms = sqrt(var) * cfg.acsVcc / ADC_MAX / cfg.acsSens;
  return rms < cfg.acsPisoA ? 0 : rms;
}

bool verificarACS712() {
  int adcVal = analogRead(cfg.pinAcs);
  return adcVal > cfg.acsAdcMin && adcVal < cfg.acsAdcMax;
}
