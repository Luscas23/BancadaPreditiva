#ifndef DESTINO_H
#define DESTINO_H

// ----------------------------------------------------------------
//  DESTINO — Passo 12
//
//  Antes: cada enum (EstadoMotor, EstadoAndon) tinha seu switch de
//  enum->texto repetido nos lugares que precisavam imprimir o nome
//  (Serial e SD hoje; o LCD usava até o Passo 11). Toda opção nova
//  no enum precisava ser lembrada nos switches, sem nada avisando se
//  algum fosse esquecido.
//
//  Agora: motorParaTexto() e andonParaTexto() (em SensorRPM e Andon)
//  centralizam esse mapeamento — um único switch por enum. As
//  strings às vezes precisam ser diferentes por destino (ex.: Serial
//  usa "PAROU!!!" pra chamar atenção de quem tá acompanhando ao
//  vivo; o CSV do SD usa "PAROU" puro pra não sujar dado que depois
//  vai ser importado/plotado), então a função recebe o destino e
//  decide o texto certo — em vez do destino decidir qual switch
//  chamar.
// ----------------------------------------------------------------
enum Destino {
  DESTINO_SERIAL,
  DESTINO_SD
};

#endif
