#include <assert.h>
#include <stdio.h>
#include "LogicaAvaliacao.h"
#include "Configuracao.h"

static LimitesAvaliacao L;

static EstadoAndon av(float temp, float corr, float rpm, float razao, bool erroVib,
                      unsigned long msPartida, unsigned long msRpmZero, int &erros) {
  LeituraAtual l = {temp, corr, rpm, razao, erroVib, false, msPartida, msRpmZero};
  return avaliarEstado(l, L, erros);
}

static EstadoAndon vib(float razao, bool erroVib, int &erros) {
  return av(60, 2.9, 1740, razao, erroVib, 10000, 0, erros);
}

int main() {
  int e;
  configPadrao();
  L = limitesDeConfig();
  assert(L.setpointRPM == 1740 && L.toleranciaRPM == 140 && L.rpmGraveMin == 1400 && L.rpmGraveMax == 2000);
  assert(L.vibMultRuim == 2.0f && L.vibMultGrave == 4.0f && L.tempRuimC == 80 && L.tempGraveC == 100);
  assert(L.corrRuimA == 3.3f && L.corrGraveA == 4.0f && L.corrParadoA == 0.3f);
  assert(L.carenciaMs == 5000 && L.rpmZeroRuimMs == 3000 && L.errosGrave == 3);
  assert(vib(0.0, false, e) == ANDON_BOM      && e == 0);
  assert(vib(2.0, false, e) == ANDON_BOM      && e == 0);
  assert(vib(2.5, false, e) == ANDON_DEFEITO  && e == 1);
  assert(vib(4.0, false, e) == ANDON_DEFEITO  && e == 1);
  assert(vib(4.5, false, e) == ANDON_GRAVE    && e == 1);
  assert(vib(0.0, true,  e) == ANDON_DEFEITO  && e == 1);

  assert(av(25, 1.5, 1790, 0, false, 10000, 0, e) == ANDON_BOM    && e == 0);
  assert(av(25, 4.5, 1790, 0, false, 10000, 0, e) == ANDON_GRAVE);
  assert(av(25, 0.1, 0,    0, false, 0,     0, e) == ANDON_APAGADO && e == 0);

  assert(av(85,  2.9, 1740, 0, false, 10000, 0, e) == ANDON_DEFEITO && e == 1);
  assert(av(105, 2.9, 1740, 0, false, 10000, 0, e) == ANDON_GRAVE);
  assert(av(25,  3.5, 1790, 0, false, 10000, 0, e) == ANDON_DEFEITO && e == 1);
  assert(av(25,  2.9, 1500, 0, false, 10000, 0, e) == ANDON_DEFEITO && e == 1);
  assert(av(25,  2.9, 1300, 0, false, 10000, 0, e) == ANDON_GRAVE);

  assert(av(25, 2.9, 0, 0, false, 10000, 3500, e) == ANDON_DEFEITO && e == 1);
  assert(av(25, 2.9, 0, 0, false, 10000, 2500, e) == ANDON_BOM     && e == 0);

  assert(av(25, 6.0, 0, 0, false, 1000, 1000, e) == ANDON_BOM && e == 0);
  assert(av(25, 6.0, 0, 0, false, 4999, 4999, e) == ANDON_BOM && e == 0);
  assert(av(25, 6.0, 900, 0, false, 5000, 0, e) == ANDON_GRAVE);

  puts("OK");
  return 0;
}
