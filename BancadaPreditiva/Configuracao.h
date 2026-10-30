#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <Arduino.h>
#include "LogicaAvaliacao.h"   // LimitesAvaliacao
#include "config.h"

// Construção em buffer estático (objetos de biblioteca com pino/endereço de execução).
// Operador próprio, com tag, para não colidir com o do core.
struct ColocarEm {};
inline void *operator new(size_t, void *p, ColocarEm) noexcept { return p; }

// Camada 2: valores em uso. configPadrao() copia os PAD_* de config.h;
// CONFIG.TXT e a validação ajustam depois.
struct Config {
  uint8_t  pinReleVerde, pinReleAmarelo, pinReleVermelho, pinTcrt, pinAcs, pinMaxCs;
  uint8_t  releAtivoEm, enderecoLcd, enderecoAdxl;
  float    motorCorrenteA;
  uint16_t motorTensaoV, motorPotenciaW;
  uint8_t  reducao;
  uint16_t rpmNominal, rpmTolerancia, rpmGraveMin, rpmGraveMax;
  float    corrRuimA, corrGraveA, corrParadoA, tempRuimC, tempGraveC, vibMultRuim, vibMultGrave;
  uint8_t  errosParaGrave;
  uint16_t carenciaPartidaMs, rpmZeroRuimMs, rpmTimeoutMs;
  uint8_t  andonHisterese;
  uint32_t pulsoMinUs;
  uint8_t  tcrtMarcas;
  float    acsSens, acsVcc;
  uint16_t acsAmostras, acsIntervaloUs;
  float    acsPisoA;
  uint16_t acsAdcMin, acsAdcMax;
  uint8_t  ptFios;
  float    ptRref, ptRnom;
  uint8_t  adxlFminHz, adxlFmaxHz, adxlAltaMinHz, adxlBandaBins, adxlJanelasPiso;
  float    adxlFatorPiso;
  uint8_t  vibFalhasErro, baselineJanelas, baselineCiclos;
  float    baselineTolRpm;
};

extern Config cfg;

void configPadrao();                              // cfg = padrões do config.h

// Uma linha do CONFIG.TXT, montada caractere a caractere (buffer fica com o chamador).
struct LinhaCfg { char b[CFG_LINHA_MAX]; uint8_t n; bool longa; };
int configAplicarLinha(char *linha, Print &log);  // 0 = ok, 1 = aviso (valor ignorado)
int configAlimentar(LinhaCfg &l, char c, Print &log);   // '\n' fecha a linha; devolve avisos

int configCarregarSD(Print &log);                 // LoggerSD.cpp; 0 se não há arquivo
int configValidar(Print &log);                    // devolve nº de avisos; grupo ruim volta ao padrão
LimitesAvaliacao limitesDeConfig();
uint16_t configAssinaturaVib();                   // muda se algo que afeta a baseline mudar

// pre: prefixo de cada linha ("# " no log). doc: acrescenta comentário de documentação por chave.
void configImprimir(Print &p, const char *pre, bool doc);

// Acesso à tabela (testes e geração de documentação)
struct ConfigItem {
  char nome[26], un[6], desc[42];
  uint8_t tipo, grupo, baseline;
  uint16_t off;
  float def, min, max;
};
uint8_t configNumItens();
void    configItem(uint8_t i, ConfigItem &it);
float   configValor(const Config &c, const ConfigItem &it);
void    configImprimirValor(Print &p, const ConfigItem &it, float v);

#endif
