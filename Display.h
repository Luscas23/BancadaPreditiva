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

// Tela principal do loop (Fase 3) — tela FIXA 16x2, sem rodízio entre
// telas. Mostra só o essencial (temperatura, corrente, RPM e
// frequência dominante da vibração); Andon/erros continuam
// sinalizados pela torre física, não pelo LCD.
void atualizarDisplay(float temperatura, float corrente, float freqVib, EstadoVibTela estadoVib);

// Tela genérica de 2 linhas (limpa antes de escrever). Usada pelas
// telas estáticas de boot e das fases 1/2.
void displayMsg(const __FlashStringHelper* linha0, const __FlashStringHelper* linha1);

// Tela de status de um item da Fase 1: nome na linha 0, "OK"/"ERRO"
// na linha 1 conforme 'ok'.
void displayStatus(const __FlashStringHelper* nome, bool ok);

// setup() — tela de abertura (2 telas de 2 linhas em sequência).
void displayTelaBoot();

void displayFase1Resultado(bool tudoOk, bool okPT100, bool okACS712, bool okTCRT5000, bool okADXL345);

// Fase 2 — countdown de 45s.
void displayFase2Countdown(unsigned long segundosRestantes);

// Utilitário genérico
void displayLimpar();

#endif
