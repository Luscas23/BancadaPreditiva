// ================================================================
//   BANCADA DE MANUTENÇÃO PREDITIVA DE MOTORES ELÉTRICOS
//   Microcontrolador : Arduino Mega 2560
//   Display          : LCD 20x4 I2C
//   Sensores         : PT100+MAX31865 | ACS712-5A | KY-003 | 2x SW-420
//   Andon            : Torre de sinalização (Verde / Amarelo / Vermelho)
//   Autor            : Lucas Altruda Salce
//   TCC              : Engenharia Mecatrônica
//   Versão           : 6.0 (em refatoração modular — Passo 8/10 concluído)
//   Alimentação      : Power Bank 5V/2A 5.000mAh via USB (sem PC)
//
//   Histórico completo de melhorias (v4/v5/v6) e da refatoração
//   modular está em CHANGELOG.md — aqui fica só o essencial.
// ================================================================

// ----------------------------------------------------------------
//  BIBLIOTECAS
// ----------------------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MAX31865.h>
#include <avr/wdt.h>             // Watchdog Timer
#include <SD.h>                  // Gravação em cartão SD

// ----------------------------------------------------------------
//  MÓDULOS DO PROJETO
// ----------------------------------------------------------------
#include "Andon.h"
#include "SensorCorrente.h"
#include "SensorPT100.h"
#include "SensorVibracao.h"
#include "SensorRPM.h"
#include "LogicaAvaliacao.h"
#include "Display.h"
#include "LoggerSD.h"

// ----------------------------------------------------------------
//  SETPOINTS E TOLERÂNCIAS (configuração de negócio)
//  Ainda globais aqui — candidatos a um futuro Config.h com perfis
//  de motor (127V/220V), mas isso fica fora do escopo do Passo 7.
// ----------------------------------------------------------------
float setpointTemp    = 60.0;
float toleranciaTemp  = 20.0;
#define TEMP_GRAVE     100.0

// Motor 127V: setpointCorr=2.9, toleranciaCorr=0.4, CORR_GRAVE=4.0
// Motor 220V: setpointCorr=1.4, toleranciaCorr=0.2, CORR_GRAVE=2.0
float setpointCorr    = 2.9;
float toleranciaCorr  = 0.4;
#define CORR_GRAVE     4.0

float setpointRPM     = 1740.0;
float toleranciaRPM   = 140.0;
#define RPM_GRAVE_MIN  1400.0
#define RPM_GRAVE_MAX  2000.0

// Passo 7 — empacota os setpoints acima para o módulo LogicaAvaliacao.
// Montada uma única vez: nada aqui muda em runtime hoje.
LimitesAvaliacao limites = {
  setpointTemp, toleranciaTemp, TEMP_GRAVE,
  setpointCorr, toleranciaCorr, CORR_GRAVE,
  setpointRPM, toleranciaRPM, RPM_GRAVE_MIN, RPM_GRAVE_MAX
};

// ----------------------------------------------------------------
//  ESTADOS DO SISTEMA
// ----------------------------------------------------------------
typedef enum {
  ESTADO_INIT,
  ESTADO_VERIFICANDO,
  ESTADO_AGUARDANDO,
  ESTADO_LENDO
} EstadoSistema;

EstadoSistema estadoAtual = ESTADO_INIT;

// ----------------------------------------------------------------
//  VARIÁVEIS DE LEITURA
// ----------------------------------------------------------------
float temperatura  = 0.0;
float corrente     = 0.0;
// erroSensor agora mora em SensorPT100.cpp (extern via SensorPT100.h)
// lcd agora mora em Display.cpp (extern via Display.h)
// sdDisponivel agora mora em LoggerSD.cpp (extern via LoggerSD.h)

bool okDisplay  = false;
bool okPT100    = false;
bool okACS712   = false;
bool okHall     = false;
bool okSW420_1  = false;
bool okSW420_2  = false;
bool okAndon    = false;
bool okSD       = false;   // informativo; falha aqui NÃO bloqueia o funcionamento da bancada

