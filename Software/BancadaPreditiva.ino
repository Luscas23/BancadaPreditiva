#include <avr/wdt.h>             // Watchdog Timer

#include "Andon.h"
#include "SensorCorrente.h"
#include "SensorPT100.h"
#include "SensorVibracao.h"
#include "SensorRPM.h"
#include "LogicaAvaliacao.h"
#include "Display.h"
#include "LoggerSD.h"

// Temperatura e corrente: #defines em LogicaAvaliacao.h
const LimitesAvaliacao limites = {
  1740.0, 140.0, 1400.0, 2000.0,  // RPM: setpoint, tolerancia, grave min/max
  2.0,  4.0                       // vibracao (amplitude/baseline): ruim, grave
};

static void falha(const __FlashStringHelper* sensor) {
  displayMsg(F("FALHA SENSOR:"), sensor);
  delay(1500);
}

void setup() {
  wdt_disable();   // watchdog residual de reset anterior

  Serial.begin(115200);
  Serial.println(F("=== BANCADA PREDITIVA v6.0 ==="));

  andonInit();     // deixa a torre apagada até a 1ª partida
  if (!vibracaoInit()) Serial.println(F("ADXL345 nao respondeu no boot"));
  rpmInit();
  displayInit();
  displayMsg(F("BANCADA PREDIT."), F("Iniciando..."));
  pt100Init();

  float piso;   // verificarADXL345 ainda devolve o piso (ver item pendente)
  if (!verificarPT100())       falha(F("PT100"));
  if (!verificarACS712())      falha(F("ACS712"));
  if (!verificarADXL345(piso)) falha(F("ADXL345"));   // motor parado: mede o piso de ruído
  sdDisponivel = loggerSDInit();
  loggerSDIniciarTempo();      // t=0 do CSV

  wdt_enable(WDTO_8S);         // só depois da inicialização completa

  Serial.println(F("Temp(C)\tCorr(A)\tRPM\tErros\tMotor\tEstado\tFreq(Hz)\t1X(g)\t2X(g)\tAlta(g)\tRazao\tCalib"));
  if (sdDisponivel) Serial.println(F("Gravando em LOG.CSV no cartao SD"));
  else              Serial.println(F("Cartao SD indisponivel - gravacao desabilitada"));
}

void loop() {
  static bool          ligado   = false;
  static unsigned long tPartida = 0, tRpmZero = 0;

  wdt_reset();

  while (Serial.available()) {
    if (Serial.read() == 'c') vibracaoRecalibrar();
  }

  float temperatura = lerTemperatura();
  float corrente    = lerCorrente();
  calcularRPM(limites.setpointRPM, limites.toleranciaRPM);

  // A janela de vibração (~0,32 s) dá o ritmo do ciclo
  VibResultado vib = {};
  bool okVib = vibracaoLerJanela(rpmAtual, vib);

  // Partida = corrente subiu acima de CORR_PARADO_A (reinicia a carência)
  unsigned long agora = millis();
  bool lig = corrente >= CORR_PARADO_A;
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
  if (!okVib) delay(500);
}
