#ifndef SENSOR_RPM_H
#define SENSOR_RPM_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  RPM — TCRT5000 no eixo de saída da redução 1:26 (~67 RPM),
//  1 marca refletiva. RPM do motor = 60e6 / período(us) * REDUCAO.
// ----------------------------------------------------------------
#define PIN_TCRT5000       18    // INT3
#define REDUCAO            26

// Período real ~870 ms: pulsos mais próximos que isso são ruído
#define PULSO_MIN_US       200000UL
// Sem pulso por mais tempo que isso: RPM = 0
#define RPM_TIMEOUT_US     2000000UL

typedef enum {
  MOTOR_PARADO,        // RPM = 0
  MOTOR_ACELERANDO,    // RPM > 0 fora da faixa nominal
  MOTOR_OPERANDO       // RPM dentro da faixa nominal
} EstadoMotor;

extern float       rpmAtual;
extern EstadoMotor estadoMotor;

// INPUT (módulo com LM393; TCRT "pelado" exige INPUT_PULLUP + resistor)
// e interrupção RISING.
void rpmInit();

// Atualiza rpmAtual e estadoMotor. Chamar a cada ciclo do loop().
void calcularRPM(float setpointRPM, float toleranciaRPM);

const __FlashStringHelper* motorParaTexto(EstadoMotor motor);

#endif
