#ifndef SENSOR_RPM_H
#define SENSOR_RPM_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  RPM — Sensor Hall KY-003
// ----------------------------------------------------------------
#define PIN_HALL           18    // RPM Hall KY-003   (INT5)
#define PULSOS_POR_VOLTA   1
#define INTERVALO_RPM_MS   1000

// Melhoria 6 — Estado do motor
typedef enum {
  MOTOR_PARADO,        // RPM = 0 desde o início
  MOTOR_ACELERANDO,    // RPM > 0 mas ainda abaixo do setpoint
  MOTOR_OPERANDO,      // RPM dentro da faixa nominal
  MOTOR_PAROU          // RPM era > 0 e voltou a 0 (parada inesperada)
} EstadoMotor;

extern float       rpmAtual;
extern EstadoMotor estadoMotor;
extern bool        motorJaGirou;

// Configura o pino do Hall (INPUT_PULLUP) e liga a interrupção RISING
void rpmInit();

// Verificação de presença/sanidade na Fase 1 — com INPUT_PULLUP o
// repouso é LOW (HIGH = sensor ausente ou ímã passando no instante)
bool verificarHall();

// Recalcula o RPM a cada INTERVALO_RPM_MS e atualiza EstadoMotor.
// setpointRPM/toleranciaRPM continuam sendo setpoints de negócio
// configuráveis no .ino — são passados por parâmetro para o módulo
// não precisar conhecer variáveis globais de configuração.
void calcularRPM(float setpointRPM, float toleranciaRPM);

#endif
