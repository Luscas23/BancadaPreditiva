#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void displayInit();

// O que o campo de vibração (colunas 9..15) mostra
typedef enum {
  VIB_TELA_OK,           // "V:29Hz"
  VIB_TELA_CALIBRANDO,   // "C:29Hz"
  VIB_TELA_PARADO,       // "V:---"
  VIB_TELA_ERRO          // "V:ERR"
} EstadoVibTela;

// Tela fixa 16x2: temperatura, corrente, RPM, frequência dominante.
// Coluna 8 da linha 1 mostra '!' quando o cartão SD está indisponível.
void atualizarDisplay(float temperatura, float corrente, float freqVib, EstadoVibTela estadoVib);

// Aviso de configuração: "CFG: erro N" (detalhes na Serial)
void displayAviso(int n);

// Tela de 2 linhas (limpa antes de escrever)
void displayMsg(const __FlashStringHelper* linha0, const __FlashStringHelper* linha1);

#endif
