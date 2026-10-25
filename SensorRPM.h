#ifndef SENSOR_RPM_H
#define SENSOR_RPM_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  RPM — Sensor óptico reflexivo TCRT5000
// ----------------------------------------------------------------
#define PIN_TCRT5000       18    // RPM TCRT5000      (INT3)
#define PULSOS_POR_VOLTA   1
#define INTERVALO_RPM_MS   1000

// Pulsos mais próximos que isso são ruído/reflexo espúrio. A 2000 RPM
// o período é 30 ms; 5 ms deixa folga até ~12000 RPM.
#define PULSO_MIN_US       5000

#define TCRT_AMOSTRAS_VERIFICACAO      10
#define TCRT_INTERVALO_VERIFICACAO_US  200

typedef enum {
  MOTOR_PARADO,        // RPM = 0 desde o início
  MOTOR_ACELERANDO,    // RPM > 0 mas ainda abaixo do setpoint
  MOTOR_OPERANDO,      // RPM dentro da faixa nominal
  MOTOR_PAROU          // RPM era > 0 e voltou a 0 (parada inesperada)
} EstadoMotor;

extern float       rpmAtual;
extern EstadoMotor estadoMotor;
extern bool        motorJaGirou;

// Configura o pino do TCRT5000 (INPUT, módulo com comparador LM393) e
// liga a interrupção RISING. A borda é fixa: com a marca refletiva em
// posição qualquer no boot não existe nível de repouso, e qualquer
// borda ocorre uma vez por volta.
void rpmInit();

// Verificação de presença/sanidade na Fase 1: o pino deve manter o
// mesmo nível em leituras seguidas (pino flutuando oscila por ruído).
bool verificarTCRT5000();

// Recalcula o RPM a cada INTERVALO_RPM_MS e atualiza EstadoMotor.
// setpointRPM/toleranciaRPM continuam sendo setpoints de negócio
// configuráveis no .ino — são passados por parâmetro para o módulo
// não precisar conhecer variáveis globais de configuração.
void calcularRPM(float setpointRPM, float toleranciaRPM);

// Único lugar que sabe o nome de cada EstadoMotor. Serial e SD
// chamam esta função em vez de terem cada um o próprio switch.
const __FlashStringHelper* motorParaTexto(EstadoMotor motor);

#endif
