#ifndef SENSOR_VIBRACAO_H
#define SENSOR_VIBRACAO_H

#include <Arduino.h>

#define PIN_SW420_1        2     // Vibração faixa 1  (INT0 — defeito)
#define PIN_SW420_2        3     // Vibração faixa 2  (INT1 — grave)
#define DEBOUNCE_MS        50

// Flags setadas pelas ISRs. O loop() principal deve capturar um
// snapshot atômico (noInterrupts()/interrupts()) antes de usá-las em
// avaliarEstado(), e zerá-las somente depois de exibidas no display.
extern volatile bool vibr1;
extern volatile bool vibr2;

// Timestamp (millis) da última vibração detectada em cada faixa,
// usado pelo display para mostrar "há quantos segundos"
extern unsigned long ultimaVibr1Ms;
extern unsigned long ultimaVibr2Ms;

// Configura os pinos como entrada, detecta o nível de repouso de cada
// sensor e liga a interrupção na borda que SAI do repouso (RISING se
// repouso=LOW, FALLING se repouso=HIGH) — sempre de borda única,
// nunca CHANGE (CHANGE causa duplo disparo por vibração). Chamar uma
// vez no setup().
void vibracaoInit();

// Verificação de presença/sanidade da Fase 1: confirma que o nível de
// repouso detectado em vibracaoInit() está ESTÁVEL, lendo o pino
// várias vezes seguidas — um pino sem sensor conectado (flutuando)
// tende a variar entre leituras por ruído, enquanto um sensor real
// mantém o mesmo nível em repouso. Não é prova definitiva de presença
// do sensor (um pino preso externamente a um nível fixo também
// passaria), mas detecta o caso mais comum de falha: sensor ausente
// ou desconectado. Chamar durante a Fase 1, depois de vibracaoInit().
void verificarVibracao(bool &ok1, bool &ok2);

#endif
