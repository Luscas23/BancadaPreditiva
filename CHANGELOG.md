## TCRT5000 no lugar do Hall + ADXL345 no lugar dos 2x SW-420
Mudanças de comportamento:

- **RPM (`SensorRPM`)**: sensor Hall KY-003 trocado por TCRT5000 (pino 18,
  INT3 — o comentário antigo dizia INT5). `verificarHall()` virou
  `verificarTCRT5000()`: 10 leituras a cada 200 us, ok se todas iguais
  (detecta pino flutuando). A ISR rejeita pulsos com menos de
  `PULSO_MIN_US` = 5000 us do anterior. **Polaridade fixa:** `INPUT` +
  `RISING`, sem ler nível de repouso (com a marca refletiva parada em
  posição qualquer no boot não existe repouso, e qualquer borda ocorre uma
  vez por volta). Módulo com LM393 assumido; TCRT5000 "pelado" exige
  `INPUT_PULLUP` + resistor.
- **SW-420 removido**: pinos 2/3, ISRs, `vibr1/vibr2`, `ultimaVibr*Ms` e
  debounce saíram. Os pinos 2/3 ficam livres; INT1 do ADXL345 não é usado.
- **Vibração (`SensorVibracao`)**: reescrito. ADXL345 por Wire direto
  (100 kHz, sem biblioteca Adafruit), ±4 g full-res, 400 Hz, FIFO em
  STREAM. Janela de 128 amostras (~0,32 s, 3,125 Hz/bin), FFT Hann por
  eixo, espectro RSS dos 3 eixos. Janela com overrun do FIFO ou erro de I2C
  é descartada; 3 falhas seguidas = `erroSensorVib` (conta como ruim,
  display `V:ERR`).
- **Avaliação relativa à baseline**: a vibração deixou de ser digital
  (`vibr2` = perigoso, `vibr1` = ruim). Agora é a razão
  amplitude/baseline do pior entre 1X, 2X e banda alta (120–180 Hz), pela
  mediana das últimas 3 janelas: ideal até `vibMultRuim` (2,0), ruim até
  `vibMultGrave` (4,0), perigoso acima. Valores de partida, a ajustar na
  bancada. Sem baseline, vibração não pontua; motor parado, razão 0.
- **Baseline**: trava após 25 ciclos seguidos com motor OPERANDO, `erros`
  == 0 e RPM dentro de `BASELINE_TOL_RPM` da média móvel; valor = média de
  8 janelas seguintes; piso = max(baseline, ruído medido na Fase 1 × 5).
  Gravada na EEPROM só ao calibrar e recarregada no boot (um reset do
  watchdog com o motor já defeituoso não "normaliza" o defeito). Serial
  `c` apaga e recalibra. A flag de validade depende de `ADXL_FS`/`ADXL_N`.
  `BASELINE_TOL_RPM` ficou em 5% e não nos 2% do plano: com 1 pulso por
  volta o RPM tem resolução de ~2,5% a 1740 RPM, e 2% reiniciaria a
  contagem a cada leitura.
- **Fase 1**: uma verificação só, `ADXL345` (DEVID 0xE5, gravidade entre
  0,7 e 1,3 g, piso de ruído em 8 janelas). `displayFase1Resultado` lista
  `TCRT` e `VIB`.
- **Display**: campo de vibração mostra a frequência dominante: `V:29Hz`,
  `C:29Hz` (calibrando), `V:---` (parado), `V:ERR`.
- **Serial e CSV** ganham `freq_vib_hz, vib_1x_g, vib_2x_g, vib_alta_g,
  vib_razao, vib_calib` (0 calibrando, 1 calibrado, 2 baseline da EEPROM).
  **Um `LOG.CSV` antigo no cartão tem o cabeçalho de 7 colunas: apague ou
  renomeie antes de gravar.**
- **`delay(500)` removido do `loop()`**: o ritmo vem da janela (~0,32 s)
  mais o processamento. Só se a janela falha há um `delay(500)`, para não
  inundar Serial/SD.
