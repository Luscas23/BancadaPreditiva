#ifndef LOGGER_SD_H
#define LOGGER_SD_H

#include "Andon.h"
#include "SensorVibracao.h"

extern bool sdDisponivel;

bool loggerSDMontar();
bool loggerSDInit();
void loggerSDIniciarTempo();

void imprimirLinha(Print &p, char sep, float temperatura, float corrente, EstadoAndon estado,
                   int erros, const VibResultado &vib);

void gravarLeituraSD(float temperatura, float corrente, EstadoAndon estado, int erros,
                     const VibResultado &vib);

#endif
