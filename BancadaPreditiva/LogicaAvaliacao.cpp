#include "LogicaAvaliacao.h"

EstadoAndon avaliarEstado(const LeituraAtual& l, const LimitesAvaliacao& lim, int &erros) {
  int ruim = 0, perigoso = 0;
  erros = 0;

  if (l.corrente < lim.corrParadoA) return ANDON_APAGADO;

  if (l.erroSensorTemp)                  ruim++;
  else if (l.temperatura > lim.tempGraveC) perigoso++;
  else if (l.temperatura > lim.tempRuimC)  ruim++;

  if (l.msDesdePartida >= lim.carenciaMs) {
    if (l.corrente > lim.corrGraveA)      perigoso++;
    else if (l.corrente > lim.corrRuimA)  ruim++;

    if (l.rpm == 0) {
      if (l.msRpmZero > lim.rpmZeroRuimMs) ruim++;
    } else if (l.rpm < lim.rpmGraveMin || l.rpm > lim.rpmGraveMax) {
      perigoso++;
    } else if (l.rpm < lim.setpointRPM - lim.toleranciaRPM ||
               l.rpm > lim.setpointRPM + lim.toleranciaRPM) {
      ruim++;
    }
  }

  if (l.erroSensorVib)                    ruim++;
  else if (l.vibRazao > lim.vibMultGrave) perigoso++;
  else if (l.vibRazao > lim.vibMultRuim)  ruim++;

  erros = ruim + perigoso;

  if (perigoso > 0 || erros >= lim.errosGrave) return ANDON_GRAVE;
  if (erros >= 1)                 return ANDON_DEFEITO;
  return ANDON_BOM;
}
