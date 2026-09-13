# CHANGELOG — Bancada Preditiva

Histórico da refatoração modular do `BancadaPreditiva.ino` (v6.0),
substituindo os comentários de versão (v4/v5/v6) que ficavam no
cabeçalho do código.

## Refatoração modular — Passo 8/10 (Display + LoggerSD)
- Extraído `Display.h` / `Display.cpp`:
  - Objeto `LiquidCrystal_I2C lcd` passa a morar aqui (`extern` para
    quem ainda usa diretamente nas telas de boot: `verificarPerifericos()`,
    `countdown45s()` e `setup()` — isso será resolvido no Passo 9).
  - `displayInit()` concentra `lcd.init()` + `lcd.backlight()`.
  - `atualizarDisplay()` migrada do `.ino`; assinatura ganhou dois
    parâmetros (`temperatura`, `corrente`) porque essas variáveis ainda
    não têm módulo próprio — mesmo padrão de "ajuste de assinatura"
    usado em `calcularRPM()` no Passo 6.
- Extraído `LoggerSD.h` / `LoggerSD.cpp`:
  - `iniciarSD()` renomeada para `loggerSDInit()`.
  - `inicioLeituraMs` agora é `static` dentro do módulo; setado via
    nova função `loggerSDIniciarTempo()`, chamada no início da Fase 3.
  - `gravarLeituraSD()` migrada, mesma assinatura + `temperatura`/`corrente`.
  - `sdDisponivel` continua `extern bool` (mesmo padrão do `erroSensor`).
- Nenhuma lógica alterada — checagem estática confirmou que `lcd`,
  `sdDisponivel`, `erroSensor`, `vibr1`/`vibr2`, `rpmAtual`, `estadoMotor`
  e `motorJaGirou` têm exatamente uma definição real cada, sem duplicidade.

## Refatoração modular — Passo 7/10 (LogicaAvaliacao)
- Extraído `LogicaAvaliacao.h` / `LogicaAvaliacao.cpp`.
- `contarErros()` e `avaliarEstado()` passam a receber tudo por
  parâmetro via `struct LeituraAtual` e `struct LimitesAvaliacao`,
  em vez de ler variáveis globais diretamente.
- Módulo não depende de `<Arduino.h>` — é puro C++, testável sem a
  bancada montada.

## Refatoração modular — Passo 6/10 (SensorRPM)
- Extraído `SensorRPM.h` / `SensorRPM.cpp`: pino Hall, `EstadoMotor`,
  `rpmAtual`/`estadoMotor`/`motorJaGirou`, a ISR (agora `static`) e
  `calcularRPM()`.
- `rpmInit()` concentra `pinMode`, `attachInterrupt` e a referência de
  tempo inicial.
- `calcularRPM()` passou a receber `setpointRPM`/`toleranciaRPM` por
  parâmetro em vez de ler globais — prepara o terreno para o Passo 7.

## Refatoração modular — Passo 5/10 (SensorVibracao)
- Extraído `SensorVibracao.h` / `SensorVibracao.cpp`: as duas ISRs do
  SW-420, debounce e timestamps da última vibração.

## Refatoração modular — Passo 4/10 (SensorPT100)
- Extraído `SensorPT100.h` / `SensorPT100.cpp`: objeto
  `Adafruit_MAX31865`, buffer da média móvel (agora `static`) e
  `pt100Init()`/`lerTemperatura()`/`verificarPT100()`.
- `erroSensor` virou `extern bool`.

## Refatoração modular — Passo 3/10 (SensorCorrente)
- Extraído `SensorCorrente.h` / `SensorCorrente.cpp`: `lerCorrente()`
  com RMS e `verificarACS712()`.

## Refatoração modular — Passo 2/10 (Andon)
- Extraído `Andon.h` / `Andon.cpp`: pinos, `EstadoAndon`,
  `andonInit()`, `setAndon()`, `piscarAndon()`.

## Passo 1/10 — Baseline
- v6.0 congelada como ponto de partida antes da refatoração modular.

---

## Melhorias v4 (histórico pré-refatoração)
Watchdog | Média móvel PT100 | Detecção de motor parado | Flags de
vibração corrigidas | Contagem de erros unificada | Tempo desde
última vibração | SW-420 mais robusto.

## Correções v5 (histórico pré-refatoração)
SW-420 `CHANGE`→`RISING` (evita duplo disparo) | `lerCorrente()` com
RMS para corrente CA | Verificação Hall corrigida (repouso = LOW com
`INPUT_PULLUP`) | Snapshots de vibração capturados atomicamente.

## Adição v6 (histórico pré-refatoração)
Gravação das leituras em cartão SD (`LOG.CSV`, 1 linha por ciclo).
Falha no SD não trava a bancada.

---

## Próximos passos
- **Passo 9**: reescrever `BancadaPreditiva.ino` como orquestrador
  fino (mover as telas de boot de `verificarPerifericos()`/
  `countdown45s()` para dentro de `Display`, remover includes de
  biblioteca que já não são usados diretamente no `.ino`).
- **Passo 10**: testes da lógica pura (`LogicaAvaliacao`), se migrar
  para PlatformIO.
