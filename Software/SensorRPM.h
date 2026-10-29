#ifndef SENSOR_RPM_H
#define SENSOR_RPM_H

#include <Arduino.h>
#include "config.h"   // RPM_AMOSTRAS

// ----------------------------------------------------------------
//  RPM — TCRT5000 no eixo do motor, cfg.tcrtMarcas marcas por volta (1).
//  RPM = 60e6 / média dos últimos RPM_AMOSTRAS períodos(us) * cfg.reducao / cfg.tcrtMarcas.
//  cfg.reducao = 1 com o sensor no motor; só muda se o sensor voltar à saída de uma redução.
//  Pulsos mais próximos que cfg.pulsoMinUs são ruído; sem pulso por
//  cfg.rpmTimeoutMs: RPM = 0 e a média é esquecida.
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

// ---- Núcleo puro: sem pino nem micros(), roda no PC (teste/teste_rpm.cpp) ----
static_assert(RPM_AMOSTRAS >= 1 && (RPM_AMOSTRAS & (RPM_AMOSTRAS - 1)) == 0, "RPM_AMOSTRAS: potencia de 2");

struct RpmEstado {
  volatile uint32_t buf[RPM_AMOSTRAS];  // últimos períodos válidos (us), circular
  volatile uint32_t ultimoUs;
  volatile uint8_t  idx, n;             // n = períodos válidos no buffer (satura em RPM_AMOSTRAS)
  volatile bool     temPulso;           // false após parada: o 1º pulso não dá período
};

// Chamada pela ISR: só aritmética inteira.
inline void rpmPulso(RpmEstado &e, uint32_t agora, uint32_t minUs) {
  if (e.temPulso) {
    uint32_t p = agora - e.ultimoUs;
    if (p < minUs) return;                          // ruído: não move o ultimoUs
    e.buf[e.idx] = p;
    e.idx = (e.idx + 1) & (RPM_AMOSTRAS - 1);
    if (e.n < RPM_AMOSTRAS) e.n++;
  }
  e.temPulso = true;
  e.ultimoUs = agora;
}

// RPM do motor; 0 = parado ou ainda sem RPM_AMOSTRAS períodos válidos.
inline float rpmCalcular(RpmEstado &e, uint32_t agora, uint32_t timeoutUs, uint8_t reducao, uint8_t marcas) {
  uint32_t c[RPM_AMOSTRAS];
  uint8_t  n;
  noInterrupts();   // só a cópia; a conta fica fora
  // Diferença com sinal: pulso que chegou entre 'agora' e o noInterrupts() não vira timeout falso
  if (e.temPulso && (int32_t)(agora - e.ultimoUs) > (int32_t)timeoutUs) {
    e.temPulso = false; e.idx = e.n = 0;
    for (uint8_t i = 0; i < RPM_AMOSTRAS; i++) e.buf[i] = 0;   // próxima partida não herda períodos
  }
  n = e.n;
  for (uint8_t i = 0; i < RPM_AMOSTRAS; i++) c[i] = e.buf[i];
  interrupts();

  if (n < RPM_AMOSTRAS) return 0.0f;
  uint32_t soma = 0;
  for (uint8_t i = 0; i < RPM_AMOSTRAS; i++) soma += c[i];
  return 60e6f / (soma / RPM_AMOSTRAS) * reducao / marcas;
}

#endif
