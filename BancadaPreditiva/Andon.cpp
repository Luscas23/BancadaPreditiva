#include "Andon.h"
#include "Configuracao.h"

static uint8_t pinos[3] = { 255, 255, 255 };   // verde, amarelo, vermelho em uso (255 = nenhum)

void andonInit() {
  const uint8_t novos[3] = { cfg.pinReleVerde, cfg.pinReleAmarelo, cfg.pinReleVermelho };
  for (uint8_t i = 0; i < 3; i++) {   // solta pino antigo que não é mais usado
    bool usado = false;
    for (uint8_t j = 0; j < 3; j++) if (pinos[i] == novos[j]) usado = true;
    if (pinos[i] != 255 && !usado) pinMode(pinos[i], INPUT);
  }
  for (uint8_t i = 0; i < 3; i++) {
    pinos[i] = novos[i];
    digitalWrite(pinos[i], cfg.releAtivoEm ^ 1);   // antes do OUTPUT: relé desligado desde o 1º instante
    pinMode(pinos[i], OUTPUT);
  }
}

// Ligar = nível ativo, desligar = o oposto
void setAndon(EstadoAndon estado) {
  const uint8_t ativo = cfg.releAtivoEm;
  digitalWrite(pinos[0], estado == ANDON_BOM     ? ativo : ativo ^ 1);
  digitalWrite(pinos[1], estado == ANDON_DEFEITO ? ativo : ativo ^ 1);
  digitalWrite(pinos[2], estado == ANDON_GRAVE   ? ativo : ativo ^ 1);
}

EstadoAndon estabilizarAndon(EstadoAndon novo) {
  static EstadoAndon conf = ANDON_BOM;
  static int         n    = 0;

  if (novo == ANDON_APAGADO) { conf = ANDON_BOM; n = 0; return novo; }
  if (novo >= conf) { conf = novo; n = 0; }
  else if (++n >= cfg.andonHisterese) { conf = novo; n = 0; }
  return conf;
}

const __FlashStringHelper* andonParaTexto(EstadoAndon estado) {
  static const __FlashStringHelper* const nomes[] = { F("BOM"), F("DEFEITO"), F("GRAVE"), F("APAGADO") };
  return nomes[estado];
}
