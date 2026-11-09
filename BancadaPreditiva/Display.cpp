#include "Display.h"
#include "SensorPT100.h"
#include "SensorRPM.h"
#include "SensorCorrente.h"
#include "SensorVibracao.h"
#include "Andon.h"
#include "LoggerSD.h"
#include "Configuracao.h"
#include "config.h"
#include <avr/wdt.h>

alignas(LiquidCrystal_I2C) static uint8_t lcdBuf[sizeof(LiquidCrystal_I2C)];
static LiquidCrystal_I2C *lcdp;
#define lcd (*lcdp)

void displayInit() {
  lcdp = new (lcdBuf, ColocarEm{}) LiquidCrystal_I2C(cfg.enderecoLcd, LCD_COLS, LCD_LINHAS);
  lcd.init();
  lcd.backlight();
}

void displayReiniciar() {
  static unsigned long ultimo = 0;
  if (ultimo && millis() - ultimo < 10000UL) return;
  ultimo = millis();
  wdt_reset();
  lcd.init();
  lcd.backlight();
}

void displayAviso(int n) {
  char l0[LCD_COLS + 1];
  snprintf(l0, sizeof l0, "CFG: erro %d", n);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(l0);
  lcd.setCursor(0, 1); lcd.print(F("ver Serial"));
}

void displayMsg(const __FlashStringHelper* linha0, const __FlashStringHelper* linha1) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(linha0);
  lcd.setCursor(0, 1); lcd.print(linha1);
}

void atualizarDisplay(float temperatura, float corrente, float freqVib, EstadoVibTela estadoVib, EstadoAndon estado, int erros) {
  char t[8], c[8], v[8], l0[17], l1[17];

  if (erroSensor) strcpy(t, "ERRO ");
  else            dtostrf(temperatura, 5, 1, t);
  dtostrf(corrente, 4, 2, c);
  if (erroSensorCorr) strcpy(c, "ERR ");
  snprintf(l0, sizeof l0, "T:%sC I:%sA", t, c);

  if      (estadoVib == VIB_TELA_ERRO)   strcpy(v, "V:ERR");
  else if (estadoVib == VIB_TELA_PARADO) strcpy(v, "V:---");
  else snprintf(v, sizeof v, "%c:%dHz", estadoVib == VIB_TELA_CALIBRANDO ? 'C' : 'V', (int)freqVib);
  snprintf(l1, sizeof l1, "RPM:%-4d%c%-7s", (int)rpmAtual, sdDisponivel ? ' ' : '!', v);

  static uint8_t paginaAnt = 255;
  uint8_t pag = (millis() / 3000) & 1;
  if (pag != paginaAnt) { lcd.clear(); paginaAnt = pag; }

  if (pag == 0) {
    lcd.setCursor(0, 0); lcd.print(l0);
    lcd.setCursor(0, 1); lcd.print(l1);
  } else {
    lcd.setCursor(0, 0); lcd.print(andonParaTexto(estado));
    lcd.print(F(" Err:")); lcd.print(erros); lcd.print(F("  "));
    lcd.setCursor(0, 1); lcd.print(F("Vib x"));
    if (estadoVib == VIB_TELA_ERRO) lcd.print(F("ERR"));
    else if (estadoVib == VIB_TELA_CALIBRANDO) lcd.print(F("CAL"));
    else lcd.print(vibRazao, 1);
    lcd.print(F("   "));
  }
}
