#include "Display.h"
#include "SensorPT100.h"     // erroSensor
#include "SensorRPM.h"       // rpmAtual
#include "LoggerSD.h"        // sdDisponivel

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

// Duas linhas de 16 colunas montadas e impressas de uma vez (sem piscar)
void atualizarDisplay(float temperatura, float corrente, float freqVib, EstadoVibTela estadoVib) {
  char t[8], c[8], v[8], l0[17], l1[17];

  if (erroSensor) strcpy(t, "ERRO ");
  else            dtostrf(temperatura, 5, 1, t);
  dtostrf(corrente, 4, 2, c);
  snprintf(l0, sizeof l0, "T:%sC I:%sA", t, c);

  if      (estadoVib == VIB_TELA_ERRO)   strcpy(v, "V:ERR");
  else if (estadoVib == VIB_TELA_PARADO) strcpy(v, "V:---");
  else snprintf(v, sizeof v, "%c:%dHz", estadoVib == VIB_TELA_CALIBRANDO ? 'C' : 'V', (int)freqVib);
  snprintf(l1, sizeof l1, "RPM:%-4d%c%-7s", (int)rpmAtual, sdDisponivel ? ' ' : '!', v);

  lcd.setCursor(0, 0); lcd.print(l0);
  lcd.setCursor(0, 1); lcd.print(l1);
}
