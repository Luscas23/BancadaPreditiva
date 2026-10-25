#include "SensorPT100.h"

Adafruit_MAX31865 pt100 = Adafruit_MAX31865(PIN_MAX31865_CS);

bool erroSensor = false;

void pt100Init() {
  pt100.begin(MAX31865_3WIRE);
}

float lerTemperatura() {
  float t    = pt100.temperature(PT100_RNOM, PT100_RREF);
  erroSensor = pt100.readFault() != 0;   // readRTD() já limpa o fault na próxima leitura
  return erroSensor ? NAN : t;
}

bool verificarPT100() {
  lerTemperatura();
  return !erroSensor;
}
