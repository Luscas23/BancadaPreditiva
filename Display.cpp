#include "Display.h"
#include "SensorPT100.h"     // erroSensor
#include "SensorVibracao.h" // vibr1, vibr2, ultimaVibr1Ms, ultimaVibr2Ms
#include "SensorRPM.h"      // rpmAtual, estadoMotor, EstadoMotor

LiquidCrystal_I2C lcd(0x27, 16, 2);

void displayInit() {
  lcd.init();
  lcd.backlight();
}

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


void displayTelaBoot() {
  // Correção — hardware real é 16x2, não 20x4: o texto original não
  // cabia (linhas 2/3 não existem) e cada linha passava de 16
  // colunas. Dividido em 2 telas sequenciais, mantendo o mesmo
  // delay total (2000ms) que já existia.
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("BANCADA PREDIT."));
  lcd.setCursor(0, 1); lcd.print(F("MOTORES ELETR."));
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("TCC - ENGENHARIA"));
  lcd.setCursor(0, 1); lcd.print(F("Iniciando..."));
  delay(1000);
}

// Correção — hardware real é 16x2, não 20x4: as funções abaixo
// escreviam em linhas (2 e 3) e colunas (até a coluna 19) que não
// existem nesse display. Redesenhadas como uma tela de 2 linhas por
// item (nome na linha 0, resultado na linha 1); a sequência de
// chamadas no .ino e os nomes das funções continuam os mesmos, só
// muda o desenho interno de cada tela.

void displayFase1Inicio() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando"));
  lcd.setCursor(0, 1); lcd.print(F("sistema..."));
}

void displayFase1StatusDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Display"));
  lcd.setCursor(0, 1); lcd.print(F("OK"));
}

void displayFase1StatusPT100(bool ok) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("PT100"));
  lcd.setCursor(0, 1); lcd.print(ok ? F("OK") : F("ERRO"));
}

void displayFase1StatusACS712(bool ok) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("ACS712"));
  lcd.setCursor(0, 1); lcd.print(ok ? F("OK") : F("ERRO"));
}

void displayFase1Continuacao() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando"));
  lcd.setCursor(0, 1); lcd.print(F("mais sensores..."));
}

void displayFase1StatusHall(bool ok) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Sensor Hall"));
  lcd.setCursor(0, 1); lcd.print(ok ? F("OK") : F("ERRO"));
}

void displayFase1StatusVibr1(bool ok) {
  // Correção — antes esta tela sempre mostrava "OK" fixo, porque a
  // Fase 1 nunca testava o sensor de verdade (ver SensorVibracao.h).
  // Agora recebe o resultado real de verificarVibracao().
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Vibr. SW1"));
  lcd.setCursor(0, 1); lcd.print(ok ? F("OK") : F("ERRO"));
}

void displayFase1StatusVibr2(bool ok) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Vibr. SW2"));
  lcd.setCursor(0, 1); lcd.print(ok ? F("OK") : F("ERRO"));
}

void displayFase1TelaAndon() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando"));
  lcd.setCursor(0, 1); lcd.print(F("Torre Andon..."));
}

void displayFase1StatusAndon() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Torre Andon"));
  lcd.setCursor(0, 1); lcd.print(F("OK"));
}

void displayFase1StatusSD(bool ok) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Cartao SD"));
  lcd.setCursor(0, 1); lcd.print(ok ? F("OK") : F("ERRO"));
}

void displayFase1Resultado(bool tudoOk, bool okPT100, bool okACS712, bool okHall,
                            bool okVibr1, bool okVibr2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(tudoOk ? F("Todos OK!") : F("Erros detect.:"));

  lcd.setCursor(0, 1);
  if (tudoOk) {
    lcd.print(F("Iniciando 45s.."));
  } else {
    // Lista de falhas agora inclui vibração (Correção: antes
    // okSW420_1/okSW420_2 eram sempre 'true', então nunca apareciam
    // aqui mesmo se o parâmetro existisse). Trunca em 16 colunas —
    // se muitos itens falharem ao mesmo tempo, os últimos podem não
    // couber na tela; o defeito continua sinalizado pela torre Andon
    // de qualquer forma.
    String falhas = "";
    if (!okPT100)  falhas += "PT100 ";
    if (!okACS712) falhas += "ACS ";
    if (!okHall)   falhas += "HALL ";
    if (!okVibr1)  falhas += "VIB1 ";
    if (!okVibr2)  falhas += "VIB2 ";
    while (falhas.length() < 16) falhas += ' ';
    lcd.print(falhas.substring(0, 16));
  }
}

void displayFase2Countdown(unsigned long segundosRestantes) {
  // Correção — antes escrevia só na linha 3, contando com o texto de
  // displayFase1Resultado() ainda visível nas linhas acima (0,1,2).
  // Num display 16x2 não existe "linha acima sobrando": a tela do
  // countdown agora ocupa as 2 linhas por completo a cada chamada.
  lcd.setCursor(0, 0);
  lcd.print(F("Iniciando em... "));

  lcd.setCursor(0, 1);
  String linha = "Aguarde: ";
  if (segundosRestantes < 10) linha += ' ';
  linha += String(segundosRestantes);
  linha += 's';
  while (linha.length() < 16) linha += ' ';
  lcd.print(linha.substring(0, 16));
}

void displayFase2IniciandoLeitura() {
  // Correção — hardware real é 16x2: "INICIANDO LEITURA" sozinho já
  // tem 17 caracteres e não cabe numa linha; dividido nas 2 linhas.
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("INICIANDO"));
  lcd.setCursor(0, 1); lcd.print(F("LEITURA DO MOTOR"));
}

void displayLimpar() {
  lcd.clear();
}
