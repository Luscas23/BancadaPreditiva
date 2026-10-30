#ifndef LOGGER_SD_H
#define LOGGER_SD_H

#include "Andon.h"
#include "SensorVibracao.h"   // VibResultado

// true = cartão presente e funcionando. A bancada monitora igual se false.
extern bool sdDisponivel;

bool loggerSDMontar();        // só SD.begin: o CONFIG.TXT é lido antes do log
// Cria o CSV com cabeçalho se preciso e anota a configuração usada (linhas '#').
// Chamar depois de configValidar().
bool loggerSDInit();
void loggerSDIniciarTempo();  // zera a referência t=0 (chamar no fim do setup)

// Imprime a leitura atual (sem o tempo) em 'p' com o separador 'sep'.
// Usada pelo Serial (tab) e pelo CSV (vírgula).
void imprimirLinha(Print &p, char sep, float temperatura, float corrente, EstadoAndon estado,
                   int erros, const VibResultado &vib);

void gravarLeituraSD(float temperatura, float corrente, EstadoAndon estado, int erros,
                     const VibResultado &vib);

#endif
