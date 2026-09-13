#include "SensorCorrente.h"

float lerCorrente() {
  float soma = 0;
  for (int i = 0; i < AMOSTRAS_CORRENTE; i++) {
    float tensao  = (analogRead(PIN_ACS712) / ADC_MAX) * VCC;
    float amostra = (tensao - ACS712_OFFSET) / ACS712_SENS;
    soma += amostra * amostra;
    delayMicroseconds(100);
  }
  float rms = sqrt(soma / AMOSTRAS_CORRENTE);
  if (rms < 0.05) rms = 0.0;
  return rms;
}

bool verificarACS712() {
  int adcVal = analogRead(PIN_ACS712);
  return (adcVal > 350 && adcVal < 680);
}
