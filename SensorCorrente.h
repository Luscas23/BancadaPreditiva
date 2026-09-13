#ifndef SENSOR_CORRENTE_H
#define SENSOR_CORRENTE_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  ACS712-5A — Sensor de corrente
// ----------------------------------------------------------------
#define PIN_ACS712         A0    // Corrente ACS712-5A

#define ACS712_SENS        0.185
#define ACS712_OFFSET      2.5
#define VCC                5.0
#define ADC_MAX            1023.0
// 400 amostras * (~110us analogRead + 100us delay) ~= 84ms ~= 5 ciclos
// de 60Hz. Antes eram 100 amostras (~21ms ~= 1,2 ciclo): a fração de
// ciclo sobrando pesava ~21% da janela e deixava o RMS sensível à fase
// inicial da amostragem. Com 5 ciclos a fração sobrante pesa ~0,8%,
// bem mais estável. Custo: lerCorrente() bloqueia ~84ms em vez de
// ~21ms, irrelevante perto do delay(500) já existente no loop().
#define AMOSTRAS_CORRENTE  400

// Leitura RMS da corrente (corrigido v5: média simples em CA tende a
// zero; RMS é o correto para corrente alternada)
float lerCorrente();

// Verificação de presença/sanidade do sensor na Fase 1 — checa se o
// ADC em repouso está dentro da faixa esperada em torno do offset (2.5V)
bool verificarACS712();

#endif
