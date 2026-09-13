#ifndef SENSOR_VIBRACAO_H
#define SENSOR_VIBRACAO_H

#include <Arduino.h>

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

// Correção — verificação de presença/sanidade da Fase 1 (antes era
// chamada em BancadaPreditiva.ino mas nunca existia, o que impedia a
// compilação). Confirma que o nível de repouso detectado em
// vibracaoInit() (sw420_1_repouso/sw420_2_repouso) está ESTÁVEL,
// lendo o pino várias vezes seguidas: um pino sem sensor conectado
// (flutuando) tende a variar entre leituras por ruído, enquanto um
// sensor real mantém o mesmo nível em repouso. Cada faixa é reportada
// separadamente via parâmetro de saída, no mesmo padrão de
// contarErros() (LogicaAvaliacao.h). Chamar durante a Fase 1, depois
// de vibracaoInit() já ter rodado (ordem já garantida no setup()).
void verificarVibracao(bool &ok1, bool &ok2);

#endif
