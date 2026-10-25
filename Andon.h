#ifndef ANDON_H
#define ANDON_H

#include <Arduino.h>

// Torre de sinalização (Verde / Amarelo / Vermelho). Relé ativo em LOW.
#define PIN_ANDON_VERDE    22
#define PIN_ANDON_AMARELO  24
#define PIN_ANDON_VERMELHO 26

// APAGADO fica por último: a histerese só compara BOM..GRAVE
typedef enum {
  ANDON_BOM,
  ANDON_DEFEITO,
  ANDON_GRAVE,
  ANDON_APAGADO     // motor parado: nenhum relé ligado
} EstadoAndon;

// Relés desligados antes de virar saída (não estalam no boot)
void andonInit();
void setAndon(EstadoAndon estado);

// Histerese: piora na hora, melhora só após N ciclos seguidos melhores.
// APAGADO zera a histerese. Chamar uma vez por ciclo do loop().
#define ANDON_CICLOS_CONFIRMACAO_MELHORA 3
EstadoAndon estabilizarAndon(EstadoAndon novoEstado);

const __FlashStringHelper* andonParaTexto(EstadoAndon estado);

#endif
