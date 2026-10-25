#include "LogicaAvaliacao.h"

EstadoAndon avaliarEstado(const LeituraAtual& l, const LimitesAvaliacao& lim, int &erros) {
  int ruim = 0, perigoso = 0;
  erros = 0;

  // Parado = corrente baixa (um TCRT solto não pode parecer motor parado)
  if (l.corrente < CORR_PARADO_A) return ANDON_APAGADO;

  if (l.erroSensorTemp)                  ruim++;
  else if (l.temperatura > TEMP_GRAVE_C) perigoso++;
  else if (l.temperatura > TEMP_RUIM_C)  ruim++;

  // Carência de partida: pico de corrente e RPM subindo não são defeito
  if (l.msDesdePartida >= CARENCIA_PARTIDA_MS) {
    if (l.corrente > CORR_GRAVE_A)      perigoso++;
    else if (l.corrente > CORR_RUIM_A)  ruim++;

    if (l.rpm == 0) {
      if (l.msRpmZero > RPM_ZERO_RUIM_MS) ruim++;   // TCRT solto ou rotor travado
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

  if (perigoso > 0 || erros >= 3) return ANDON_GRAVE;
  if (erros >= 1)                 return ANDON_DEFEITO;
  return ANDON_BOM;
}
