#include "SensorRPM.h"
#include "Configuracao.h"

static RpmEstado estRpm;

float       rpmAtual    = 0.0;
EstadoMotor estadoMotor = MOTOR_PARADO;

static void ISR_tcrt5000() { rpmPulso(estRpm, micros(), cfg.pulsoMinUs); }

void rpmInit() {
  pinMode(cfg.pinTcrt, INPUT);
  attachInterrupt(digitalPinToInterrupt(cfg.pinTcrt), ISR_tcrt5000, RISING);
}

void calcularRPM(float setpointRPM, float toleranciaRPM) {
  rpmAtual = rpmCalcular(estRpm, micros(), cfg.rpmTimeoutMs * 1000UL, cfg.reducao, cfg.tcrtMarcas);

  bool dentroDaFaixa = (rpmAtual >= setpointRPM - toleranciaRPM) &&
                       (rpmAtual <= setpointRPM + toleranciaRPM);
  estadoMotor = (rpmAtual > 0) ? (dentroDaFaixa ? MOTOR_OPERANDO : MOTOR_ACELERANDO)
                               : MOTOR_PARADO;
}

const __FlashStringHelper* motorParaTexto(EstadoMotor motor) {
  static const __FlashStringHelper* const nomes[] = { F("PARADO"), F("ACELERANDO"), F("OPERANDO") };
  return nomes[motor];
}
