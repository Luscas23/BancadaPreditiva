#ifndef LOGICA_AVALIACAO_H
#define LOGICA_AVALIACAO_H

#include "Andon.h"   // reaproveita EstadoAndon (ANDON_BOM/DEFEITO/GRAVE)

// ----------------------------------------------------------------
//  MÓDULO: LogicaAvaliacao
//
//  avaliarEstado() não toca em nenhum pino, sensor ou variável
//  global de hardware: tudo que precisa chega por parâmetro. Isso
//  é o que torna a lógica testável sem a bancada montada.
// ----------------------------------------------------------------

// Snapshot de tudo que a lógica de decisão precisa saber sobre o
// estado atual do motor em um dado ciclo. Preenchido pelo main.ino
// a partir das leituras dos sensores.
struct LeituraAtual {
  float temperatura;
  float corrente;
  float rpm;
  bool  motorJaGirou;
  float vibRazao;        // amplitude / baseline (pior de 1X, 2X, ALTA)
  bool  vibCalibrado;    // false = ainda sem baseline; vibração não pontua
  bool  erroSensorVib;   // true = ADXL345 sem janela válida
  bool  erroSensorTemp;  // true = MAX31865 acusou fault neste ciclo;
                          // 'temperatura' vem como -999.0 e não deve
                          // ser comparada contra as bandas normais
};

// Setpoints e tolerâncias usados para avaliar as leituras.
struct LimitesAvaliacao {
  float setpointTemp;
  float toleranciaTemp;
  float tempGrave;

  float setpointCorr;
  float toleranciaCorr;
  float corrGrave;

  float setpointRPM;
  float toleranciaRPM;
  float rpmGraveMin;
  float rpmGraveMax;

  float vibMultRuim;    // razão acima disso = RUIM
  float vibMultGrave;   // razão acima disso = PERIGOSO
};

// Classifica cada variável em 3 faixas (IDEAL / RUIM / PERIGOSO), no
// padrão da "Sinaleira": IDEAL = dentro do setpoint±tolerância,
// RUIM = fora do ideal mas dentro do limite grave, PERIGOSO = além
// do limite grave. 'erros' sai com a soma de RUIM+PERIGOSO. Decide o
// estado do Andon:
//   alguma variável PERIGOSO, OU erros >= 3  -> ANDON_GRAVE
//   erros >= 1                                -> ANDON_DEFEITO
//   caso contrário                             -> ANDON_BOM
EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros);

#endif
