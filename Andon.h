#ifndef ANDON_H
#define ANDON_H

#include <Arduino.h>
#include "Destino.h"

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

// Passo 12 — único lugar que sabe o nome de cada EstadoAndon. Serial
// e SD chamam esta função em vez de terem cada um o próprio switch.
// Hoje as duas usam a mesma palavra pros 3 estados, mas a assinatura
// já recebe o destino pra não precisar mudar se algum dia divergir.
const __FlashStringHelper* andonParaTexto(EstadoAndon estado, Destino destino);

#endif
