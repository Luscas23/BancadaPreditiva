#include "SensorRPM.h"

static volatile unsigned long ultimoPulsoUs = 0;
static volatile unsigned long periodoUs     = 0;     // 0 = sem período válido
static volatile bool          temPulso      = false; // false após parada: 1º pulso não dá período

float       rpmAtual    = 0.0;
EstadoMotor estadoMotor = MOTOR_PARADO;

static void ISR_tcrt5000() {
  unsigned long agora = micros();
  if (temPulso) {
    if (agora - ultimoPulsoUs < PULSO_MIN_US) return;
    periodoUs = agora - ultimoPulsoUs;
  }
  temPulso      = true;
  ultimoPulsoUs = agora;
}

void rpmInit() {
  pinMode(PIN_TCRT5000, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_TCRT5000), ISR_tcrt5000, RISING);
}

void calcularRPM(float setpointRPM, float toleranciaRPM) {
  noInterrupts();   // periodoUs tem 32 bits: leitura atômica
  if (temPulso && micros() - ultimoPulsoUs > RPM_TIMEOUT_US) { temPulso = false; periodoUs = 0; }
  unsigned long p = periodoUs;
  interrupts();

  rpmAtual = p ? 60e6f / p * REDUCAO : 0.0f;

  bool dentroDaFaixa = (rpmAtual >= setpointRPM - toleranciaRPM) &&
                       (rpmAtual <= setpointRPM + toleranciaRPM);
  estadoMotor = (rpmAtual > 0) ? (dentroDaFaixa ? MOTOR_OPERANDO : MOTOR_ACELERANDO)
                               : MOTOR_PARADO;
}

const __FlashStringHelper* motorParaTexto(EstadoMotor motor) {
  static const __FlashStringHelper* const nomes[] = { F("PARADO"), F("ACELERANDO"), F("OPERANDO") };
  return nomes[motor];
}
