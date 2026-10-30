#ifndef SENSOR_RPM_H
#define SENSOR_RPM_H

#include <Arduino.h>
#include "config.h"

typedef enum {
  MOTOR_PARADO,
  MOTOR_ACELERANDO,
  MOTOR_OPERANDO
} EstadoMotor;

extern float       rpmAtual;
extern EstadoMotor estadoMotor;

void rpmInit();

void calcularRPM(float setpointRPM, float toleranciaRPM);

const __FlashStringHelper* motorParaTexto(EstadoMotor motor);

static_assert(RPM_AMOSTRAS >= 1 && (RPM_AMOSTRAS & (RPM_AMOSTRAS - 1)) == 0, "RPM_AMOSTRAS: potencia de 2");

struct RpmEstado {
  volatile uint32_t buf[RPM_AMOSTRAS];
  volatile uint32_t ultimoUs;
  volatile uint8_t  idx, n;
  volatile bool     temPulso;
};

inline void rpmPulso(RpmEstado &e, uint32_t agora, uint32_t minUs) {
  if (e.temPulso) {
    uint32_t p = agora - e.ultimoUs;
    if (p < minUs) return;
    e.buf[e.idx] = p;
    e.idx = (e.idx + 1) & (RPM_AMOSTRAS - 1);
    if (e.n < RPM_AMOSTRAS) e.n++;
  }
  e.temPulso = true;
  e.ultimoUs = agora;
}

inline float rpmCalcular(RpmEstado &e, uint32_t agora, uint32_t timeoutUs, uint8_t reducao, uint8_t marcas) {
  uint32_t c[RPM_AMOSTRAS];
  uint8_t  n;
  noInterrupts();
  if (e.temPulso && (int32_t)(agora - e.ultimoUs) > (int32_t)timeoutUs) {
    e.temPulso = false; e.idx = e.n = 0;
    for (uint8_t i = 0; i < RPM_AMOSTRAS; i++) e.buf[i] = 0;
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
