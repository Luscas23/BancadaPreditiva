#ifndef LOGICA_AVALIACAO_H
#define LOGICA_AVALIACAO_H

#include "Andon.h"   // EstadoAndon

// ----------------------------------------------------------------
//  Limites de temperatura e corrente — só limite superior.
//  Motor 127 V / 325 W: 2,9 A nominais. Ajustar na bancada.
// ----------------------------------------------------------------
#define TEMP_RUIM_C          80.0
#define TEMP_GRAVE_C        100.0
#define CORR_RUIM_A           3.3
#define CORR_GRAVE_A          4.0
#define CORR_PARADO_A         0.3      // abaixo disso o motor está parado
#define CARENCIA_PARTIDA_MS  5000      // após a partida: corrente e RPM não são avaliados
#define RPM_ZERO_RUIM_MS     3000      // corrente > CORR_PARADO_A e RPM 0 por mais que isso = RUIM

// avaliarEstado() não toca em pino, sensor ou global: tudo chega por
// parâmetro, o que a torna testável sem a bancada.
struct LeituraAtual {
  float temperatura;
  float corrente;
  float rpm;
  float vibRazao;        // amplitude / baseline (pior de 1X, 2X, ALTA); 0 sem baseline
  bool  erroSensorVib;   // ADXL345 sem janela válida
  bool  erroSensorTemp;  // MAX31865 acusou fault: 'temperatura' não vale
  unsigned long msDesdePartida;  // ms desde que a corrente passou de CORR_PARADO_A
  unsigned long msRpmZero;       // ms seguidos com RPM 0 desde a partida
};

struct LimitesAvaliacao {
  float setpointRPM;
  float toleranciaRPM;
  float rpmGraveMin;
  float rpmGraveMax;
  float vibMultRuim;    // razão acima disso = RUIM
  float vibMultGrave;   // razão acima disso = PERIGOSO
};

// RUIM = fora do ideal; PERIGOSO = além do limite grave. 'erros' = RUIM + PERIGOSO.
//   corrente < CORR_PARADO_A                    -> ANDON_APAGADO (erros = 0)
//   alguma variável PERIGOSO, OU erros >= 3     -> ANDON_GRAVE
//   erros >= 1                                  -> ANDON_DEFEITO
//   caso contrário                              -> ANDON_BOM
EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros);

#endif