// ----------------------------------------------------------------
//  WATCHDOG: reinicia o Arduino se travar por >8s
// ----------------------------------------------------------------
void iniciarWatchdog() {
  wdt_enable(WDTO_8S);
}

// ================================================================
//  FASE 1 — VERIFICAÇÃO DE PERIFÉRICOS
// ================================================================
void verificarPerifericos() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));
  lcd.setCursor(0, 1); lcd.print(F("Aguarde...          "));
  delay(1000);

  // Display
  okDisplay = true;
  lcd.setCursor(0, 1); lcd.print(F("Display.........OK  "));
  delay(500);

  // PT100
  okPT100 = verificarPT100();
  lcd.setCursor(0, 2); lcd.print(F("PT100..........."));
  lcd.print(okPT100 ? F("OK  ") : F("ERRO"));
  delay(500);

  // ACS712
  okACS712 = verificarACS712();
  lcd.setCursor(0, 3); lcd.print(F("ACS712.........."));
  lcd.print(okACS712 ? F("OK  ") : F("ERRO"));
  delay(800);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));

  // Hall
  okHall = verificarHall();
  lcd.setCursor(0, 1); lcd.print(F("Sensor Hall....."));
  lcd.print(okHall ? F("OK  ") : F("ERRO"));
  delay(500);

  // SW-420: detecta estado de repouso do sensor
  verificarVibracao();
  okSW420_1 = true;  // Sensor presente e lido com sucesso
  okSW420_2 = true;
  lcd.setCursor(0, 2); lcd.print(F("Vibr. SW1.......OK  "));
  delay(500);
  lcd.setCursor(0, 3); lcd.print(F("Vibr. SW2.......OK  "));
  delay(500);

  // Andon
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));
  lcd.setCursor(0, 1); lcd.print(F("Andon torre.....    "));
  setAndon(ANDON_GRAVE);   delay(300);
  setAndon(ANDON_DEFEITO); delay(300);
  setAndon(ANDON_BOM);     delay(300);
  setAndon(ANDON_GRAVE);
  okAndon = true;
  lcd.setCursor(16, 1); lcd.print(F("OK  "));
  delay(500);

  // Cartão SD (informativo: falha aqui não impede o resumo final)
  okSD = loggerSDInit();
  sdDisponivel = okSD;
  lcd.setCursor(0, 2); lcd.print(F("Cartao SD........"));
  lcd.print(okSD ? F("OK  ") : F("ERRO"));
  delay(500);

  // Resumo
  bool tudo_ok = okDisplay && okPT100 && okACS712 &&
                 okHall && okSW420_1 && okSW420_2 && okAndon;

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("=== RESULTADO ===   "));
  lcd.setCursor(0, 1); lcd.print(tudo_ok ? F("Todos OK!           ")
                                         : F("Erros detectados!   "));
  if (!tudo_ok) {
    lcd.setCursor(0, 2);
    String falhas = "";
    if (!okPT100)  falhas += "PT100 ";
    if (!okACS712) falhas += "ACS ";
    if (!okHall)   falhas += "HALL ";
    falhas += "                ";
    lcd.print(falhas.substring(0, 20));
  }
  lcd.setCursor(0, 3); lcd.print(F("Iniciando em 45s... "));

  if (tudo_ok) setAndon(ANDON_BOM);
  else { piscarAndon(ANDON_DEFEITO, 3, 300); setAndon(ANDON_DEFEITO); }
}

// ================================================================
//  FASE 2 — COUNTDOWN 45 SEGUNDOS
// ================================================================
void countdown45s() {
  unsigned long inicio  = millis();
  unsigned long duracao = 45000UL;

  while (millis() - inicio < duracao) {
    wdt_reset();  // alimenta watchdog durante countdown
    unsigned long restante = (duracao - (millis() - inicio)) / 1000;
    lcd.setCursor(0, 3);
    lcd.print(F("Aguardando: "));
    if (restante < 10) lcd.print(F(" "));
    lcd.print(restante);
    lcd.print(F("s   "));
    delay(500);
  }

  lcd.clear();
  lcd.setCursor(2, 1); lcd.print(F("INICIANDO LEITURA"));
  lcd.setCursor(4, 2); lcd.print(F("DO MOTOR..."));
  piscarAndon(ANDON_BOM, 3, 200);
  setAndon(ANDON_BOM);
  delay(1500);
  lcd.clear();
}

