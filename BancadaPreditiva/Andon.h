#ifndef ANDON_H
#define ANDON_H

#include <Arduino.h>

typedef enum {
  ANDON_BOM,
  ANDON_DEFEITO,
  ANDON_GRAVE,
  ANDON_APAGADO
} EstadoAndon;

void andonInit();
void setAndon(EstadoAndon estado);

EstadoAndon estabilizarAndon(EstadoAndon novoEstado);

const __FlashStringHelper* andonParaTexto(EstadoAndon estado);

#endif
