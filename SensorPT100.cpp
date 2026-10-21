#include "SensorPT100.h"

Adafruit_MAX31865 pt100 = Adafruit_MAX31865(PIN_MAX31865_CS);

bool erroSensor = false;

// Filtro passa-baixa (média móvel exponencial) na temperatura, pra
// suavizar ruído de leitura sem guardar um histórico de amostras.
// ALFA mais perto de 1 = responde mais rápido a mudanças reais;
// mais perto de 0 = mais suave (mais atraso).
#define FILTRO_TEMP_ALFA 0.2f
static float temperaturaFiltrada = 0;
static bool  filtroIniciado      = false;

static float filtrarTemp(float novaLeitura) {
  if (!filtroIniciado) {
    temperaturaFiltrada = novaLeitura;
    filtroIniciado      = true;
  } else {
    temperaturaFiltrada += (novaLeitura - temperaturaFiltrada) * FILTRO_TEMP_ALFA;
  }
  return temperaturaFiltrada;
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
  return filtrarTemp(t);
}

bool verificarPT100() {
  float testTemp = pt100.temperature(PT100_RNOM, PT100_RREF);
  uint8_t fault  = pt100.readFault();
  bool ok = (fault == 0 && testTemp > -200.0 && testTemp < 500.0);
  pt100.clearFault();
  return ok;
}
