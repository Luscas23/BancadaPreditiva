// g++ -I teste -I . teste/teste_logica.cpp LogicaAvaliacao.cpp -o /tmp/teste_logica && /tmp/teste_logica
#include <assert.h>
#include <stdio.h>
#include "LogicaAvaliacao.h"

static const LimitesAvaliacao L = {1740, 140, 1400, 2000, 2.0, 4.0};

// Motor em regime (partida há 10 s, RPM girando) salvo o que o teste mudar
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
  assert(vib(0.0, false, e) == ANDON_BOM      && e == 0);  // sem baseline: razao 0
  assert(vib(2.0, false, e) == ANDON_BOM      && e == 0);  // limite do ideal
  assert(vib(2.5, false, e) == ANDON_DEFEITO  && e == 1);  // ruim
  assert(vib(4.0, false, e) == ANDON_DEFEITO  && e == 1);  // limite do ruim
  assert(vib(4.5, false, e) == ANDON_GRAVE    && e == 1);  // perigoso
  assert(vib(0.0, true,  e) == ANDON_DEFEITO  && e == 1);  // erro do sensor = ruim

  // motor frio e sem carga
  assert(av(25, 1.5, 1790, 0, false, 10000, 0, e) == ANDON_BOM    && e == 0);
  // corrente acima do grave
  assert(av(25, 4.5, 1790, 0, false, 10000, 0, e) == ANDON_GRAVE);
  // motor parado (corrente < 0,3 A)
  assert(av(25, 0.1, 0,    0, false, 0,     0, e) == ANDON_APAGADO && e == 0);

  // limites superiores
  assert(av(85,  2.9, 1740, 0, false, 10000, 0, e) == ANDON_DEFEITO && e == 1);  // temp ruim
  assert(av(105, 2.9, 1740, 0, false, 10000, 0, e) == ANDON_GRAVE);              // temp grave
  assert(av(25,  3.5, 1790, 0, false, 10000, 0, e) == ANDON_DEFEITO && e == 1);  // corrente ruim
  assert(av(25,  2.9, 1500, 0, false, 10000, 0, e) == ANDON_DEFEITO && e == 1);  // RPM fora da faixa
  assert(av(25,  2.9, 1300, 0, false, 10000, 0, e) == ANDON_GRAVE);              // RPM grave

  // corrente > 0,3 A com RPM 0: so e RUIM depois de 3 s
  assert(av(25, 2.9, 0, 0, false, 10000, 3500, e) == ANDON_DEFEITO && e == 1);
  assert(av(25, 2.9, 0, 0, false, 10000, 2500, e) == ANDON_BOM     && e == 0);

  // carencia de 5 s: pico de corrente e RPM 0 nao contam
  assert(av(25, 6.0, 0, 0, false, 1000, 1000, e) == ANDON_BOM && e == 0);
  assert(av(25, 6.0, 0, 0, false, 4999, 4999, e) == ANDON_BOM && e == 0);
  assert(av(25, 6.0, 900, 0, false, 5000, 0, e) == ANDON_GRAVE);

  puts("OK");
  return 0;
}
