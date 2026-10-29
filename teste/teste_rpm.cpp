// g++ -I teste -I Software teste/teste_rpm.cpp -o /tmp/teste_rpm && /tmp/teste_rpm
#include <assert.h>
#include <math.h>
#include "SensorRPM.h"

static RpmEstado E;
static uint32_t t;   // relógio simulado (us)

static void zerar() { E = RpmEstado(); t = 1000000; }
// n pulsos espaçados de p us (reducao/minUs como no padrão)
static void pulsos(int n, uint32_t p) { for (int i = 0; i < n; i++) { t += p; rpmPulso(E, t, 10000); } }
static float rpm(uint32_t timeoutUs = 500000, uint8_t red = 1) { return rpmCalcular(E, t, timeoutUs, red, 1); }

int main() {
  // 1) 34483 us -> 1740 RPM
  zerar(); pulsos(6, 34483);
  assert(fabsf(rpm() - 1740) <= 1);

  // 2) 33520 us -> 1790 RPM (vazio)
  zerar(); pulsos(6, 33520);
  assert(fabsf(rpm() - 1790) <= 1);

  // 3) pulso 5 ms depois do anterior: rejeitado, RPM igual
  zerar(); pulsos(6, 34483);
  float antes = rpm();
  t += 5000; rpmPulso(E, t, 10000);
  assert(rpm() == antes);
  t += 29483; rpmPulso(E, t, 10000);            // o pulso real seguinte ainda mede 34483 us
  assert(fabsf(rpm() - antes) <= 1);

  // 4) sem pulso por mais de 500 ms -> 0
  zerar(); pulsos(6, 34483);
  t += 499000; assert(rpm() > 0);
  t += 2000;   assert(rpm() == 0);

  // 5) média: 4 períodos iguais = instantâneo; após RPM 0 o buffer é zerado
  zerar(); pulsos(5, 34483);                    // 1º pulso só marca; 4 períodos
  assert(fabsf(rpm() - 60e6f / 34483) < 0.01f);
  t += 600000; assert(rpm() == 0);
  for (int i = 0; i < RPM_AMOSTRAS; i++) assert(E.buf[i] == 0);
  pulsos(1, 34483); assert(rpm() == 0);         // 1º pulso após parada: sem período
  pulsos(3, 34483); assert(rpm() == 0);         // ainda sem RPM_AMOSTRAS períodos (não herda os antigos)
  pulsos(1, 34483); assert(fabsf(rpm() - 1740) <= 1);
  // a média suaviza: um período fora (+10%) move o RPM só ~2,4%
  pulsos(1, 37931); assert(fabsf(rpm() - 1740) < 1740 * 0.03f && rpm() < 1740);

  // 6) montagem antiga: redução 26, período na saída 896552 us -> ~1740
  zerar();
  for (int i = 0; i < 6; i++) { t += 896552; rpmPulso(E, t, 200000); }
  assert(fabsf(rpm(2000000, 26) - 1740) <= 1);

  puts("OK");
  return 0;
}
