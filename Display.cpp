#include "Display.h"
#include "SensorPT100.h"     // erroSensor
#include "SensorRPM.h"      // rpmAtual, estadoMotor, EstadoMotor

static LiquidCrystal_I2C lcd(0x27, 16, 2);

void displayInit() {
  lcd.init();
  lcd.backlight();
}

void displayMsg(const __FlashStringHelper* linha0, const __FlashStringHelper* linha1) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(linha0);
  lcd.setCursor(0, 1); lcd.print(linha1);
}

void displayStatus(const __FlashStringHelper* nome, bool ok) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(nome);
  lcd.setCursor(0, 1); lcd.print(ok ? F("OK") : F("ERRO"));
}

void atualizarDisplay(float temperatura, float corrente, float freqVib, EstadoVibTela estadoVib) {

  // ---- Linha 0 — Temperatura e Corrente ----
  lcd.setCursor(0, 0);
  lcd.print(F("T:"));
  lcd.setCursor(2, 0);
  if (erroSensor) {
    lcd.print(F("ERRO "));           // campo de 5 colunas (2..6)
  } else {
    if (temperatura >= 0.0 && temperatura < 100.0) lcd.print(F(" "));
    lcd.print(temperatura, 1);       // 5 colunas: " XX.X" ou "XXX.X"
  }
  lcd.setCursor(7, 0);
  lcd.print(F("C I:"));
  lcd.print(corrente, 2);            // ACS712-5A: sempre "X.XX" (4 col)
  lcd.print(F("A"));

  // ---- Linha 1 — RPM e vibração ----
  lcd.setCursor(0, 1);
  lcd.print(F("RPM:"));
  int rpmInt = (int)rpmAtual;
  lcd.setCursor(4, 1);
  lcd.print(F("    "));              // limpa campo de 4 col (4..7)
  lcd.setCursor(4, 1);
  lcd.print(rpmInt);

  // Campo de vibração: 7 colunas fixas (9..15)
  lcd.setCursor(9, 1);
  lcd.print(F("       "));
  lcd.setCursor(9, 1);
  switch (estadoVib) {
    case VIB_TELA_ERRO:   lcd.print(F("V:ERR")); break;
    case VIB_TELA_PARADO: lcd.print(F("V:---")); break;
    default:
      lcd.print(estadoVib == VIB_TELA_CALIBRANDO ? F("C:") : F("V:"));
      lcd.print((int)freqVib);
      lcd.print(F("Hz"));
  }
}

void displayTelaBoot() {
  displayMsg(F("BANCADA PREDIT."), F("MOTORES ELETR."));
  delay(1000);
  displayMsg(F("TCC - ENGENHARIA"), F("Iniciando..."));
  delay(1000);
}

void displayFase1Resultado(bool tudoOk, bool okPT100, bool okACS712, bool okTCRT5000,
                            bool okADXL345) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(tudoOk ? F("Todos OK!") : F("Erros detect.:"));

  lcd.setCursor(0, 1);
  if (tudoOk) {
    lcd.print(F("Iniciando 45s.."));
  } else {
    // Lista de falhas, truncada em 16 colunas — se muitos itens
    // falharem ao mesmo tempo os últimos podem não couber na tela;
    // o defeito continua sinalizado pela torre Andon de qualquer forma.
    uint8_t col = 0;
    if (!okPT100  && col < 16) { lcd.print(F("PT100 ")); col += 6; }
    if (!okACS712 && col < 16) { lcd.print(F("ACS "));   col += 4; }
    if (!okTCRT5000 && col < 16) { lcd.print(F("TCRT ")); col += 5; }
    if (!okADXL345  && col < 16) { lcd.print(F("VIB "));  col += 4; }
  }
}

void displayFase2Countdown(unsigned long segundosRestantes) {
  // Tela do countdown ocupa as 2 linhas por completo a cada chamada.
  lcd.setCursor(0, 0);
  lcd.print(F("Iniciando em... "));

  lcd.setCursor(0, 1);
  lcd.print(F("                "));   // limpa a linha inteira (16 col)
  lcd.setCursor(0, 1);
  lcd.print(F("Aguarde: "));
  if (segundosRestantes < 10) lcd.print(F(" "));
  lcd.print(segundosRestantes);
  lcd.print(F("s"));
}

void displayLimpar() {
  lcd.clear();
}
