#ifndef SENSOR_CORRENTE_H
#define SENSOR_CORRENTE_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  ACS712-5A — Sensor de corrente
// ----------------------------------------------------------------
// Pino, sensibilidade, VCC e amostragem vêm de cfg (ADC_MAX em config.h).

// Corrente RMS. O offset sai da média da própria janela (sem calibração).
float lerCorrente();

// Sanidade: ADC em repouso perto de 2,5 V
bool verificarACS712();

#endif
