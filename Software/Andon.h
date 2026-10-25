#ifndef ANDON_H
#define ANDON_H

#include <Arduino.h>

// Torre de sinalização (Verde / Amarelo / Vermelho). Pinos e nível ativo vêm de cfg.

// APAGADO fica por último: a histerese só compara BOM..GRAVE
typedef enum {
  ANDON_BOM,
  ANDON_DEFEITO,
  ANDON_GRAVE,
  ANDON_APAGADO     // motor parado: nenhum relé ligado
} EstadoAndon;

// Relés desligados antes de virar saída (não estalam no boot). Pode ser chamada
// de novo após carregar o CONFIG.TXT: pinos antigos ficam como entrada.
void andonInit();
void setAndon(EstadoAndon estado);

// Histerese: piora na hora, melhora só após cfg.andonHisterese ciclos seguidos melhores.
// APAGADO zera a histerese. Chamar uma vez por ciclo do loop().
EstadoAndon estabilizarAndon(EstadoAndon novoEstado);

const __FlashStringHelper* andonParaTexto(EstadoAndon estado);

#endif
