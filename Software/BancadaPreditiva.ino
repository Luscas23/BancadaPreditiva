#include <avr/wdt.h>             // Watchdog Timer

#include "Andon.h"
#include "SensorCorrente.h"
#include "SensorPT100.h"
#include "SensorVibracao.h"
#include "SensorRPM.h"
#include "LogicaAvaliacao.h"
#include "Display.h"
#include "LoggerSD.h"
#include "Configuracao.h"
#include "config.h"

static LimitesAvaliacao limites;   // preenchido em setup(), depois da configuração

static void falha(const __FlashStringHelper* sensor) {
  displayMsg(F("FALHA SENSOR:"), sensor);
  delay(FALHA_MSG_MS);
}

void setup() {
  wdt_disable();   // watchdog residual de reset anterior

  Serial.begin(SERIAL_BAUD);
  Serial.println(F("=== BANCADA PREDITIVA v6.0 ==="));

  // Config: padrão do config.h; CONFIG.TXT do SD (opcional) sobrescreve; validação por último
  configPadrao();
  pinMode(cfg.pinMaxCs, OUTPUT);       // PT100 fora do SPI enquanto o SD é lido
  digitalWrite(cfg.pinMaxCs, HIGH);
  andonInit();                         // torre apagada até a 1ª partida (pinos padrão)
  sdDisponivel = loggerSDMontar();
  int avisos = sdDisponivel ? configCarregarSD(Serial) : 0;
  avisos += configValidar(Serial);
  pinMode(cfg.pinMaxCs, OUTPUT);       // o CS pode ter mudado
  digitalWrite(cfg.pinMaxCs, HIGH);
  andonInit();                         // pinos e polaridade finais
  limites = limitesDeConfig();
  configImprimir(Serial, "", false);

  if (!vibracaoInit()) Serial.println(F("ADXL345 nao respondeu no boot"));
  rpmInit();
  displayInit();
  if (avisos) { displayAviso(avisos); delay(CFG_MSG_MS); }
  displayMsg(F("BANCADA PREDIT."), F("Iniciando..."));
  pt100Init();

  float piso;   // verificarADXL345 ainda devolve o piso (ver item pendente)
  if (!verificarPT100())       falha(F("PT100"));
  if (!verificarACS712())      falha(F("ACS712"));
  if (!verificarADXL345(piso)) falha(F("ADXL345"));   // motor parado: mede o piso de ruído
  if (sdDisponivel) sdDisponivel = loggerSDInit();   // cria o CSV e anota a configuração usada
  loggerSDIniciarTempo();      // t=0 do CSV

  wdt_enable(WDT_TIMEOUT);         // só depois da inicialização completa

  Serial.println(F("Temp(C)\tCorr(A)\tRPM\tErros\tMotor\tEstado\tFreq(Hz)\t1X(g)\t2X(g)\tAlta(g)\tRazao\tCalib"));
  if (sdDisponivel) Serial.println(F("Gravando em LOG.CSV no cartao SD"));
  else              Serial.println(F("Cartao SD indisponivel - gravacao desabilitada"));
}

void loop() {
  static bool          ligado   = false;
  static unsigned long tPartida = 0, tRpmZero = 0;

  wdt_reset();

  while (Serial.available()) {
    if (Serial.read() == CMD_RECALIBRAR) vibracaoRecalibrar();
  }

  float temperatura = lerTemperatura();
  float corrente    = lerCorrente();
  calcularRPM(limites.setpointRPM, limites.toleranciaRPM);

  // A janela de vibração (~0,32 s) dá o ritmo do ciclo
  VibResultado vib = {};
  bool okVib = vibracaoLerJanela(rpmAtual, vib);

  // Partida = corrente subiu acima de cfg.corrParadoA (reinicia a carência)
  unsigned long agora = millis();
  bool lig = corrente >= cfg.corrParadoA;
  if (lig && !ligado) tPartida = tRpmZero = agora;
  if (rpmAtual > 0)   tRpmZero = agora;
  ligado = lig;

  LeituraAtual leitura;
  leitura.temperatura    = temperatura;
  leitura.corrente       = corrente;
  leitura.rpm            = rpmAtual;
  leitura.vibRazao       = vibRazao;
  leitura.erroSensorVib  = erroSensorVib;
  leitura.erroSensorTemp = erroSensor;
  leitura.msDesdePartida = lig ? agora - tPartida : 0;
  leitura.msRpmZero      = agora - tRpmZero;

  int erros = 0;
  EstadoAndon estado = avaliarEstado(leitura, limites, erros);
  vibracaoCalibrar(rpmAtual, estadoMotor, erros, okVib, vib);

  // 'estado' passa a ser o valor já filtrado: torre, Serial e CSV mostram o mesmo
  estado = estabilizarAndon(estado);
  setAndon(estado);

  EstadoVibTela telaVib = VIB_TELA_OK;
  if      (erroSensorVib)              telaVib = VIB_TELA_ERRO;
  else if (rpmAtual == 0)              telaVib = VIB_TELA_PARADO;
  else if (vibCalib == VIB_CALIBRANDO) telaVib = VIB_TELA_CALIBRANDO;
  atualizarDisplay(temperatura, corrente, vib.freqDominante, telaVib);

  imprimirLinha(Serial, '\t', temperatura, corrente, estado, erros, vib);

  // Sem janela válida a linha teria vibração zerada: não grava dado falso
  if (okVib) gravarLeituraSD(temperatura, corrente, estado, erros, vib);

  // Sem janela válida o ciclo não tem ritmo próprio: evita inundar Serial
  if (!okVib) delay(LOOP_SEM_JANELA_MS);
}
