#ifndef LOGICA_AVALIACAO_H
#define LOGICA_AVALIACAO_H

#include "Andon.h"   // EstadoAndon

// avaliarEstado() não toca em pino, sensor ou global: tudo chega por
// parâmetro, o que a torna testável sem a bancada.
struct LeituraAtual {
  float temperatura;
  float corrente;
  float rpm;
  float vibRazao;        // amplitude / baseline (pior de 1X, 2X, ALTA); 0 sem baseline
  bool  erroSensorVib;   // ADXL345 sem janela válida
  bool  erroSensorTemp;  // MAX31865 acusou fault: 'temperatura' não vale
  unsigned long msDesdePartida;  // ms desde que a corrente passou de corrParadoA
  unsigned long msRpmZero;       // ms seguidos com RPM 0 desde a partida
};

struct LimitesAvaliacao {
  float setpointRPM;
  float toleranciaRPM;
  float rpmGraveMin;
  float rpmGraveMax;
  float vibMultRuim;    // razão acima disso = RUIM
  float vibMultGrave;   // razão acima disso = PERIGOSO
  float tempRuimC;      // temperatura e corrente: só limite superior
  float tempGraveC;
  float corrRuimA;
  float corrGraveA;
  float corrParadoA;    // abaixo disso o motor está parado
  unsigned long carenciaMs;     // após a partida: corrente e RPM não são avaliados
  unsigned long rpmZeroRuimMs;  // corrente > corrParadoA e RPM 0 por mais que isso = RUIM
  int   errosGrave;     // erros somados que viram GRAVE
};

// RUIM = fora do ideal; PERIGOSO = além do limite grave. 'erros' = RUIM + PERIGOSO.
//   corrente < corrParadoA                      -> ANDON_APAGADO (erros = 0)
//   alguma variável PERIGOSO, OU erros >= errosGrave -> ANDON_GRAVE
//   erros >= 1                                  -> ANDON_DEFEITO
//   caso contrário                              -> ANDON_BOM
EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros);

#endif
