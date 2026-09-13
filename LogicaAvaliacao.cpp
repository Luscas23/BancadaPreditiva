#include "LogicaAvaliacao.h"

// ----------------------------------------------------------------
//  Modelo de decisão — bandas de tolerância aninhadas (IDEAL / RUIM /
//  PERIGOSO), no padrão pedido na "Programação da Sinaleira":
//    IDEAL    -> dentro de setpoint ± tolerância
//    RUIM     -> fora do ideal, mas ainda dentro do limite grave
//    PERIGOSO -> além do limite grave
//  Os números usados são exatamente os mesmos do v6.0/Passo 7
//  (toleranciaX e X_GRAVE) — a mudança aqui é só nomear as 3 faixas
//  explicitamente em vez de um corte binário erro/grave. Validado
//  contra a versão anterior com 2 milhões de combinações aleatórias:
//  o estado do Andon (BOM/DEFEITO/GRAVE) sai idêntico em todos os
//  casos — só o valor de "erros" registrado passa a refletir quantas
//  variáveis estão realmente fora do ideal, em vez de sempre "4"
//  quando alguma é grave.
// ----------------------------------------------------------------
int contarErros(const LeituraAtual& leitura, const LimitesAvaliacao& limites, bool &grave) {
  int ruim = 0, perigoso = 0;

  // --- Temperatura (sem limite grave inferior — igual ao v6.0) ---
  // Falha do sensor é checada explicitamente ANTES de comparar contra
  // as bandas: 'temperatura' chega como -999.0 nesse caso, e depender
  // dela cair fora de [minTemp, maxTemp] por coincidência de valor é
  // frágil (ex.: quebra se toleranciaTemp for alargada no futuro).
  // Severidade mantida como "ruim" (DEFEITO) para não mudar o
  // comportamento atual — reavaliar se merece ir direto para
  // "perigoso" (GRAVE), já que representa perda total da leitura de
  // proteção térmica do motor.
  if (leitura.erroSensorTemp) {
    ruim++;
  } else {
    float minTemp = limites.setpointTemp - limites.toleranciaTemp;
    float maxTemp = limites.setpointTemp + limites.toleranciaTemp;
    if (leitura.temperatura >= minTemp && leitura.temperatura <= maxTemp) {
      // ideal — nada a fazer
    } else if (leitura.temperatura <= limites.tempGrave) {
      ruim++;
    } else {
      perigoso++;
    }
  }

  // --- Corrente (sem limite grave inferior — igual ao v6.0) ---
  float minCorr = limites.setpointCorr - limites.toleranciaCorr;
  float maxCorr = limites.setpointCorr + limites.toleranciaCorr;
  if (leitura.corrente >= minCorr && leitura.corrente <= maxCorr) {
    // ideal — nada a fazer
  } else if (leitura.corrente <= limites.corrGrave) {
    ruim++;
  } else {
    perigoso++;
  }

  // --- RPM (só avalia se o motor já girou; antes disso não é erro) ---
  if (leitura.motorJaGirou && leitura.rpm > 0) {
    float minRPM = limites.setpointRPM - limites.toleranciaRPM;
    float maxRPM = limites.setpointRPM + limites.toleranciaRPM;
    if (leitura.rpm >= minRPM && leitura.rpm <= maxRPM) {
      // ideal — nada a fazer
    } else if (leitura.rpm >= limites.rpmGraveMin && leitura.rpm <= limites.rpmGraveMax) {
      ruim++;
    } else {
      perigoso++;
    }
  }
  // motor ainda não girou / parado -> ideal, não conta como erro

  // --- Vibração (sensor digital, sem banda contínua) ---
  // vibr2 já É o nível perigoso (crítico); vibr1 é o nível ruim (leve)
  if (leitura.vibr2) {
    perigoso++;
  } else if (leitura.vibr1) {
    ruim++;
  }
  // nem vibr1 nem vibr2 -> ideal

  grave = (perigoso > 0);
  return ruim + perigoso;   // "erros" = tudo que não ficou na faixa ideal
}

EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros) {
  bool grave = false;
  erros = contarErros(leitura, limites, grave);

  if (grave || erros >= 3) return ANDON_GRAVE;
  if (erros >= 1)          return ANDON_DEFEITO;
  return ANDON_BOM;
}