// ================================================================
//  SETUP
// ================================================================
void setup() {
  // Desabilita watchdog residual de reset anterior
  wdt_disable();

  Serial.begin(9600);
  Serial.println(F("=== BANCADA PREDITIVA v6.0 ==="));

  // Módulos de hardware
  andonInit();
  setAndon(ANDON_GRAVE);

  vibracaoInit();
  rpmInit();

  // LCD
  displayInit();
  lcd.setCursor(0, 0); lcd.print(F("  BANCADA PREDITIVA "));
  lcd.setCursor(0, 1); lcd.print(F("  MOTORES ELETRICOS "));
  lcd.setCursor(0, 2); lcd.print(F("   TCC - ENGENHARIA "));
  lcd.setCursor(0, 3); lcd.print(F("   Iniciando...     "));
  delay(2000);

  // PT100
  pt100Init();

  // Fases de inicialização
  estadoAtual = ESTADO_VERIFICANDO;
  verificarPerifericos();

  estadoAtual = ESTADO_AGUARDANDO;
  countdown45s();

  estadoAtual = ESTADO_LENDO;
  loggerSDIniciarTempo();  // referência t=0 do CSV: começo da Fase 3, não do boot

  // Ativa watchdog apenas após inicialização completa
  iniciarWatchdog();

  Serial.println(F("Temp(C)\tCorr(A)\tRPM\tErros\tMotor\tEstado"));
  if (sdDisponivel) Serial.println(F("Gravando em LOG.CSV no cartao SD"));
  else              Serial.println(F("Cartao SD indisponivel - gravacao desabilitada"));
}

// ================================================================
//  LOOP PRINCIPAL
// ================================================================
void loop() {

  wdt_reset();  // alimenta watchdog a cada ciclo

  // Leituras
  temperatura = lerTemperatura();
  corrente    = lerCorrente();
  calcularRPM(setpointRPM, toleranciaRPM);

  // Captura estado das flags atomicamente ANTES de qualquer avaliação
  noInterrupts();
  bool vibr1_snapshot = vibr1;
  bool vibr2_snapshot = vibr2;
  interrupts();

  // Passo 7 — monta a leitura atual para a lógica pura de decisão
  LeituraAtual leitura;
  leitura.temperatura  = temperatura;
  leitura.corrente     = corrente;
  leitura.rpm          = rpmAtual;
  leitura.motorJaGirou = motorJaGirou;
  leitura.vibr1        = vibr1_snapshot;
  leitura.vibr2        = vibr2_snapshot;

  int erros = 0;
  EstadoAndon estado = avaliarEstado(leitura, limites, erros);

  // Atualiza Andon
  setAndon(estado);

  // Atualiza display (flags zeradas dentro, após exibição)
  atualizarDisplay(temperatura, corrente, estado, erros);

  // Log Serial
  Serial.print(temperatura, 2); Serial.print(F("\t"));
  Serial.print(corrente, 3);    Serial.print(F("\t"));
  Serial.print((int)rpmAtual);  Serial.print(F("\t"));
  Serial.print(erros);          Serial.print(F("\t"));
  switch (estadoMotor) {
    case MOTOR_PARADO:     Serial.print(F("PARADO\t"));     break;
    case MOTOR_ACELERANDO: Serial.print(F("ACELERANDO\t")); break;
    case MOTOR_OPERANDO:   Serial.print(F("OPERANDO\t"));   break;
    case MOTOR_PAROU:      Serial.print(F("PAROU!!!\t"));   break;
  }
  switch (estado) {
    case ANDON_BOM:    Serial.println(F("BOM"));     break;
    case ANDON_DEFEITO:Serial.println(F("DEFEITO"));  break;
    case ANDON_GRAVE:  Serial.println(F("GRAVE"));   break;
  }

  // Grava a mesma leitura no cartão SD (se disponível)
  gravarLeituraSD(temperatura, corrente, estado, erros);

  delay(500);
}
