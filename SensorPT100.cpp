#include "SensorPT100.h"

Adafruit_MAX31865 pt100 = Adafruit_MAX31865(PIN_MAX31865_CS);

bool erroSensor = false;

// Melhoria 5 — Média móvel temperatura (últimas 5 leituras)
// Buffer e índice agora são privados ao módulo (static): nenhum outro
// arquivo precisa enxergá-los, só o resultado via lerTemperatura().
#define MEDIA_MOVEL_N 5
static float bufferTemp[MEDIA_MOVEL_N] = {0};
static int   indexTemp                 = 0;
static bool  bufferPreenchido          = false;

static float mediaMovelTemp(float novaLeitura) {
  bufferTemp[indexTemp] = novaLeitura;
  indexTemp = (indexTemp + 1) % MEDIA_MOVEL_N;
  if (indexTemp == 0) bufferPreenchido = true;

  int n    = bufferPreenchido ? MEDIA_MOVEL_N : indexTemp;
  float soma = 0;
  for (int i = 0; i < n; i++) soma += bufferTemp[i];
  return soma / n;
}

void pt100Init() {
  pt100.begin(MAX31865_2WIRE);
}

float lerTemperatura() {
  float t       = pt100.temperature(PT100_RNOM, PT100_RREF);
  uint8_t fault = pt100.readFault();
  if (fault) {
    pt100.clearFault();
    erroSensor = true;
    return -999.0;
  }
  erroSensor = false;
  return mediaMovelTemp(t);   // Melhoria 5
}

bool verificarPT100() {
  float testTemp = pt100.temperature(PT100_RNOM, PT100_RREF);
  uint8_t fault  = pt100.readFault();
  bool ok = (fault == 0 && testTemp > -200.0 && testTemp < 500.0);
  pt100.clearFault();
  return ok;
}
