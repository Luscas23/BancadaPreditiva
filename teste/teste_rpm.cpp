#include <assert.h>
#include <math.h>
#include "SensorRPM.h"

static RpmEstado E;
static uint32_t t;

static void zerar() { E = RpmEstado(); t = 1000000; }
static void pulsos(int n, uint32_t p) { for (int i = 0; i < n; i++) { t += p; rpmPulso(E, t, 10000); } }
static float rpm(uint32_t timeoutUs = 500000, uint8_t red = 1) { return rpmCalcular(E, t, timeoutUs, red, 1); }

int main() {
  zerar(); pulsos(6, 34483);
  assert(fabsf(rpm() - 1740) <= 1);

  zerar(); pulsos(6, 33520);
  assert(fabsf(rpm() - 1790) <= 1);

  zerar(); pulsos(6, 34483);
  float antes = rpm();
  t += 5000; rpmPulso(E, t, 10000);
  assert(rpm() == antes);
  t += 29483; rpmPulso(E, t, 10000);
  assert(fabsf(rpm() - antes) <= 1);

  zerar(); pulsos(6, 34483);
  t += 499000; assert(rpm() > 0);
  t += 2000;   assert(rpm() == 0);

  zerar(); pulsos(5, 34483);
  assert(fabsf(rpm() - 60e6f / 34483) < 0.01f);
  t += 600000; assert(rpm() == 0);
  for (int i = 0; i < RPM_AMOSTRAS; i++) assert(E.buf[i] == 0);
  pulsos(1, 34483); assert(rpm() == 0);
  pulsos(3, 34483); assert(rpm() == 0);
  pulsos(1, 34483); assert(fabsf(rpm() - 1740) <= 1);
  pulsos(1, 37931); assert(fabsf(rpm() - 1740) < 1740 * 0.03f && rpm() < 1740);

  zerar();
  for (int i = 0; i < 6; i++) { t += 896552; rpmPulso(E, t, 200000); }
  assert(fabsf(rpm(2000000, 26) - 1740) <= 1);

  puts("OK");
  return 0;
}
