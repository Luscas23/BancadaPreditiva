#include "SensorVibracao.h"

volatile bool vibr1 = false;
volatile bool vibr2 = false;

unsigned long ultimaVibr1Ms = 0;
unsigned long ultimaVibr2Ms = 0;

// Repouso e debounce são detalhe interno do módulo — não precisam
// ser vistos de fora
static int sw420_1_repouso = LOW;
static int sw420_2_repouso = LOW;

static void ISR_vibr1() {
  unsigned long agora = millis();
  if (agora - ultimaVibr1Ms > DEBOUNCE_MS) {
    vibr1         = true;
    ultimaVibr1Ms = agora;
  }
}

static void ISR_vibr2() {
  unsigned long agora = millis();
  if (agora - ultimaVibr2Ms > DEBOUNCE_MS) {
    vibr2         = true;
    ultimaVibr2Ms = agora;
  }
}

void vibracaoInit() {
  pinMode(PIN_SW420_1, INPUT);
  pinMode(PIN_SW420_2, INPUT);

  // Detecta o nível de repouso de cada sensor e usa isso pra escolher
  // a borda de disparo: dispara sempre que o sinal SAI do repouso, em
  // vez de assumir de antemão que repouso = LOW. Cobre o caso de um
  // SW-420 (ou fiação) que fique em nível alto parado. Continua de
  // borda única (nunca CHANGE — CHANGE causa duplo disparo).
  sw420_1_repouso = digitalRead(PIN_SW420_1);
  sw420_2_repouso = digitalRead(PIN_SW420_2);

  int modoDisparo1 = (sw420_1_repouso == LOW) ? RISING : FALLING;
  int modoDisparo2 = (sw420_2_repouso == LOW) ? RISING : FALLING;

  attachInterrupt(digitalPinToInterrupt(PIN_SW420_1), ISR_vibr1, modoDisparo1);
  attachInterrupt(digitalPinToInterrupt(PIN_SW420_2), ISR_vibr2, modoDisparo2);
}

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
