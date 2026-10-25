#ifndef SENSOR_CORRENTE_H
#define SENSOR_CORRENTE_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  ACS712-5A — Sensor de corrente
// ----------------------------------------------------------------
#define PIN_ACS712         A0

#define ACS712_SENS        0.185
#define VCC                5.0
#define ADC_MAX            1023.0
#define AMOSTRAS_CORRENTE  400    // ~84 ms = 5 ciclos de 60 Hz

// Corrente RMS. O offset sai da média da própria janela (sem calibração).
float lerCorrente();

// Sanidade: ADC em repouso perto de 2,5 V
bool verificarACS712();

#endif