- **Medições (a preencher na bancada)**: o boot imprime
  `ADXL345: janela X ms, piso Y g` (tempo de coleta + FFT dos 3 eixos);
  a taxa real de linhas do CSV sai da coluna `tempo_s`.
  - Tempo da janela no Mega: ____ ms
  - Linhas por segundo no CSV: ____
- Limites conhecidos: `ADXL_FS` = 800 não fecha com Wire a 100 kHz (~1,3 ms
  por amostra contra 1,25 ms de período), o FIFO estoura e toda janela
  falha. A escala em g de `vib_*_g` depende do fator de compensação do
  `arduinoFFT` (`ADXL_ESCALA_FFT`): conferir com uma vibração conhecida; a
  razão e a avaliação não são afetadas.
- `teste/teste_logica.cpp` cobre as faixas de vibração em
  `avaliarEstado()` e roda no PC.

---

## Limpeza (ponytail-review) — remoção de código morto e duplicado
Passagem de revisão focada só em simplificação (nenhuma correção de bug,
nenhuma feature nova). ~330 linhas a menos no total. Duas mudanças aqui
alteram comportamento observável, marcadas abaixo.

- **`BancadaPreditiva.ino`** — os `#define`/globais de setpoint
  (`setpointTemp`, `TEMP_GRAVE` etc.) foram escritos direto dentro de
  `limites` (agora `const`), em vez de existirem soltos só para serem
  copiados uma vez no boot. `okDisplay`, `okPT100`, `okACS712`, `okHall`,
  `okSW420_1/2`, `okAndon`, `okSD`, `temperatura` e `corrente` viraram
  variáveis locais das funções que realmente usam (eram globais sem
  necessidade). `iniciarWatchdog()` (um wrapper de uma linha, um só
  chamador) foi inlinado como `wdt_enable(WDTO_8S)`.
- **`Destino.h` removido** — existia só para `motorParaTexto()` poder
  diferenciar `"PAROU!!!"` (Serial) de `"PAROU"` (SD); `andonParaTexto()`
  recebia o mesmo parâmetro e nunca o usava. **Mudança de comportamento:**
  o Serial agora imprime `"PAROU"` como o SD, sem o alerta `!!!`.
  `andonParaTexto()`/`motorParaTexto()` perderam o parâmetro `destino` e
  os dois switches de 3–4 casos viraram tabelas `const char* const[]`.
- **`Display.h`/`Display.cpp`** — as 15 funções de tela quase idênticas
  da Fase 1 (`displayFase1StatusPT100`, `displayFase1StatusHall` etc.)
  foram substituídas por duas genéricas: `displayMsg(linha0, linha1)`
  para telas estáticas e `displayStatus(nome, ok)` para as de OK/ERRO.
  Textos e posições de cursor mantidos idênticos. `lcd` virou `static`
  (só é usado dentro de `Display.cpp`). Os trechos que montavam `String`
  e faziam padding manual por contagem de dígitos (resumo da Fase 1,
  countdown da Fase 2, campo de RPM/vibração no display principal)
  foram trocados por "limpa o campo, depois escreve o valor", sem
  alocação de `String` em heap.
- **`LogicaAvaliacao.cpp`** — `contarErros()` (parâmetro de saída
  `bool &grave`) foi incorporada em `avaliarEstado()`, que agora é a
  única função pública do módulo. As três bandas (temperatura, corrente,
  RPM) passaram a usar uma função auxiliar `faixa()` comum em vez de
  três blocos if/else quase idênticos; resultado numericamente
  equivalente ao original (conferido caso a caso, incluindo o corte
  assimétrico de `tempGrave`/`corrGrave` sem limite inferior).
- **`SensorVibracao`** — `ultimoDebounce1/2` (sempre iguais a
  `ultimaVibr1Ms`/`ultimaVibr2Ms`) foram removidas; o debounce agora
  compara direto contra o timestamp público. `sw420_1_repouso`/
  `sw420_2_repouso` viraram `static` (não eram lidas fora do módulo).
