#include "Display.h"
#include "SensorPT100.h"     // erroSensor
#include "SensorVibracao.h" // vibr1, vibr2, ultimaVibr1Ms, ultimaVibr2Ms
#include "SensorRPM.h"      // rpmAtual, estadoMotor, EstadoMotor

LiquidCrystal_I2C lcd(0x27, 16, 2);

void displayInit() {
  lcd.init();
  lcd.backlight();
}

// ================================================================
//  FASE 3 — DISPLAY 16x2 (Passo 11 — hardware trocado de 20x4 p/ 16x2)
//
//  Tela FIXA, sem rodízio: só o essencial fica sempre visível.
//  Estado do Andon e contagem de erros NÃO aparecem mais aqui — quem
//  sinaliza isso agora é só a torre física (setAndon() já é chamado
//  separadamente no .ino). Motivo: 16 colunas não sobra espaço pra
//  tudo que cabia nas 20x4, e vibr+temp+corrente+RPM é o que importa
//  olhar direto na bancada.
//
//  Linha 0 (16 col): T:XX.XC I:X.XXA
//  Linha 1 (16 col): RPM:XXXX V:XXXs   (ou V:DEF / V:GRV se houve
//                    vibração neste ciclo)
//
//  Cada campo tem posição de cursor e largura fixas (sem lcd.clear()
//  a cada ciclo, pra não piscar) — valor mais curto é completado com
//  espaço até a largura do campo, igual já era feito no layout 20x4.
// ================================================================
void atualizarDisplay(float temperatura, float corrente) {

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
  lcd.print(rpmInt);
  int rpmDigitos = (rpmInt >= 1000) ? 4 : (rpmInt >= 100) ? 3 : (rpmInt >= 10) ? 2 : 1;
  for (int i = rpmDigitos; i < 4; i++) lcd.print(F(" "));  // completa campo de 4 col (4..7)

  // Campo de vibração: 7 colunas fixas (9..15)
  // Flags lidas ANTES de serem zeradas
  lcd.setCursor(9, 1);
  if (vibr2) {
    lcd.print(F("V:GRV  "));         // vibração faixa 2 (grave) neste ciclo
  } else if (vibr1) {
    lcd.print(F("V:DEF  "));         // vibração faixa 1 (defeito) neste ciclo
  } else {
    // Sem vibração neste ciclo: mostra há quanto tempo foi a última,
    // considerando as duas faixas juntas (a mais recente das duas) —
    // simplificação necessária pra caber ao lado do RPM nas 16 colunas.
    unsigned long agora = millis();
    unsigned long ultimaVibrMs = (ultimaVibr2Ms > ultimaVibr1Ms) ? ultimaVibr2Ms : ultimaVibr1Ms;
    if (ultimaVibrMs == 0) {
      lcd.print(F("V:---s "));
    } else {
      unsigned long s = (agora - ultimaVibrMs) / 1000;
      if (s > 999) s = 999;
      lcd.print(F("V:"));
      lcd.print(s);
      lcd.print(F("s"));
      int sDigitos = (s >= 100) ? 3 : (s >= 10) ? 2 : 1;
      for (int i = 2 + sDigitos + 1; i < 7; i++) lcd.print(F(" "));  // completa campo de 7 col (9..15)
    }
  }

  // Limpa flags APÓS avaliação e exibição
  vibr1 = false;
  vibr2 = false;
}

// ================================================================
//  Passo 9 — telas de boot e das Fases 1 e 2
//  Texto, posição de cursor e ordem idênticos ao que estava em
//  verificarPerifericos()/countdown45s()/setup() no .ino do Passo 8.
// ================================================================

void displayTelaBoot() {
  lcd.setCursor(0, 0); lcd.print(F("  BANCADA PREDITIVA "));
  lcd.setCursor(0, 1); lcd.print(F("  MOTORES ELETRICOS "));
  lcd.setCursor(0, 2); lcd.print(F("   TCC - ENGENHARIA "));
  lcd.setCursor(0, 3); lcd.print(F("   Iniciando...     "));
  delay(2000);
}

void displayFase1Inicio() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));
  lcd.setCursor(0, 1); lcd.print(F("Aguarde...          "));
}

void displayFase1StatusDisplay() {
  lcd.setCursor(0, 1); lcd.print(F("Display.........OK  "));
}

void displayFase1StatusPT100(bool ok) {
  lcd.setCursor(0, 2); lcd.print(F("PT100..........."));
  lcd.print(ok ? F("OK  ") : F("ERRO"));
}

void displayFase1StatusACS712(bool ok) {
  lcd.setCursor(0, 3); lcd.print(F("ACS712.........."));
  lcd.print(ok ? F("OK  ") : F("ERRO"));
}

void displayFase1Continuacao() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));
}

void displayFase1StatusHall(bool ok) {
  lcd.setCursor(0, 1); lcd.print(F("Sensor Hall....."));
  lcd.print(ok ? F("OK  ") : F("ERRO"));
}

void displayFase1StatusVibr1() {
  lcd.setCursor(0, 2); lcd.print(F("Vibr. SW1.......OK  "));
}

void displayFase1StatusVibr2() {
  lcd.setCursor(0, 3); lcd.print(F("Vibr. SW2.......OK  "));
}

void displayFase1TelaAndon() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));
  lcd.setCursor(0, 1); lcd.print(F("Andon torre.....    "));
}

void displayFase1StatusAndon() {
  lcd.setCursor(16, 1); lcd.print(F("OK  "));
}

void displayFase1StatusSD(bool ok) {
  lcd.setCursor(0, 2); lcd.print(F("Cartao SD........"));
  lcd.print(ok ? F("OK  ") : F("ERRO"));
}

void displayFase1Resultado(bool tudoOk, bool okPT100, bool okACS712, bool okHall) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("=== RESULTADO ===   "));
  lcd.setCursor(0, 1); lcd.print(tudoOk ? F("Todos OK!           ")
                                        : F("Erros detectados!   "));
  if (!tudoOk) {
    lcd.setCursor(0, 2);
    String falhas = "";
    if (!okPT100)  falhas += "PT100 ";
    if (!okACS712) falhas += "ACS ";
    if (!okHall)   falhas += "HALL ";
    falhas += "                ";
    lcd.print(falhas.substring(0, 20));
  }
  lcd.setCursor(0, 3); lcd.print(F("Iniciando em 45s... "));
}

void displayFase2Countdown(unsigned long segundosRestantes) {
  lcd.setCursor(0, 3);
  lcd.print(F("Aguardando: "));
  if (segundosRestantes < 10) lcd.print(F(" "));
  lcd.print(segundosRestantes);
  lcd.print(F("s   "));
}

void displayFase2IniciandoLeitura() {
  lcd.clear();
  lcd.setCursor(2, 1); lcd.print(F("INICIANDO LEITURA"));
  lcd.setCursor(4, 2); lcd.print(F("DO MOTOR..."));
}

void displayLimpar() {
  lcd.clear();
}
