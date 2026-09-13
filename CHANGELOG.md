# Changelog — Bancada Preditiva de Motores Elétricos

Formato: data, versão/etapa, o que mudou, por quê.

## [Modularização] Passo 5 — SensorVibracao extraído
- Criados `SensorVibracao.h` / `SensorVibracao.cpp`.
- Migrados: `PIN_SW420_1`, `PIN_SW420_2`, `DEBOUNCE_MS`, as flags
  `volatile bool vibr1/vibr2`, os timestamps `ultimaVibr1Ms/2Ms`, os
  estados de repouso `sw420_1_repouso/2_repouso`, e as ISRs
  `ISR_vibr1()`/`ISR_vibr2()` (agora `static`, privadas ao módulo).
- Nova função `vibracaoInit()` concentra `pinMode()` dos dois pinos,
  a detecção do repouso (melhoria 3) e os dois `attachInterrupt(...,
  RISING)` — antes espalhados em três pontos diferentes do `setup()`.
- `verificarPerifericos()` não repete mais a leitura de repouso (já
  feita em `vibracaoInit()` antes da Fase 1 começar); mantém os
  mesmos indicadores `okSW420_1`/`okSW420_2` e as mensagens no
  display. Comportamento equivalente: o nível dos pinos não muda
  entre `vibracaoInit()` e a Fase 1.
- `ultimoDebounce1/2` viraram `static` no `.cpp` — são detalhe interno
  do debounce, nenhum outro módulo precisa vê-los.
- Primeiro módulo com interrupções: o contrato de leitura do `loop()`
  principal (`noInterrupts()`/`interrupts()` para tirar um snapshot
  atômico das flags antes de avaliar erros, e zerá-las só depois de
  exibidas no display) foi preservado sem alteração — continua no
  `.ino`, já que ainda é lógica de orquestração, não do sensor em si.
- Checagem estática: nenhuma referência a `PIN_SW420_1/2`,
  `DEBOUNCE_MS`, `ultimoDebounce1/2` ou às ISRs de vibração restou
  fora do módulo; `vibr1`/`vibr2`/timestamps continuam acessados no
  `.ino` exclusivamente via `extern`.

## [Modularização] Passo 4 — SensorPT100 extraído
- Criados `SensorPT100.h` / `SensorPT100.cpp`.
- Migrados: `PIN_MAX31865_CS`, `PT100_RNOM`, `PT100_RREF`, o objeto
  `Adafruit_MAX31865 pt100`, o buffer da média móvel (`MEDIA_MOVEL_N`,
  `bufferTemp`, `indexTemp`, `bufferPreenchido`), `mediaMovelTemp()`,
  `lerTemperatura()` e a checagem de presença do sensor usada na
  Fase 1 (`verificarPT100()`).
- `erroSensor` virou `extern bool` declarado em `SensorPT100.h` e
  definido em `SensorPT100.cpp` — continua acessível pelo `.ino`
  (usado hoje só pelo display) sem precisar duplicar a variável.
- Melhoria de encapsulamento: o buffer da média móvel e seu índice
  agora são `static` (privados ao arquivo `.cpp`) — nenhum outro
  módulo pode mexer neles por acidente, algo que não era garantido
  quando eram globais soltos no `.ino`.
- `setup()` chama `pt100Init()` em vez de `pt100.begin(...)` direto;
  `verificarPerifericos()` chama `verificarPT100()` em vez de acessar
  o objeto `pt100` diretamente.
- Checagem estática: nenhuma referência a `pt100.`, `PT100_RNOM`,
  `PT100_RREF`, `PIN_MAX31865_CS` ou ao buffer da média móvel restou
  fora do módulo.

## [Modularização] Passo 3 — SensorCorrente extraído
- Criados `SensorCorrente.h` / `SensorCorrente.cpp`.
- Migrados: `PIN_ACS712`, `ACS712_SENS`, `ACS712_OFFSET`, `VCC`, `ADC_MAX`,
  `AMOSTRAS_CORRENTE`, a função `lerCorrente()` (RMS) e a checagem de
  presença do sensor usada na Fase 1 (`verificarACS712()`).
- `BancadaPreditiva.ino` passa a incluir `SensorCorrente.h` e chamar
  `lerCorrente()` / `verificarACS712()` — nenhuma lógica foi alterada,
  apenas reposicionamento de código.
- Checagem estática: nenhuma redefinição de `PIN_ACS712` ou das
  constantes do ACS712 restou no `.ino`.

## [Modularização] Passo 2 — Andon extraído
- Criados `Andon.h` / `Andon.cpp`.
- Migrados: `PIN_ANDON_VERDE/AMARELO/VERMELHO`, `enum EstadoAndon`,
  `setAndon()`, `piscarAndon()`.
- Nova função `andonInit()` concentra os três `pinMode()` que antes
  estavam soltos no `setup()`.
- `BancadaPreditiva.ino` passa a incluir `Andon.h` — nenhuma lógica
  foi alterada, apenas reposicionamento de código.

## [Modularização] Passo 1 — Baseline congelada
- Código v6.0 (monolítico) copiado sem alterações para a estrutura de
  projeto do Arduino IDE (pasta `BancadaPreditiva/` = nome do `.ino`).
- Repositório Git iniciado.
- Objetivo: ponto de retorno seguro antes de iniciar a extração dos
  módulos.

---

## v6.0 (histórico anterior à modularização)
- Gravação das leituras em cartão SD (módulo SPI).
- Arquivo `LOG.CSV`: `tempo_s,temp,corrente,rpm,erros,estadoMotor,estadoAndon`
  — 1 linha por ciclo.
- Falha no SD não trava a bancada (monitoramento do motor é a função
  principal; gravação é tratada como recurso secundário tolerante a falha).

## v5.0
- SW-420: `CHANGE` → `RISING` (evita duplo disparo por vibração).
- `lerCorrente()` com RMS para corrente CA (média simples tendia a zero).
- Verificação do sensor Hall corrigida: repouso = LOW com `INPUT_PULLUP`.
- Snapshots de vibração capturados atomicamente e passados para
  `contarErros()` / `avaliarEstado()`.

## v4.0
- Watchdog (reinicia se travar por mais de 8s).
- Média móvel na leitura do PT100.
- Detecção de estado do motor (parado / acelerando / operando / parou).
- Correção das flags de vibração.
- Contagem de erros unificada.
- Tempo desde a última vibração exibido no display.
- Leitura do SW-420 mais robusta.