- **`SensorPT100.cpp`** — a média móvel de 5 amostras (buffer + índice
  + flag de preenchimento) foi trocada por um filtro passa-baixa
  exponencial de 1 variável (`temperaturaFiltrada += (t -
  temperaturaFiltrada) * 0.2`). **Mudança de comportamento:** a curva de
  resposta do filtro muda (EMA reage mais rápido a uma mudança brusca e
  nunca "esquece" totalmente uma leitura antiga, diferente da média
  móvel de janela fixa); se a forma exata do filtro importar para o
  TCC, vale revisar.
- **`SensorRPM.cpp`** — o if/else aninhado que decide `EstadoMotor` virou
  uma expressão ternária única.
- Comentários de narração de changelog (`Correção —`, `Passo N`,
  `Melhoria N`) removidos dos arquivos-fonte; esse histórico já vive
  aqui no `CHANGELOG.md`. `CHANGELOG_PASSO9_snippet.md` (rascunho para
  colar no changelog, já coberto pela entrada do Passo 9 abaixo) foi
  removido.

---

## Nova mudança — Centraliza mapeamento enum→texto (Destino.h)
- Confirmado por inspeção: eram 4 blocos `switch` (não 6 — o LCD já
  tinha saído dessa conta no passo anterior), 2 por enum: Serial em
  `BancadaPreditiva.ino` e SD em `LoggerSD.cpp`, um para `EstadoMotor`
  e um para `EstadoAndon` cada.
- Criado `Destino.h`: só o `enum Destino { DESTINO_SERIAL, DESTINO_SD }`,
  sem `.cpp` próprio (não precisa).
- `motorParaTexto(EstadoMotor, Destino)` — declarada em `SensorRPM.h`,
  definida em `SensorRPM.cpp`, ao lado do enum que ela descreve. Único
  ponto de divergência real entre destinos: `MOTOR_PAROU` vira
  `"PAROU!!!"` no Serial (chama atenção de quem acompanha ao vivo) e
  `"PAROU"` no SD (não suja o CSV).
- `andonParaTexto(EstadoAndon, Destino)` — declarada em `Andon.h`,
  definida em `Andon.cpp`. Hoje devolve a mesma palavra pros dois
  destinos; parâmetro `destino` recebido mas não usado no switch
  (`(void)destino`), só pra assinatura já ficar pronta se um dia
  precisar divergir sem quebrar quem chama.
- `BancadaPreditiva.ino` (log Serial) e `LoggerSD.cpp` (CSV) trocaram
  os switches por chamadas às duas funções acima.
- Resultado: uma opção nova em qualquer um dos dois enums agora só
  precisa ser tratada em 1 lugar (a função correspondente), não mais
  em 2 switches espalhados — e esquecer um `case` vira aviso do
  compilador (`-Wswitch` do enum) em vez de bug silencioso.
- Delimitadores (`\t` no Serial, `,` no CSV) continuam por conta de
  quem chama — não fazem parte do "nome" do estado, então ficaram de
  fora das funções.

## Nova mudança — Remove código morto `estadoAtual`/`EstadoSistema`
- Confirmado por inspeção: `estadoAtual` só recebia valor (declaração +
  3 atribuições em `setup()` — `ESTADO_VERIFICANDO`/`ESTADO_AGUARDANDO`/
  `ESTADO_LENDO`) e nunca era lido em nenhum `if`/`switch`/comparação
  nem passado como parâmetro pra nenhuma função, no `.ino` ou em
  qualquer módulo.
- Removidos do `.ino`: o `typedef enum EstadoSistema` e a variável
  `estadoAtual`, junto com as 3 atribuições em `setup()`.
- Nenhuma mudança de comportamento — a variável não influenciava nada
  em runtime.

