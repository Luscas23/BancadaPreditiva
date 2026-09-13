#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Andon.h"

// ----------------------------------------------------------------
//  LCD 20x4 I2C — o objeto mora aqui (Passo 8)
// ----------------------------------------------------------------
extern LiquidCrystal_I2C lcd;

void displayInit();

// Tela principal do loop (Fase 3) — Passo 7/8
void atualizarDisplay(float temperatura, float corrente, EstadoAndon estado, int erros);

// ----------------------------------------------------------------
//  Passo 9 — telas de boot e das Fases 1 e 2, antes soltas no .ino
//  Cada função corresponde a exatamente um bloco de lcd.setCursor/print
//  que existia em verificarPerifericos()/countdown45s()/setup(), na
//  mesma ordem e com o mesmo texto/posição de cursor de antes.
// ----------------------------------------------------------------

// setup() — tela de abertura (inclui o delay(2000) que já existia)
void displayTelaBoot();

// Fase 1 — verificação de periféricos
void displayFase1Inicio();                 // clear + "Verificando sistema" + "Aguarde..."
void displayFase1StatusDisplay();          // linha 1: "Display.........OK"
void displayFase1StatusPT100(bool ok);     // linha 2: "PT100...........OK/ERRO"
void displayFase1StatusACS712(bool ok);    // linha 3: "ACS712..........OK/ERRO"
void displayFase1Continuacao();            // clear + "Verificando sistema" (2ª tela)
void displayFase1StatusHall(bool ok);      // linha 1: "Sensor Hall.....OK/ERRO"
void displayFase1StatusVibr1();            // linha 2: "Vibr. SW1.......OK"
void displayFase1StatusVibr2();            // linha 3: "Vibr. SW2.......OK"
void displayFase1TelaAndon();              // clear + título + "Andon torre....."
void displayFase1StatusAndon();            // (16,1): "OK"
void displayFase1StatusSD(bool ok);        // linha 2: "Cartao SD........OK/ERRO"
void displayFase1Resultado(bool tudoOk, bool okPT100, bool okACS712, bool okHall);

// Fase 2 — countdown de 45s
void displayFase2Countdown(unsigned long segundosRestantes);
void displayFase2IniciandoLeitura();

// Utilitário genérico (usado no fim da Fase 2)
void displayLimpar();

#endif
