#ifndef SENSOR_VIBRACAO_H
#define SENSOR_VIBRACAO_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  VIBRAÇÃO — 2x SW-420 (faixa 1 = defeito, faixa 2 = grave)
// ----------------------------------------------------------------
#define PIN_SW420_1        2     // Vibração faixa 1  (INT0 — defeito)
#define PIN_SW420_2        3     // Vibração faixa 2  (INT1 — grave)
#define DEBOUNCE_MS        50

// Flags setadas pelas ISRs. O loop() principal deve capturar um
// snapshot atômico (noInterrupts()/interrupts()) antes de usá-las em
// contarErros()/avaliarEstado(), e zerá-las somente depois de exibidas
// no display (ver Melhoria 1 no .ino) — esse contrato não muda com a
// extração do módulo.
extern volatile bool vibr1;
extern volatile bool vibr2;

// Melhoria 7 — timestamp (millis) da última vibração detectada em cada
// faixa, usado pelo display para mostrar "há quantos segundos"
extern unsigned long ultimaVibr1Ms;
extern unsigned long ultimaVibr2Ms;

// Melhoria 3 — estado de repouso de cada sensor, detectado em
// vibracaoInit() e usado ali mesmo pra escolher a borda de disparo da
// interrupção (RISING se repouso=LOW, FALLING se repouso=HIGH).
// Exposto aqui também pra quem quiser inspecionar a polaridade
// detectada (ex.: diagnóstico/log).
extern int sw420_1_repouso;
extern int sw420_2_repouso;

// Configura os pinos como entrada, detecta o nível de repouso de cada
// sensor (Melhoria 3) e liga a interrupção na borda que SAI do
// repouso — sempre de borda única, nunca CHANGE (corrigido na v5:
// CHANGE causava duplo disparo por vibração). Chamar uma vez no
// setup().
void vibracaoInit();

#endif
