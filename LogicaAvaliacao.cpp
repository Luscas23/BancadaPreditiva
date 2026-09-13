#include "LogicaAvaliacao.h"

int contarErros(const LeituraAtual& leitura, const LimitesAvaliacao& limites, bool &grave) {
  grave     = false;
  int erros = 0;

  // Vibração grave — nível crítico imediato
  if (leitura.vibr2) { grave = true; return 4; }

  // Temperatura
  if (leitura.temperatura > limites.tempGrave) { grave = true; return 4; }
  float minTemp = limites.setpointTemp - limites.toleranciaTemp;
  float maxTemp = limites.setpointTemp + limites.toleranciaTemp;
  if (leitura.temperatura < minTemp || leitura.temperatura > maxTemp) erros++;

  // Corrente
  if (leitura.corrente > limites.corrGrave) { grave = true; return 4; }
  float minCorr = limites.setpointCorr - limites.toleranciaCorr;
  float maxCorr = limites.setpointCorr + limites.toleranciaCorr;
  if (leitura.corrente < minCorr || leitura.corrente > maxCorr) erros++;

  // RPM — só avalia se o motor já girou
  if (leitura.motorJaGirou && leitura.rpm > 0) {
    if (leitura.rpm < limites.rpmGraveMin || leitura.rpm > limites.rpmGraveMax) {
      grave = true; return 4;
    }
    float minRPM = limites.setpointRPM - limites.toleranciaRPM;
    float maxRPM = limites.setpointRPM + limites.toleranciaRPM;
    if (leitura.rpm < minRPM || leitura.rpm > maxRPM) erros++;
  }

  // Vibração faixa 1
  if (leitura.vibr1) erros++;

  return erros;
}

EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros) {
  bool grave = false;
  erros = contarErros(leitura, limites, grave);

  if (grave || erros >= 3) return ANDON_GRAVE;
  if (erros >= 1)          return ANDON_DEFEITO;
  return ANDON_BOM;
}
