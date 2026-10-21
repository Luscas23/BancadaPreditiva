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

EstadoAndon estabilizarAndon(EstadoAndon novoEstado) {
  static EstadoAndon estadoConfirmado     = ANDON_BOM;
  static int         ciclosMelhorSeguidos = 0;

  if (novoEstado >= estadoConfirmado) {
    // Piora ou mantém: reage na hora. EstadoAndon é 0=BOM..2=GRAVE,
    // então ">=" cobre tanto "piorou" quanto "ficou igual".
    estadoConfirmado     = novoEstado;
    ciclosMelhorSeguidos = 0;
  } else {
    // Melhora: só confirma após N ciclos seguidos melhores que o
    // estado atual. Não exige que sejam todos o MESMO estado-alvo —
    // se oscilar entre DEFEITO e BOM antes de completar N, ainda
    // conta como "seguido melhorando" e confirma o que a leitura
    // disser no ciclo em que o contador fechar (comportamento
    // deliberadamente simples; não é um controle de segurança).
    ciclosMelhorSeguidos++;
    if (ciclosMelhorSeguidos >= ANDON_CICLOS_CONFIRMACAO_MELHORA) {
      estadoConfirmado     = novoEstado;
      ciclosMelhorSeguidos = 0;
    }
  }

  return estadoConfirmado;
}

const __FlashStringHelper* andonParaTexto(EstadoAndon estado) {
  static const __FlashStringHelper* const nomes[] = { F("BOM"), F("DEFEITO"), F("GRAVE") };
  return nomes[estado];
}
