#ifndef SENSOR_RPM_H
#define SENSOR_RPM_H

#include <Arduino.h>

// ----------------------------------------------------------------
//  RPM — TCRT5000 no eixo de saída da redução (ver cfg.reducao),
//  marcas por volta em cfg.tcrtMarcas.
//  RPM do motor = 60e6 / período(us) * cfg.reducao / cfg.tcrtMarcas.
//  Pulsos mais próximos que cfg.pulsoMinUs são ruído; sem pulso por
//  cfg.rpmTimeoutMs: RPM = 0.
// ----------------------------------------------------------------

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
