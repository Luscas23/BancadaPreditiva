#include <Arduino.h>
#include "Display.h"
#include "SensorPT100.h"      // erroSensor
#include "SensorRPM.h"        // rpmAtual, estadoMotor, EstadoMotor
#include "SensorVibracao.h"   // vibr1, vibr2, ultimaVibr1Ms, ultimaVibr2Ms

LiquidCrystal_I2C lcd(0x27, 20, 4);

void displayInit() {
  lcd.init();
  lcd.backlight();
}

// ================================================================
//  FASE 3 — DISPLAY 20x4
//
//  Linha 0: BANCADA PREDITIVA [BOM/DEF/GRV]
//  Linha 1: T: XX.XC   I: X.XXA
//  Linha 2: RPM:XXXX  E:X [estado motor]
//  Linha 3: VIB1:XXs  VIB2:XXs
// ================================================================
void atualizarDisplay(float temperatura, float corrente, EstadoAndon estado, int erros) {

  // Linha 0 — título + estado
  lcd.setCursor(0, 0);
  lcd.print(F("BANCADA PREDITIVA   "));
  lcd.setCursor(17, 0);
  switch (estado) {
    case ANDON_BOM:    lcd.print(F("BOM")); break;
    case ANDON_DEFEITO:lcd.print(F("DEF")); break;
    case ANDON_GRAVE:  lcd.print(F("GRV")); break;
  }

  // Linha 1 — Temperatura e Corrente
  lcd.setCursor(0, 1);
  lcd.print(F("T:"));
  if (erroSensor) {
    lcd.print(F("ERRO  "));
  } else {
    if (temperatura >= 0.0 && temperatura < 100.0) lcd.print(F(" "));
    lcd.print(temperatura, 1);
    lcd.print(F("C "));
  }
  lcd.setCursor(10, 1);
  lcd.print(F("I:"));
  lcd.print(corrente, 2);
  lcd.print(F("A  "));

  // Linha 2 — RPM + erros + estado motor
  lcd.setCursor(0, 2);
  lcd.print(F("RPM:"));
  lcd.print((int)rpmAtual);
  lcd.print(F("  "));
  lcd.setCursor(10, 2);
  lcd.print(F("E:"));
  lcd.print(erros);
  lcd.print(F(" "));
  switch (estadoMotor) {
    case MOTOR_PARADO:     lcd.print(F("PAR")); break;
    case MOTOR_ACELERANDO: lcd.print(F("ACE")); break;
    case MOTOR_OPERANDO:   lcd.print(F("OPE")); break;
    case MOTOR_PAROU:      lcd.print(F("!!!"));  break;
  }

  // Linha 3 — tempo desde última vibração
  // (flags lidas ANTES de serem zeradas)
  unsigned long agora = millis();
  lcd.setCursor(0, 3);

  if (vibr1 || vibr2) {
    // Vibração ativa neste ciclo
    lcd.print(vibr2 ? F("VIB:GRAVE           ")
                    : F("VIB:DEFEITO         "));
  } else {
    // Mostra há quantos segundos foi a última vibração
    lcd.print(F("V1:"));
    if (ultimaVibr1Ms == 0) {
      lcd.print(F("---s "));
    } else {
      unsigned long s1 = (agora - ultimaVibr1Ms) / 1000;
      if (s1 > 999) s1 = 999;
      if (s1 < 10)  lcd.print(F("  "));
      else if (s1 < 100) lcd.print(F(" "));
      lcd.print(s1);
      lcd.print(F("s "));
    }
    lcd.setCursor(10, 3);
    lcd.print(F("V2:"));
    if (ultimaVibr2Ms == 0) {
      lcd.print(F("---s "));
    } else {
      unsigned long s2 = (agora - ultimaVibr2Ms) / 1000;
      if (s2 > 999) s2 = 999;
      if (s2 < 10)  lcd.print(F("  "));
      else if (s2 < 100) lcd.print(F(" "));
      lcd.print(s2);
      lcd.print(F("s "));
    }
  }

  // Limpa flags APÓS avaliação e exibição
  vibr1 = false;
  vibr2 = false;
}
