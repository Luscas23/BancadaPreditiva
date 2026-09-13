#ifndef SENSOR_CORRENTE_H
#define SENSOR_CORRENTE_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  ACS712-5A — Sensor de corrente
// ----------------------------------------------------------------
#define PIN_ACS712         A0    // Corrente ACS712-5A

#define ACS712_SENS        0.185
#define ACS712_OFFSET      2.5    // fallback nominal — usado só se a
                                   // calibração em verificarACS712()
                                   // não rodar ou vier fora da faixa
                                   // plausível (sensor ausente/com defeito)
#define VCC                5.0
#define ADC_MAX            1023.0
// 400 amostras * (~110us analogRead + 100us delay) ~= 84ms ~= 5 ciclos
// de 60Hz. Antes eram 100 amostras (~21ms ~= 1,2 ciclo): a fração de
// ciclo sobrando pesava ~21% da janela e deixava o RMS sensível à fase
// inicial da amostragem. Com 5 ciclos a fração sobrante pesa ~0,8%,
// bem mais estável. Custo: lerCorrente() bloqueia ~84ms em vez de
// ~21ms, irrelevante perto do delay(500) já existente no loop().
#define AMOSTRAS_CORRENTE  400

// Offset real deste ACS712, calibrado em verificarACS712() (Fase 1,
// motor ainda parado -> corrente ~0A -> a tensão lida ali É o offset
// real da unidade, que varia ~2.45-2.55V de sensor pra sensor).
// Começa em ACS712_OFFSET e só é atualizado se a leitura de
// calibração passar no mesmo teste de sanidade que já existia.
extern float acs712OffsetCalibrado;

// Leitura RMS da corrente (corrigido v5: média simples em CA tende a
// zero; RMS é o correto para corrente alternada)
float lerCorrente();

// Verificação de presença/sanidade do sensor na Fase 1 — checa se o
// ADC em repouso está dentro da faixa esperada em torno do offset
// (2.5V nominal) e, se estiver, usa essa mesma leitura pra calibrar
// acs712OffsetCalibrado (mesma ideia do repouso do SW-420: aproveitar
// uma leitura que já ia ser feita mesmo, em vez de confiar num valor
// nominal fixo). NÃO corrige deriva térmica durante a operação — só
// a variação fixa de fábrica entre unidades, capturada uma vez no boot.
bool verificarACS712();

#endif
