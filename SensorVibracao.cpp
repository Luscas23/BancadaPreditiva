#include "SensorVibracao.h"

volatile bool vibr1 = false;
volatile bool vibr2 = false;

// Debounce é detalhe interno do módulo — não precisa ser visto de fora
static unsigned long ultimoDebounce1 = 0;
static unsigned long ultimoDebounce2 = 0;

unsigned long ultimaVibr1Ms = 0;
unsigned long ultimaVibr2Ms = 0;

int sw420_1_repouso = LOW;
int sw420_2_repouso = LOW;

static void ISR_vibr1() {
  unsigned long agora = millis();
  if (agora - ultimoDebounce1 > DEBOUNCE_MS) {
    vibr1           = true;
    ultimaVibr1Ms   = agora;   // Melhoria 7
    ultimoDebounce1 = agora;
  }
}

static void ISR_vibr2() {
  unsigned long agora = millis();
  if (agora - ultimoDebounce2 > DEBOUNCE_MS) {
    vibr2           = true;
    ultimaVibr2Ms   = agora;   // Melhoria 7
    ultimoDebounce2 = agora;
  }
}

void vibracaoInit() {
  pinMode(PIN_SW420_1, INPUT);
  pinMode(PIN_SW420_2, INPUT);

  // Melhoria 3 — detecta o nível de repouso de cada sensor e usa isso
  // pra escolher a borda de disparo: dispara sempre que o sinal SAI do
  // repouso, em vez de assumir de antemão que repouso = LOW. Cobre o
  // caso de um SW-420 (ou fiação) que fique em nível alto parado.
  // Continua de borda única (nunca CHANGE — corrigido na v5: CHANGE
  // causava duplo disparo por vibração), só a borda escolhida é que
  // agora depende do que foi lido aqui.
  sw420_1_repouso = digitalRead(PIN_SW420_1);
  sw420_2_repouso = digitalRead(PIN_SW420_2);

  int modoDisparo1 = (sw420_1_repouso == LOW) ? RISING : FALLING;
  int modoDisparo2 = (sw420_2_repouso == LOW) ? RISING : FALLING;

  attachInterrupt(digitalPinToInterrupt(PIN_SW420_1), ISR_vibr1, modoDisparo1);
  attachInterrupt(digitalPinToInterrupt(PIN_SW420_2), ISR_vibr2, modoDisparo2);
}

// Correção — antes esta função era chamada em verificarPerifericos()
// (BancadaPreditiva.ino) mas não existia em lugar nenhum do projeto,
// o que impedia a compilação; okSW420_1/okSW420_2 eram apenas
// fixados em 'true', sem checar o sensor de fato.
//
// Teste adotado: lê o pino várias vezes seguidas, com um pequeno
// intervalo entre leituras, e confirma que o nível lido continua
// igual ao repouso já detectado por vibracaoInit(). Um pino com
// sensor de verdade conectado mantém o nível de repouso estável;
// um pino sem sensor (flutuando, sem pull-up/down) tende a variar
// entre leituras por captar ruído do ambiente. Não é uma prova
// definitiva de presença do sensor (um pino preso externamente a um
// nível fixo também passaria), mas já detecta o caso mais comum de
// falha: sensor ausente ou desconectado.
#define VIBR_AMOSTRAS_VERIFICACAO   10
#define VIBR_INTERVALO_VERIFICACAO_US 200

static bool nivelRepousoEstavel(uint8_t pino, int nivelEsperado) {
  for (int i = 0; i < VIBR_AMOSTRAS_VERIFICACAO; i++) {
    if (digitalRead(pino) != nivelEsperado) return false;
    delayMicroseconds(VIBR_INTERVALO_VERIFICACAO_US);
  }
  return true;
}

void verificarVibracao(bool &ok1, bool &ok2) {
  ok1 = nivelRepousoEstavel(PIN_SW420_1, sw420_1_repouso);
  ok2 = nivelRepousoEstavel(PIN_SW420_2, sw420_2_repouso);
}
