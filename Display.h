#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include "Andon.h"

// O LCD é um recurso do Display — outros módulos (Fase 1/2 no .ino)
// ainda usam esse objeto diretamente para as telas de boot; isso é
// intencional por enquanto e será resolvido no Passo 9 (main.ino
// como orquestrador fino).
extern LiquidCrystal_I2C lcd;

void displayInit();
void atualizarDisplay(float temperatura, float corrente, EstadoAndon estado, int erros);

#endif
