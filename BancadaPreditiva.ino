// ================================================================
//   BANCADA DE MANUTENÇÃO PREDITIVA DE MOTORES ELÉTRICOS
//   Microcontrolador : Arduino Mega 2560
//   Display          : LCD 20x4 I2C
//   Sensores         : PT100+MAX31865 | ACS712-5A | KY-003 | 2x SW-420
//   Andon            : Torre de sinalização (Verde / Amarelo / Vermelho)
//   Autor            : Lucas Altruda Salce
//   TCC              : Engenharia Mecatrônica
//   Versão           : 6.0 (em refatoração modular — Passo 9/10 concluído)
//   Alimentação      : Power Bank 5V/2A 5.000mAh via USB (sem PC)
//
//   Histórico completo de melhorias (v4/v5/v6) e da refatoração
//   modular está em CHANGELOG.md — aqui fica só o essencial.
// ================================================================

// ----------------------------------------------------------------
//  BIBLIOTECAS
//  Passo 9 — Wire.h, LiquidCrystal_I2C.h, Adafruit_MAX31865.h e SD.h
//  saíram daqui: nada neste arquivo as usa diretamente mais, cada
//  uma já mora dentro do módulo que realmente precisa dela
//  (Display, SensorPT100, LoggerSD). avr/wdt.h continua, porque o
//  .ino chama wdt_disable()/wdt_enable()/wdt_reset() diretamente.
// ----------------------------------------------------------------
#include <avr/wdt.h>             // Watchdog Timer

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
#include "Destino.h"       // DESTINO_SERIAL, usado no log Serial

// ----------------------------------------------------------------
//  SETPOINTS E TOLERÂNCIAS (configuração de negócio)
//  Ainda globais aqui — candidatos a um futuro Config.h com perfis
//  de motor (127V/220V), mas isso fica fora do escopo do Passo 9.
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
//  VARIÁVEIS DE LEITURA
// ----------------------------------------------------------------
float temperatura  = 0.0;
float corrente     = 0.0;
// erroSensor mora em SensorPT100.cpp (extern via SensorPT100.h)
// lcd mora em Display.cpp (extern via Display.h)
// sdDisponivel mora em LoggerSD.cpp (extern via LoggerSD.h)

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
//  Passo 9 — só orquestra: cada tela virou uma chamada a Display,
//  cada verificação de hardware continua chamando o módulo do sensor.
// ================================================================
void verificarPerifericos() {
  displayFase1Inicio();
  delay(1000);

  // Display
  okDisplay = true;
  displayFase1StatusDisplay();
  delay(500);

  // PT100
  okPT100 = verificarPT100();
  displayFase1StatusPT100(okPT100);
  delay(500);

  // ACS712
  okACS712 = verificarACS712();
  displayFase1StatusACS712(okACS712);
  delay(800);

  displayFase1Continuacao();

  // Hall
  okHall = verificarHall();
  displayFase1StatusHall(okHall);
  delay(500);

  // SW-420: detecta estado de repouso do sensor
  verificarVibracao();
  okSW420_1 = true;  // Sensor presente e lido com sucesso
  okSW420_2 = true;
  displayFase1StatusVibr1();
  delay(500);
  displayFase1StatusVibr2();
  delay(500);

  // Andon
  displayFase1TelaAndon();
  setAndon(ANDON_GRAVE);   delay(300);
  setAndon(ANDON_DEFEITO); delay(300);
  setAndon(ANDON_BOM);     delay(300);
  setAndon(ANDON_GRAVE);
  okAndon = true;
  displayFase1StatusAndon();
  delay(500);

  // Cartão SD (informativo: falha aqui não impede o resumo final)
  okSD = loggerSDInit();
  sdDisponivel = okSD;
  displayFase1StatusSD(okSD);
  delay(500);

  // Resumo
  bool tudo_ok = okDisplay && okPT100 && okACS712 &&
                 okHall && okSW420_1 && okSW420_2 && okAndon;

  displayFase1Resultado(tudo_ok, okPT100, okACS712, okHall);

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
    wdt_reset();  // no-op hoje: watchdog só é ativado em iniciarWatchdog(),
                  // chamada depois do countdown (ver setup()). Mantido aqui
                  // de propósito, caso a ordem de ativação mude no futuro.
    unsigned long restante = (duracao - (millis() - inicio)) / 1000;
    displayFase2Countdown(restante);
    delay(500);
  }

  displayFase2IniciandoLeitura();
  piscarAndon(ANDON_BOM, 3, 200);
  setAndon(ANDON_BOM);
  delay(1500);
  displayLimpar();
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
  displayTelaBoot();

  // PT100
  pt100Init();

  // Fases de inicialização
  verificarPerifericos();
  countdown45s();
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

  // Monta a leitura atual para a lógica pura de decisão
  LeituraAtual leitura;
  leitura.temperatura  = temperatura;
  leitura.corrente     = corrente;
  leitura.rpm          = rpmAtual;
  leitura.motorJaGirou = motorJaGirou;
  leitura.vibr1        = vibr1_snapshot;
  leitura.vibr2        = vibr2_snapshot;
  leitura.erroSensorTemp = erroSensor;

  int erros = 0;
  EstadoAndon estado = avaliarEstado(leitura, limites, erros);

  // Atualiza Andon
  setAndon(estado);

  // Atualiza display — tela fixa 16x2 (Passo 11): só o essencial
  // (temperatura, corrente, RPM, vibração). Estado/erros não vão mais
  // pro LCD, quem sinaliza isso é a torre (setAndon() já foi chamado
  // acima); flags de vibração são zeradas dentro, após a exibição.
  atualizarDisplay(temperatura, corrente);

  // Log Serial
  Serial.print(temperatura, 2); Serial.print(F("\t"));
  Serial.print(corrente, 3);    Serial.print(F("\t"));
  Serial.print((int)rpmAtual);  Serial.print(F("\t"));
  Serial.print(erros);          Serial.print(F("\t"));
  Serial.print(motorParaTexto(estadoMotor, DESTINO_SERIAL)); Serial.print(F("\t"));
  Serial.println(andonParaTexto(estado, DESTINO_SERIAL));

  // Grava a mesma leitura no cartão SD (se disponível)
  gravarLeituraSD(temperatura, corrente, estado, erros);

  delay(500);
}
