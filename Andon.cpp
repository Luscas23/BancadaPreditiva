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

// Passo 12 — ver comentário em Andon.h. `destino` não muda o texto
// hoje (Serial e SD usam as mesmas 3 palavras); parâmetro nomeado
// mas não usado no switch, então marcado (void) pra não gerar aviso
// de "parâmetro não usado" na compilação.
const __FlashStringHelper* andonParaTexto(EstadoAndon estado, Destino destino) {
  (void)destino;
  switch (estado) {
    case ANDON_BOM:     return F("BOM");
    case ANDON_DEFEITO: return F("DEFEITO");
    case ANDON_GRAVE:   return F("GRAVE");
  }
  return F("");  // enum bem formado nunca cai aqui
}
