// g++ -I teste -I . teste/teste_logica.cpp LogicaAvaliacao.cpp -o /tmp/teste_logica && /tmp/teste_logica
#include <assert.h>
#include <stdio.h>
#include "LogicaAvaliacao.h"

static const LimitesAvaliacao L = {60, 20, 100, 2.9, 0.4, 4.0, 1740, 140, 1400, 2000, 2.0, 4.0};

static EstadoAndon avalia(float razao, bool calibrado, bool erroVib, int &erros) {
  LeituraAtual l = {60, 2.9, 1740, true, razao, calibrado, erroVib, false};
  return avaliarEstado(l, L, erros);
}

int main() {
  int e;
  assert(avalia(0.0, true,  false, e) == ANDON_BOM      && e == 0);  // motor parado
  assert(avalia(2.0, true,  false, e) == ANDON_BOM      && e == 0);  // limite do ideal
  assert(avalia(2.5, true,  false, e) == ANDON_DEFEITO  && e == 1);  // ruim
  assert(avalia(4.0, true,  false, e) == ANDON_DEFEITO  && e == 1);  // limite do ruim
  assert(avalia(4.5, true,  false, e) == ANDON_GRAVE    && e == 1);  // perigoso
  assert(avalia(9.0, false, false, e) == ANDON_BOM      && e == 0);  // sem baseline nao pontua
  assert(avalia(0.0, true,  true,  e) == ANDON_DEFEITO  && e == 1);  // erro do sensor = ruim
  puts("OK");
  return 0;
}
