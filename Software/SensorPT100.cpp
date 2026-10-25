#include "SensorPT100.h"
#include "Configuracao.h"

// CS vem de cfg: objeto construído em buffer estático, sem heap
alignas(Adafruit_MAX31865) static uint8_t pt100Buf[sizeof(Adafruit_MAX31865)];
static Adafruit_MAX31865 *pt100;

bool erroSensor = false;

void pt100Init() {
  pt100 = new (pt100Buf, ColocarEm{}) Adafruit_MAX31865(cfg.pinMaxCs);
  pt100->begin(cfg.ptFios == 3 ? MAX31865_3WIRE : MAX31865_2WIRE);   // 2 e 4 fios: mesmo valor
}

float lerTemperatura() {
  float t    = pt100->temperature(cfg.ptRnom, cfg.ptRref);
  erroSensor = pt100->readFault() != 0;   // readRTD() já limpa o fault na próxima leitura
  return erroSensor ? NAN : t;
}

bool verificarPT100() {
  lerTemperatura();
  return !erroSensor;
}