## Nova mudança — Display 16x2 fixo na Fase 3, sem rodízio
- Hardware trocado: LCD I2C 20x4 → 16x2. `lcd` agora é
  `LiquidCrystal_I2C lcd(0x27, 16, 2)`.
- `atualizarDisplay()` perdeu os parâmetros `estado`/`erros` — não
  cabem mais nas 16 colunas e não fazem parte do que foi definido como
  essencial. Assinatura nova: `atualizarDisplay(temperatura, corrente)`;
  chamada em `loop()` atualizada.
- Tela agora é FIXA (sem rodízio entre telas): os 4 itens essenciais
  ficam sempre visíveis ao mesmo tempo.
  - Linha 0: `T:XX.XC I:X.XXA`
  - Linha 1: `RPM:XXXX V:XXXs` (ou `V:DEF`/`V:GRV` quando há vibração
    detectada naquele ciclo)
- Estado do Andon (BOM/DEFEITO/GRAVE) e contagem de erros saem do
  LCD — quem continua sinalizando isso é só a torre física
  (`setAndon()`, já chamado separadamente no `.ino`).
- As duas faixas de vibração (SW-420 1 = defeito, SW-420 2 = grave)
  foram condensadas num único campo `V:` de 7 colunas: mostra a faixa
  que disparou neste ciclo (`DEF`/`GRV`) ou, em repouso, há quantos
  segundos desde a vibração mais recente entre as duas faixas.
  Simplificação necessária pra caber ao lado do RPM em 16 colunas —
  se precisar separar V1/V2 de novo, sobra espaço tirando o RPM da
  mesma linha.
- **Pendente**: `displayTelaBoot()` e as telas de `verificarPerifericos()`/
  `countdown45s()` (`displayFase1*`/`displayFase2*`) ainda assumem 4
  linhas de 20 colunas cada — vão precisar do mesmo ajuste pro 16x2,
  senão o conteúdo das linhas 2 e 3 simplesmente não aparece no
  hardware novo. Não alterado nesta mudança; fica pro próximo passo.

## Nova mudança — Modelo de decisão IDEAL/RUIM/PERIGOSO (LogicaAvaliacao)
- `contarErros()` reescrita para classificar cada variável em 3 faixas
  nomeadas — **IDEAL** (dentro do setpoint±tolerância), **RUIM** (fora
  do ideal mas dentro do limite grave) e **PERIGOSO** (além do limite
  grave) — no padrão pedido no documento "Programação da Sinaleira",
  em vez do corte binário erro/grave com `return` antecipado.
- Usa exatamente os mesmos números já existentes (`toleranciaTemp`,
  `tempGrave`, `toleranciaCorr`, `corrGrave`, `toleranciaRPM`,
  `rpmGraveMin/Max`) — nenhum setpoint ou tolerância mudou.
- Vibração (sensor digital, sem faixa contínua): `vibr2` já É o nível
  PERIGOSO, `vibr1` é o nível RUIM.
- **Assinatura pública inalterada** (`contarErros`/`avaliarEstado`
  continuam recebendo `LeituraAtual`+`LimitesAvaliacao` e devolvendo
  `EstadoAndon`+`erros`) — nenhum outro módulo ou o `.ino` precisou
  mudar.
- Validado com 2.000.000 de combinações aleatórias de leitura
  comparando a decisão antiga vs. a nova: **0 divergências** no
  estado do Andon (BOM/DEFEITO/GRAVE).
- Única diferença de comportamento: o valor de `erros` registrado no
  Serial/LCD/CSV durante um estado grave deixa de ser sempre fixo em
  `4` e passa a refletir quantas variáveis estão de fato fora do
  ideal (ex.: só vibração grave e resto ok → `erros=1`, não `4`).

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
- **Passo 9** (concluído): telas de boot em `Display`, `.ino` só com
  includes dos módulos.
- **Passo 10**: testes da lógica pura (`LogicaAvaliacao`), se migrar
  para PlatformIO.
