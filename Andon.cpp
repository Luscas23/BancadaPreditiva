#include <Arduino.h>
#include "Andon.h"

void andonInit() {
  pinMode(PIN_ANDON_VERDE,    OUTPUT);
  pinMode(PIN_ANDON_AMARELO,  OUTPUT);
  pinMode(PIN_ANDON_VERMELHO, OUTPUT);
}

void setAndon(EstadoAndon estado) {
  digitalWrite(PIN_ANDON_VERDE,    estado == ANDON_BOM     ? HIGH : LOW);
  digitalWrite(PIN_ANDON_AMARELO,  estado == ANDON_DEFEITO ? HIGH : LOW);
  digitalWrite(PIN_ANDON_VERMELHO, estado == ANDON_GRAVE   ? HIGH : LOW);
}

void piscarAndon(EstadoAndon estado, int vezes, int ms) {
  for (int i = 0; i < vezes; i++) {
    setAndon(estado);    delay(ms);
    setAndon(ANDON_BOM); delay(ms / 2);
  }
}
