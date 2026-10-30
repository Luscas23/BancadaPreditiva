#ifndef SENSOR_PT100_H
#define SENSOR_PT100_H

#include <Arduino.h>
#include <Adafruit_MAX31865.h>

extern bool erroSensor;

void pt100Init();

float lerTemperatura();

bool verificarPT100();

#endif
