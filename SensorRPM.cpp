#include "SensorRPM.h"

// Contador de pulsos e referência de tempo são detalhe interno do
// módulo — privados ao arquivo (static)
static volatile unsigned long pulsos          = 0;
static unsigned long          ultimoCalculoRPM = 0;

float       rpmAtual    = 0.0;
EstadoMotor estadoMotor  = MOTOR_PARADO;
bool        motorJaGirou = false;

// Nível de repouso do TCRT5000 (lido uma vez em rpmInit)
static int tcrt5000_repouso = LOW;

static void ISR_tcrt5000() {
  pulsos++;
}

void rpmInit() {
  pinMode(PIN_TCRT5000, INPUT);

  // Mesma técnica do SW-420: detecta o repouso e dispara na borda que
  // SAI dele (borda única, nunca CHANGE — evita contagem dupla).
  tcrt5000_repouso = digitalRead(PIN_TCRT5000);
  int modoDisparo = (tcrt5000_repouso == LOW) ? RISING : FALLING;

  attachInterrupt(digitalPinToInterrupt(PIN_TCRT5000), ISR_tcrt5000, modoDisparo);
  ultimoCalculoRPM = millis();
}

#define TCRT_AMOSTRAS_VERIFICACAO     10
#define TCRT_INTERVALO_VERIFICACAO_US 200

static bool nivelRepousoEstavel(uint8_t pino, int nivelEsperado) {
  for (int i = 0; i < TCRT_AMOSTRAS_VERIFICACAO; i++) {
    if (digitalRead(pino) != nivelEsperado) return false;
    delayMicroseconds(TCRT_INTERVALO_VERIFICACAO_US);
  }
  return true;
}

bool verificarTCRT5000() {
  return nivelRepousoEstavel(PIN_TCRT5000, tcrt5000_repouso);
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
