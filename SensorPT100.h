#ifndef SENSOR_PT100_H
#define SENSOR_PT100_H

#include <Arduino.h>
#include <Adafruit_MAX31865.h>

// ----------------------------------------------------------------
//  PT100 3 fios / MAX31865, referência 430 ohm
// ----------------------------------------------------------------
#define PIN_MAX31865_CS    10    // SPI: SCK=52 MISO=50 MOSI=51
#define PT100_RNOM         100.0
#define PT100_RREF         430.0

// true quando a última leitura teve fault (o display mostra "ERRO")
extern bool erroSensor;

void pt100Init();

// Temperatura em °C. Com fault: erroSensor = true e o retorno (NAN) não vale.
float lerTemperatura();

// Sanidade do sensor no boot
bool verificarPT100();

#endif
