#ifndef LOGGER_SD_H
#define LOGGER_SD_H

#include "Andon.h"

#define PIN_SD_CS  4   // mesmo barramento SPI do PT100

// true = cartão presente e funcionando; false = ausente/falhou.
// A bancada continua monitorando o motor normalmente mesmo se false.
extern bool sdDisponivel;

bool loggerSDInit();          // monta o cartão e cria o CSV com cabeçalho se preciso
void loggerSDIniciarTempo();  // zera a referência t=0 (chamar no início da Fase 3)
void gravarLeituraSD(float temperatura, float corrente, EstadoAndon estado, int erros);

#endif
