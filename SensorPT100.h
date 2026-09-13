#ifndef SENSOR_PT100_H
#define SENSOR_PT100_H

#include <Arduino.h>
#include <Adafruit_MAX31865.h>

// ----------------------------------------------------------------
//  PT100 / MAX31865
// ----------------------------------------------------------------
#define PIN_MAX31865_CS    10    // PT100 CS (SPI: SCK=52 MISO=50 MOSI=51)
#define PT100_RNOM         100.0
#define PT100_RREF         430.0

// true quando a última leitura teve fault (usado pelo display para
// mostrar "ERRO" em vez do valor de temperatura)
extern bool erroSensor;

// Inicializa o driver MAX31865 (chamar uma vez no setup, via pt100.begin)
void pt100Init();

// Leitura de temperatura com média móvel (últimas 5 amostras) e
// tratamento de fault. Retorna -999.0 e seta erroSensor=true em caso
// de fault (fault é limpo internamente via pt100.clearFault()).
float lerTemperatura();

// Verificação de presença/sanidade do sensor na Fase 1
bool verificarPT100();

#endif
