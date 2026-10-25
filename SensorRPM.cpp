#include "SensorRPM.h"

// Contador de pulsos e referência de tempo são detalhe interno do
// módulo — privados ao arquivo (static)
static volatile unsigned long pulsos          = 0;
static unsigned long          ultimoCalculoRPM = 0;

float       rpmAtual    = 0.0;
EstadoMotor estadoMotor  = MOTOR_PARADO;
bool        motorJaGirou = false;

static void ISR_hall() {
  pulsos++;
}

void rpmInit() {
  pinMode(PIN_HALL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_HALL), ISR_hall, RISING);
  ultimoCalculoRPM = millis();
}

bool verificarHall() {
  // Corrigido: com INPUT_PULLUP, repouso é LOW (HIGH = ausente ou ímã passando)
  return (digitalRead(PIN_HALL) == LOW);
}

void calcularRPM(float setpointRPM, float toleranciaRPM) {
  unsigned long agora     = millis();
  unsigned long intervalo = agora - ultimoCalculoRPM;
  if (intervalo >= INTERVALO_RPM_MS) {
    noInterrupts();
    unsigned long p = pulsos;
    pulsos = 0;
    interrupts();

    float novoRPM = (p / (float)PULSOS_POR_VOLTA) * (60000.0 / intervalo);

    // Detecta estado do motor
    bool dentroDaFaixa = (novoRPM >= setpointRPM - toleranciaRPM) &&
                         (novoRPM <= setpointRPM + toleranciaRPM);
    estadoMotor = (novoRPM > 0)
                    ? (dentroDaFaixa ? MOTOR_OPERANDO : MOTOR_ACELERANDO)
                    : (motorJaGirou  ? MOTOR_PAROU    : MOTOR_PARADO);
    if (novoRPM > 0) motorJaGirou = true;

    rpmAtual         = novoRPM;
    ultimoCalculoRPM = agora;
  }
}

const __FlashStringHelper* motorParaTexto(EstadoMotor motor) {
  static const __FlashStringHelper* const nomes[] = {
    F("PARADO"), F("ACELERANDO"), F("OPERANDO"), F("PAROU")
  };
  return nomes[motor];
}
