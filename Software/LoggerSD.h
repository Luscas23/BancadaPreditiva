#ifndef LOGGER_SD_H
#define LOGGER_SD_H

#include "Andon.h"
#include "SensorVibracao.h"   // VibResultado

#define PIN_SD_CS  4   // mesmo barramento SPI do PT100

// true = cartão presente e funcionando. A bancada monitora igual se false.
extern bool sdDisponivel;

bool loggerSDInit();          // monta o cartão e cria o CSV com cabeçalho se preciso
void loggerSDIniciarTempo();  // zera a referência t=0 (chamar no fim do setup)

// Imprime a leitura atual (sem o tempo) em 'p' com o separador 'sep'.
// Usada pelo Serial (tab) e pelo CSV (vírgula).
void imprimirLinha(Print &p, char sep, float temperatura, float corrente, EstadoAndon estado,
                   int erros, const VibResultado &vib);

void gravarLeituraSD(float temperatura, float corrente, EstadoAndon estado, int erros,
                     const VibResultado &vib);

#endif
