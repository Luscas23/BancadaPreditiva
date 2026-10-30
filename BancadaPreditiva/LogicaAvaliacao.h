#ifndef LOGICA_AVALIACAO_H
#define LOGICA_AVALIACAO_H

#include "Andon.h"

struct LeituraAtual {
  float temperatura;
  float corrente;
  float rpm;
  float vibRazao;
  bool  erroSensorVib;
  bool  erroSensorTemp;
  unsigned long msDesdePartida;
  unsigned long msRpmZero;
};

struct LimitesAvaliacao {
  float setpointRPM;
  float toleranciaRPM;
  float rpmGraveMin;
  float rpmGraveMax;
  float vibMultRuim;
  float vibMultGrave;
  float tempRuimC;
  float tempGraveC;
  float corrRuimA;
  float corrGraveA;
  float corrParadoA;
  unsigned long carenciaMs;
  unsigned long rpmZeroRuimMs;
  int   errosGrave;
};

EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros);

#endif
