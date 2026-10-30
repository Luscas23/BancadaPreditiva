#include "SensorPT100.h"
#include "Configuracao.h"

alignas(Adafruit_MAX31865) static uint8_t pt100Buf[sizeof(Adafruit_MAX31865)];
static Adafruit_MAX31865 *pt100;

bool erroSensor = false;

void pt100Init() {
  pt100 = new (pt100Buf, ColocarEm{}) Adafruit_MAX31865(cfg.pinMaxCs);
  pt100->begin(cfg.ptFios == 3 ? MAX31865_3WIRE : MAX31865_2WIRE);
}

float lerTemperatura() {
  float t    = pt100->temperature(cfg.ptRnom, cfg.ptRref);
  erroSensor = pt100->readFault() != 0;
  return erroSensor ? NAN : t;
}

bool verificarPT100() {
  lerTemperatura();
  return !erroSensor;
}
