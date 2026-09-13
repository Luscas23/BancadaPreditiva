#ifndef LOGICA_AVALIACAO_H
#define LOGICA_AVALIACAO_H

#include "Andon.h"   // reaproveita EstadoAndon (ANDON_BOM/DEFEITO/GRAVE)

// ----------------------------------------------------------------
//  MÓDULO: LogicaAvaliacao
//  Extraído no Passo 7 da modularização.
//
//  Este é o módulo mais importante para o TCC: contarErros() e
//  avaliarEstado() não tocam em nenhum pino, sensor ou variável
//  global de hardware. Tudo que precisam chega por parâmetro.
//  Isso é o que torna a lógica testável sem a bancada montada —
//  e, como nem este header nem o .cpp incluem <Arduino.h>, o módulo
//  já está pronto para compilar nativamente (ex.: testes com
//  PlatformIO) se o projeto migrar dessa forma no futuro.
// ----------------------------------------------------------------

// Snapshot de tudo que a lógica de decisão precisa saber sobre o
// estado atual do motor em um dado ciclo. Preenchido pelo main.ino
// a partir das leituras dos sensores.
struct LeituraAtual {
  float temperatura;
  float corrente;
  float rpm;
  bool  motorJaGirou;
  bool  vibr1;
  bool  vibr2;
};

// Setpoints e tolerâncias usados para avaliar as leituras. Reúne numa
// struct só o que antes eram #defines/globais soltos pelo .ino,
// preparando terreno para trocar de perfil de motor (127V/220V) no
// futuro sem tocar na lógica de decisão.
struct LimitesAvaliacao {
  float setpointTemp;
  float toleranciaTemp;
  float tempGrave;

  float setpointCorr;
  float toleranciaCorr;
  float corrGrave;

  float setpointRPM;
  float toleranciaRPM;
  float rpmGraveMin;
  float rpmGraveMax;
};

// Classifica cada variável em 3 faixas (IDEAL / RUIM / PERIGOSO), no
// padrão da "Sinaleira": IDEAL = dentro do setpoint±tolerância,
// RUIM = fora do ideal mas dentro do limite grave, PERIGOSO = além
// do limite grave. Retorna a soma de RUIM+PERIGOSO ("erros") e
// sinaliza 'grave' se alguma variável caiu em PERIGOSO.
int contarErros(const LeituraAtual& leitura, const LimitesAvaliacao& limites, bool &grave);

// Usa contarErros() para decidir o estado do Andon:
//   grave OU erros >= 3  -> ANDON_GRAVE
//   erros >= 1            -> ANDON_DEFEITO
//   caso contrário         -> ANDON_BOM
EstadoAndon avaliarEstado(const LeituraAtual& leitura, const LimitesAvaliacao& limites, int &erros);

#endif
