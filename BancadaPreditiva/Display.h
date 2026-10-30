#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void displayInit();

typedef enum {
  VIB_TELA_OK,
  VIB_TELA_CALIBRANDO,
  VIB_TELA_PARADO,
  VIB_TELA_ERRO
} EstadoVibTela;

void atualizarDisplay(float temperatura, float corrente, float freqVib, EstadoVibTela estadoVib);

void displayAviso(int n);

void displayMsg(const __FlashStringHelper* linha0, const __FlashStringHelper* linha1);

#endif
