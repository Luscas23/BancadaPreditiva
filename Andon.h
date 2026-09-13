// ================================================================
//  MÓDULO ANDON — Torre de sinalização (Verde / Amarelo / Vermelho)
//  Responsabilidade única: acender/piscar as cores da torre.
//  Não conhece sensores, LCD, SD nem regras de negócio — apenas
//  recebe um EstadoAndon já decidido e traduz em digitalWrite().
// ================================================================
#ifndef ANDON_H
#define ANDON_H

// ----------------------------------------------------------------
//  PINOS — Arduino Mega 2560
// ----------------------------------------------------------------
#define PIN_ANDON_VERDE    22
#define PIN_ANDON_AMARELO  24
#define PIN_ANDON_VERMELHO 26

// ----------------------------------------------------------------
//  Estado do Andon (usado também pela lógica de avaliação e pelo SD)
// ----------------------------------------------------------------
typedef enum {
  ANDON_BOM,
  ANDON_DEFEITO,
  ANDON_GRAVE
} EstadoAndon;

// Configura os pinos da torre como saída. Chamar uma vez, no setup().
void andonInit();

// Acende a cor correspondente ao estado e apaga as demais.
void setAndon(EstadoAndon estado);

// Pisca a cor do estado 'vezes' vezes, com 'ms' de intervalo,
// voltando para ANDON_BOM entre os piscados (comportamento idêntico
// ao original: usado nas fases de verificação e countdown).
void piscarAndon(EstadoAndon estado, int vezes, int ms);

#endif
