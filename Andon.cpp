#include "Andon.h"

void andonInit() {
  const uint8_t pinos[] = { PIN_ANDON_VERDE, PIN_ANDON_AMARELO, PIN_ANDON_VERMELHO };
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(pinos[i], HIGH);   // antes do OUTPUT: relé desligado desde o 1º instante
    pinMode(pinos[i], OUTPUT);
  }
}

// Relé ativo em LOW: ligar = LOW, desligar = HIGH
void setAndon(EstadoAndon estado) {
  digitalWrite(PIN_ANDON_VERDE,    estado == ANDON_BOM     ? LOW : HIGH);
  digitalWrite(PIN_ANDON_AMARELO,  estado == ANDON_DEFEITO ? LOW : HIGH);
  digitalWrite(PIN_ANDON_VERMELHO, estado == ANDON_GRAVE   ? LOW : HIGH);
}

EstadoAndon estabilizarAndon(EstadoAndon novo) {
  static EstadoAndon conf = ANDON_BOM;
  static int         n    = 0;

  if (novo == ANDON_APAGADO) { conf = ANDON_BOM; n = 0; return novo; }
  if (novo >= conf) { conf = novo; n = 0; }
  else if (++n >= ANDON_CICLOS_CONFIRMACAO_MELHORA) { conf = novo; n = 0; }
  return conf;
}

const __FlashStringHelper* andonParaTexto(EstadoAndon estado) {
  static const __FlashStringHelper* const nomes[] = { F("BOM"), F("DEFEITO"), F("GRAVE"), F("APAGADO") };
  return nomes[estado];
}
