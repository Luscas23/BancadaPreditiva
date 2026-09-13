# Changelog — Bancada Preditiva

Histórico da refatoração modular. Formato livre, mais recente no topo.

## [Não lançado] — Refatoração modular em andamento

### Passo 7 — Extrai módulo LogicaAvaliacao
- Novo: `LogicaAvaliacao.h` / `LogicaAvaliacao.cpp`, com `contarErros()` e
  `avaliarEstado()` migrados do `.ino`.
- Novo: struct `LeituraAtual` (temperatura, corrente, rpm, motorJaGirou,
  vibr1, vibr2) — tudo que a lógica de decisão precisa saber sobre o motor
  em um dado ciclo.
- Novo: struct `LimitesAvaliacao` (setpoints/tolerâncias de temperatura,
  corrente e RPM) — reúne numa struct só o que antes eram `#define`/globais
  soltos, preparando terreno para trocar perfil de motor (127V/220V) sem
  tocar na lógica de decisão.
- `contarErros()`/`avaliarEstado()` não leem mais nenhuma variável global de
  hardware — recebem tudo via `const LeituraAtual&` e `const LimitesAvaliacao&`.
  Nenhuma regra de negócio mudou: mesma ordem de checagem, mesmos limiares,
  mesmo retorno antecipado (4) em condição grave.
- `LogicaAvaliacao.h`/`.cpp` não incluem `<Arduino.h>` — só `Andon.h` (que
  também não inclui `<Arduino.h>`), para reaproveitar o enum `EstadoAndon`.
  Esse é o primeiro módulo 100% livre de dependência de hardware: já está
  pronto para testes nativos se o projeto migrar para PlatformIO.
- `main.ino`: `loop()` agora monta um `LeituraAtual` a partir das leituras
  do ciclo e chama `avaliarEstado(leitura, limites, erros)`. Os setpoints
  continuam como globais no `.ino` por enquanto; são empacotados uma única
  vez em `LimitesAvaliacao limites`.

### Passo 6 — Extrai módulo SensorRPM
- Novo: `SensorRPM.h` / `SensorRPM.cpp`: pino do Hall, `EstadoMotor`,
  `rpmAtual`/`estadoMotor`/`motorJaGirou`, a ISR (agora `static`) e
  `calcularRPM()`.
- `rpmInit()` concentra `pinMode`, `attachInterrupt` e a inicialização da
  referência de tempo.
- `verificarHall()` encapsula a checagem da Fase 1.
- `calcularRPM()` passou a receber `setpointRPM`/`toleranciaRPM` por
  parâmetro em vez de ler os globais diretamente.

### Passo 5 — Extrai módulo SensorVibracao
- Novo: `SensorVibracao.h` / `SensorVibracao.cpp`: pinos dos dois SW-420,
  `vibr1`/`vibr2` (volatile), debounce, timestamps da última vibração
  (`ultimaVibr1Ms`/`ultimaVibr2Ms`) e o estado de repouso detectado na
  Fase 1.
- `vibracaoInit()` concentra `pinMode` e os dois `attachInterrupt`.
- `verificarVibracao()` encapsula a checagem da Fase 1.
- As ISRs mantêm a mesma lógica de debounce; o `loop()` do `.ino` continua
  responsável por capturar o snapshot atômico (`noInterrupts()`/
  `interrupts()`) antes de qualquer avaliação.

### Passo 4 — Extrai módulo SensorPT100
- Novo: `SensorPT100.h` / `SensorPT100.cpp`: pino, constantes (`PT100_RNOM`,
  `PT100_RREF`), o objeto `pt100`, o buffer da média móvel e
  `pt100Init()`, `lerTemperatura()`, `verificarPT100()`.
- `erroSensor` virou `extern bool`: continua acessível pelo `.ino` (o
  display ainda o usa) sem ficar duplicado.
- Buffer da média móvel e índice agora são `static` dentro do `.cpp`.

### Passo 3 — Extrai módulo SensorCorrente
- Novo: `SensorCorrente.h` / `SensorCorrente.cpp`: pino do ACS712,
  constantes e `lerCorrente()`/`verificarACS712()`.
- `verificarPerifericos()` agora chama `okACS712 = verificarACS712()` em
  vez de fazer o `analogRead`/threshold manualmente.

### Passo 2 — Extrai módulo Andon
- Novo: `Andon.h` / `Andon.cpp`: pinos, `enum EstadoAndon`, `andonInit()`,
  `setAndon()`, `piscarAndon()`.
- `setup()` passou a chamar `andonInit()` em vez de 3 `pinMode()` soltos.

### Passo 1 — Baseline congelada em Git
- Código v6.0 copiado sem alteração para `BancadaPreditiva/`.
- Repositório Git iniciado (commit `baseline`, depois `.gitignore`).
- `README.md` e este `CHANGELOG.md` criados.

## v6.0 — Baseline monolítica (antes da refatoração)
- Watchdog | Média móvel PT100 | Detecção de motor parado
- Flags de vibração corrigidas | Contagem de erros unificada
- Tempo desde última vibração | SW-420 mais robusto (RISING em vez de CHANGE)
- `lerCorrente()` com RMS para corrente CA
- Verificação do Hall corrigida (repouso = LOW com `INPUT_PULLUP`)
- Snapshots de vibração capturados atomicamente antes da avaliação
- Gravação das leituras em cartão SD (`LOG.CSV`), tolerante a falha
