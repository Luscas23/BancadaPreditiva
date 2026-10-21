#include <math.h>   // INFINITY
#include "LogicaAvaliacao.h"

// ----------------------------------------------------------------
//  Modelo de decisão — bandas de tolerância aninhadas (IDEAL / RUIM /
//  PERIGOSO), no padrão pedido na "Programação da Sinaleira":
//    IDEAL    -> dentro de setpoint ± tolerância
//    RUIM     -> fora do ideal, mas ainda dentro do limite grave
//    PERIGOSO -> além do limite grave
// ----------------------------------------------------------------

// Classifica um valor contra uma faixa setpoint±tolerancia e um
// limite grave de um só lado (lo ou hi; o lado sem limite grave
// passa -INFINITY/INFINITY) e soma em 'ruim'/'perigoso' conforme o
// caso. Reaproveitado para temperatura, corrente e RPM.
static void faixa(float v, float setpoint, float tolerancia,
                   float limiteGraveLo, float limiteGraveHi,
                   int &ruim, int &perigoso) {
  float min = setpoint - tolerancia;
  float max = setpoint + tolerancia;
  if (v >= min && v <= max) return;  // ideal
  if (v >= limiteGraveLo && v <= limiteGraveHi) ruim++;
  else perigoso++;
}

EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros) {
  int ruim = 0, perigoso = 0;

  // Temperatura: falha do sensor é checada explicitamente ANTES de
  // comparar contra as bandas ('temperatura' chega como -999.0 nesse
  // caso). Sem limite grave inferior.
  if (leitura.erroSensorTemp) {
    ruim++;
  } else {
    faixa(leitura.temperatura, limites.setpointTemp, limites.toleranciaTemp,
          -INFINITY, limites.tempGrave, ruim, perigoso);
  }

  // Corrente: sem limite grave inferior.
  faixa(leitura.corrente, limites.setpointCorr, limites.toleranciaCorr,
        -INFINITY, limites.corrGrave, ruim, perigoso);

  // RPM: só avalia se o motor já girou; antes disso não é erro.
  if (leitura.motorJaGirou && leitura.rpm > 0) {
    faixa(leitura.rpm, limites.setpointRPM, limites.toleranciaRPM,
          limites.rpmGraveMin, limites.rpmGraveMax, ruim, perigoso);
  }

  // Vibração (sensor digital, sem banda contínua): vibr2 já É o
  // nível perigoso (crítico); vibr1 é o nível ruim (leve).
  if (leitura.vibr2) perigoso++;
  else if (leitura.vibr1) ruim++;

  erros = ruim + perigoso;

  if (perigoso > 0 || erros >= 3) return ANDON_GRAVE;
  if (erros >= 1)                 return ANDON_DEFEITO;
  return ANDON_BOM;
}
