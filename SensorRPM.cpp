#include "SensorRPM.h"

// Contador de pulsos e referência de tempo são detalhe interno do
// módulo — privados ao arquivo (static)
static volatile unsigned long pulsos          = 0;
static unsigned long          ultimoCalculoRPM = 0;

float       rpmAtual    = 0.0;
EstadoMotor estadoMotor  = MOTOR_PARADO;
bool        motorJaGirou = false;

static volatile unsigned long ultimoPulsoUs = 0;

static void ISR_tcrt5000() {
  unsigned long agora = micros();
  if (agora - ultimoPulsoUs < PULSO_MIN_US) return;
  ultimoPulsoUs = agora;
  pulsos++;
}

void rpmInit() {
  // TCRT5000 "pelado" (sem LM393): usar INPUT_PULLUP + resistor
  pinMode(PIN_TCRT5000, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_TCRT5000), ISR_tcrt5000, RISING);
  ultimoCalculoRPM = millis();
}

static bool pinoEstavel(uint8_t pino) {
  int primeiro = digitalRead(pino);
  for (int i = 1; i < TCRT_AMOSTRAS_VERIFICACAO; i++) {
    delayMicroseconds(TCRT_INTERVALO_VERIFICACAO_US);
    if (digitalRead(pino) != primeiro) return false;
  }
  return true;
}

bool verificarTCRT5000() {
  return pinoEstavel(PIN_TCRT5000);
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
