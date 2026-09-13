#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

void displayInit();

// Tela principal do loop (Fase 3) — Passo 7/8, redesenhada no Passo 11
// para caber em 16x2: tela FIXA, sem rodízio entre telas. Mostra só o
// essencial (temperatura, corrente, RPM e vibração); Andon/erros
// continuam sinalizados pela torre física, não pelo LCD.
void atualizarDisplay(float temperatura, float corrente);


// setup() — tela de abertura. Correção: hardware real é 16x2 (não
// 20x4, como o código antigo assumia implicitamente ao escrever nas
// linhas 2/3). Mostra 2 telas de 2 linhas em sequência, com o mesmo
// delay total (2000ms) que já existia.
void displayTelaBoot();

// Fase 1 — verificação de periféricos. Correção: cada teste agora é
// sua própria tela de 2 linhas (nome do item na linha 0, resultado
// na linha 1) em vez de escrever em linhas/colunas que não existem
// num display 16x2. Os nomes das funções e a ordem de chamada no
// .ino não mudaram, só o conteúdo interno de cada uma.
void displayFase1Inicio();                 // tela: "Verificando" / "sistema..."
void displayFase1StatusDisplay();          // tela: "Display" / "OK"
void displayFase1StatusPT100(bool ok);     // tela: "PT100" / "OK ou ERRO"
void displayFase1StatusACS712(bool ok);    // tela: "ACS712" / "OK ou ERRO"
void displayFase1Continuacao();            // tela de transição: "Verificando" / "mais sensores..."
void displayFase1StatusHall(bool ok);      // tela: "Sensor Hall" / "OK ou ERRO"
void displayFase1StatusVibr1(bool ok);     // tela: "Vibr. SW1" / "OK ou ERRO" — agora recebe o resultado real (ver SensorVibracao.h)
void displayFase1StatusVibr2(bool ok);     // tela: "Vibr. SW2" / "OK ou ERRO" — idem
void displayFase1TelaAndon();              // tela: "Verificando" / "Torre Andon..." (mantida durante o pisca-pisca de teste)
void displayFase1StatusAndon();            // tela: "Torre Andon" / "OK"
void displayFase1StatusSD(bool ok);        // tela: "Cartao SD" / "OK ou ERRO"
void displayFase1Resultado(bool tudoOk, bool okPT100, bool okACS712, bool okHall, bool okVibr1, bool okVibr2);

// Fase 2 — countdown de 45s. Correção: displayFase2Countdown() agora
// escreve as 2 linhas inteiras a cada chamada (antes escrevia só na
// linha 3, contando com conteúdo de displayFase1Resultado() ainda
// visível acima — não existe "acima" num display de 2 linhas).
void displayFase2Countdown(unsigned long segundosRestantes);
void displayFase2IniciandoLeitura();

// Utilitário genérico (usado no fim da Fase 2)
void displayLimpar();

#endif
