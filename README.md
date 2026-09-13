# Bancada Preditiva de Motores Elétricos

TCC — Engenharia Mecatrônica — Lucas Altruda Salce

Bancada de manutenção preditiva para motores elétricos, com leitura de
temperatura (PT100 + MAX31865), corrente (ACS712-5A), rotação (sensor
Hall KY-003) e vibração (2x SW-420), sinalização Andon e gravação de
log em cartão SD.

## Ambiente

Arduino IDE, usando abas (cada aba é um `.h`/`.cpp` na mesma pasta do
`.ino`). Abra `BancadaPreditiva.ino` — as demais abas aparecem
automaticamente ao lado.

## Progresso da modularização

O código nasceu como um único arquivo `.ino` (v6.0, ver `CHANGELOG.md`)
e está sendo dividido em módulos por hardware/responsabilidade, para
que novas mudanças não exijam mexer em várias partes do arquivo ao
mesmo tempo. Módulos extraídos até agora:

- [x] **Passo 1** — Baseline v6.0 congelada em Git.
- [x] **Passo 2** — `Andon.h/.cpp` (torre de sinalização).
- [x] **Passo 3** — `SensorCorrente.h/.cpp` (leitura RMS do ACS712 +
      verificação de presença na Fase 1).
- [x] **Passo 4** — `SensorPT100.h/.cpp` (leitura + média móvel + fault +
      verificação de presença na Fase 1).
- [ ] **Passo 5** — `SensorVibracao.h/.cpp` (ISRs SW-420 + debounce +
      tempo desde última vibração).
- [ ] **Passo 6** — `SensorRPM.h/.cpp` (ISR Hall + cálculo de RPM +
      `EstadoMotor`).
- [ ] **Passo 7** — `LogicaAvaliacao.h/.cpp` (`contarErros()` /
      `avaliarEstado()` sem hardware — puro cálculo, testável).
- [ ] **Passo 8** — `Display.h/.cpp` e `LoggerSD.h/.cpp`.
- [ ] **Passo 9** — `main`/`.ino` como orquestrador fino (`setup()`/`loop()`
      só chamando módulos).
- [ ] **Passo 10** — Novas mudanças do TCC, já encaixadas nos módulos
      certos.

Cada módulo é extraído um de cada vez: extrai → compila → grava na
bancada real → confirma que o comportamento não mudou → só então segue
para o próximo. Isso isola o risco: se algo quebrar, dá pra saber
exatamente qual módulo causou.

## Estrutura atual

```
BancadaPreditiva/
├── BancadaPreditiva.ino   // setup()/loop() + o que ainda não foi extraído
├── Andon.h / Andon.cpp
├── SensorCorrente.h / SensorCorrente.cpp
├── SensorPT100.h / SensorPT100.cpp
├── README.md
└── CHANGELOG.md
```

## Configuração de motor (127V x 220V)

Atualmente os setpoints/tolerâncias de corrente (`setpointCorr`,
`toleranciaCorr`, `CORR_GRAVE`) ainda estão como variáveis/`#define`
soltos no `.ino` — comentário original já documenta os dois perfis
(127V e 220V). A troca dinâmica entre perfis está planejada para
quando `Config.h` for extraído (ver plano de modularização).
