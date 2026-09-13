#ifndef ANDON_H
#define ANDON_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  ANDON — Torre de sinalização (Verde / Amarelo / Vermelho)
// ----------------------------------------------------------------
#define PIN_ANDON_VERDE    22
#define PIN_ANDON_AMARELO  24
#define PIN_ANDON_VERMELHO 26

typedef enum {
  ANDON_BOM,
  ANDON_DEFEITO,
  ANDON_GRAVE
} EstadoAndon;

// Configura os pinos da torre como saída (chamar uma vez no setup)
void andonInit();

// Acende o LED correspondente ao estado, apaga os demais
void setAndon(EstadoAndon estado);

// Pisca a torre em um estado "vezes" vezes, intercalando com ANDON_BOM
void piscarAndon(EstadoAndon estado, int vezes, int ms);

#endif
