
#include <avr/wdt.h>             // Watchdog Timer

#include "Andon.h"
#include "SensorCorrente.h"
#include "SensorPT100.h"
#include "SensorVibracao.h"
#include "SensorRPM.h"
#include "LogicaAvaliacao.h"
#include "Display.h"
#include "LoggerSD.h"

// Motor 127V: setpointCorr=2.9, toleranciaCorr=0.4, corrGrave=4.0
// Motor 220V: setpointCorr=1.4, toleranciaCorr=0.2, corrGrave=2.0
const LimitesAvaliacao limites = {
  60.0, 20.0, 100.0,          // temperatura: setpoint, tolerancia, grave
  2.9,  0.4,  4.0,            // corrente:    setpoint, tolerancia, grave
  1740.0, 140.0, 1400.0, 2000.0, // RPM: setpoint, tolerancia, grave min/max
  2.0,  4.0                      // vibracao (amplitude/baseline): ruim, grave
};

void verificarPerifericos() {
  displayMsg(F("Verificando"), F("sistema..."));
  delay(1000);

  // Display
  displayStatus(F("Display"), true);
  delay(500);

  // PT100
  bool okPT100 = verificarPT100();
  displayStatus(F("PT100"), okPT100);
  delay(500);

  // ACS712
  bool okACS712 = verificarACS712();
  displayStatus(F("ACS712"), okACS712);
  delay(800);

  displayMsg(F("Verificando"), F("mais sensores..."));

  // TCRT5000
  bool okTCRT5000 = verificarTCRT5000();
  displayStatus(F("TCRT5000"), okTCRT5000);
  delay(500);

  // ADXL345 (motor parado: mede o piso de ruído)
  float pisoRuido;
  bool okADXL345 = verificarADXL345(pisoRuido);
  displayStatus(F("ADXL345"), okADXL345);
  delay(500);

  // Andon
  displayMsg(F("Verificando"), F("Torre Andon..."));
  setAndon(ANDON_GRAVE);   delay(300);
  setAndon(ANDON_DEFEITO); delay(300);
  setAndon(ANDON_BOM);     delay(300);
  setAndon(ANDON_GRAVE);
  displayStatus(F("Torre Andon"), true);
  delay(500);

  // Cartão SD (informativo: falha aqui não impede o resumo final)
  sdDisponivel = loggerSDInit();
  displayStatus(F("Cartao SD"), sdDisponivel);
  delay(500);

  // Resumo
  bool tudo_ok = okPT100 && okACS712 && okTCRT5000 && okADXL345;

  displayFase1Resultado(tudo_ok, okPT100, okACS712, okTCRT5000, okADXL345);

  if (tudo_ok) setAndon(ANDON_BOM);
  else { piscarAndon(ANDON_DEFEITO, 3, 300); setAndon(ANDON_DEFEITO); }
}

void countdown45s() {
  unsigned long inicio  = millis();
  unsigned long duracao = 45000UL;

  while (millis() - inicio < duracao) {
    wdt_reset();  // alimenta watchdog durante countdown
    unsigned long restante = (duracao - (millis() - inicio)) / 1000;
    displayFase2Countdown(restante);
    delay(500);
  }

  displayMsg(F("INICIANDO"), F("LEITURA DO MOTOR"));
  piscarAndon(ANDON_BOM, 3, 200);
  setAndon(ANDON_BOM);
  delay(1500);
  displayLimpar();
}

void setup() {
  // Desabilita watchdog residual de reset anterior
  wdt_disable();

  Serial.begin(9600);
  Serial.println(F("=== BANCADA PREDITIVA v6.0 ==="));

  // Módulos de hardware
  andonInit();
  setAndon(ANDON_GRAVE);

  if (!vibracaoInit()) Serial.println(F("ADXL345 nao respondeu no boot"));
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
  wdt_enable(WDTO_8S);

  Serial.println(F("Temp(C)\tCorr(A)\tRPM\tErros\tMotor\tEstado\tFreq(Hz)\t1X(g)\t2X(g)\tAlta(g)\tRazao\tCalib"));
  if (sdDisponivel) Serial.println(F("Gravando em LOG.CSV no cartao SD"));
  else              Serial.println(F("Cartao SD indisponivel - gravacao desabilitada"));
}

void loop() {

  wdt_reset();  // alimenta watchdog a cada ciclo

  while (Serial.available()) {
    if (Serial.read() == 'c') vibracaoRecalibrar();
  }

  // Leituras
  float temperatura = lerTemperatura();
  float corrente    = lerCorrente();
  calcularRPM(limites.setpointRPM, limites.toleranciaRPM);

  // A janela de vibração (~0,32 s) dá o ritmo do ciclo
  VibResultado vib = {};
  bool okVib = vibracaoLerJanela(rpmAtual, vib);

  // Monta a leitura atual para a lógica pura de decisão
  LeituraAtual leitura;
  leitura.temperatura  = temperatura;
  leitura.corrente     = corrente;
  leitura.rpm          = rpmAtual;
  leitura.motorJaGirou = motorJaGirou;
  leitura.vibRazao       = vibRazao;
  leitura.vibCalibrado   = (vibCalib != VIB_CALIBRANDO);
  leitura.erroSensorVib  = erroSensorVib;
  leitura.erroSensorTemp = erroSensor;

  int erros = 0;
  EstadoAndon estado = avaliarEstado(leitura, limites, erros);
  vibracaoCalibrar(rpmAtual, estadoMotor, erros);

  // Histerese assimétrica: piora aparece na hora, melhora só depois
  // de alguns ciclos seguidos (ver Andon.h). 'estado' vira o valor já
  // filtrado a partir daqui — Andon, Serial e CSV mostram o mesmo
  // estado, sem risco de log e torre divergirem entre si.
  estado = estabilizarAndon(estado);

  // Atualiza Andon
  setAndon(estado);

  // Atualiza display — tela fixa 16x2 com o essencial (temperatura,
  // corrente, RPM, frequência dominante); estado/erros ficam só na
  // torre Andon.
  EstadoVibTela telaVib = VIB_TELA_OK;
  if      (erroSensorVib)              telaVib = VIB_TELA_ERRO;
  else if (rpmAtual == 0)              telaVib = VIB_TELA_PARADO;
  else if (vibCalib == VIB_CALIBRANDO) telaVib = VIB_TELA_CALIBRANDO;
  atualizarDisplay(temperatura, corrente, vib.freqDominante, telaVib);

  // Log Serial
  Serial.print(temperatura, 2); Serial.print(F("\t"));
  Serial.print(corrente, 3);    Serial.print(F("\t"));
  Serial.print((int)rpmAtual);  Serial.print(F("\t"));
  Serial.print(erros);          Serial.print(F("\t"));
  Serial.print(motorParaTexto(estadoMotor)); Serial.print(F("\t"));
  Serial.print(andonParaTexto(estado));      Serial.print(F("\t"));
  Serial.print(vib.freqDominante, 1);        Serial.print(F("\t"));
  Serial.print(vib.amp1X, 4);                Serial.print(F("\t"));
  Serial.print(vib.amp2X, 4);                Serial.print(F("\t"));
  Serial.print(vib.ampAlta, 4);              Serial.print(F("\t"));
  Serial.print(vibRazao, 2);                 Serial.print(F("\t"));
  Serial.println((int)vibCalib);

  // Grava a mesma leitura no cartão SD (se disponível)
  gravarLeituraSD(temperatura, corrente, estado, erros, vib, vibRazao);

  // Sem janela válida o ciclo não tem ritmo próprio: evita inundar Serial/SD
  if (!okVib) delay(500);
}
