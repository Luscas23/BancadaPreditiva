# Bancada de Manutenção Preditiva de Motores Elétricos

TCC — Engenharia Mecatrônica | Lucas Altruda Salce

Arduino Mega 2560 + LCD 16x2 I2C + PT100/MAX31865 + ACS712-5A +
sensor óptico TCRT5000 no eixo do motor, 1 marca refletiva, RPM medido pelo período entre pulsos (pino 18) + acelerômetro ADXL345 (vibração,
I2C 0x53, SDA=20/SCL=21) + torre Andon + cartão SD.

Bibliotecas: `arduinoFFT` 2.x, `LiquidCrystal_I2C`, `Adafruit_MAX31865`,
`SD`. Comando Serial `c` apaga a baseline de vibração e recalibra.

## Status da refatoração modular

Reestruturando o projeto de um único `.ino` monolítico para módulos
por responsabilidade, sem alterar o comportamento da bancada.
Progresso: **Passo 10/10 concluído** (testes da lógica pura em `teste/`).
Veja `CHANGELOG.md` para o estado atual.

## Estrutura atual (abas do Arduino IDE)

```
BancadaPreditiva/
├── BancadaPreditiva.ino   — orquestra setup()/loop() e as fases 1-3
├── Andon.h / .cpp         — torre de sinalização
├── SensorCorrente.h / .cpp — leitura RMS do ACS712
├── SensorPT100.h / .cpp   — leitura + fault do PT100 (3 fios)
├── SensorVibracao.h / .cpp — ADXL345 (I2C) + FFT por janela + baseline na EEPROM
├── SensorRPM.h / .cpp     — ISR TCRT5000 + cálculo de RPM + EstadoMotor
├── LogicaAvaliacao.h / .cpp — avaliarEstado(), puro C++
├── Display.h / .cpp       — objeto lcd + atualizarDisplay()
├── LoggerSD.h / .cpp      — gravação em cartão SD (LOG.CSV) e leitura do CONFIG.TXT
├── config.h               — padrões de fábrica e constantes de compilação
├── Configuracao.h / .cpp  — struct cfg, tabela de chaves, parser, validação
├── exemplo/CONFIG.TXT     — exemplo completo (gerado da tabela)
├── docs/CONFIG.md         — tabela de chaves (gerada da tabela)
├── teste/                 — teste_logica.cpp (roda no PC, sem a bancada)
├── CHANGELOG.md
└── README.md
```

## Configuração

Duas camadas:

1. **`config.h` (compilação):** fonte única dos padrões (`PAD_*`) e das constantes
   que definem buffers e hardware. Sem `CONFIG.TXT`, a bancada se comporta como
   sempre.
2. **`CONFIG.TXT` na raiz do cartão SD (execução, opcional):** sobrescreve os
   `PAD_*` no boot, sem recompilar. Uma chave por linha (`chave = valor`, `#`
   comenta). Copie `exemplo/CONFIG.TXT` e apague as linhas que não quer mudar.

Chave desconhecida, valor inválido ou fora da faixa é ignorado e avisado na
Serial; o resto carrega. Depois, a validação cruzada (pinos distintos e com
interrupção no TCRT, `ruim < grave`, 1X dentro da banda da FFT...) devolve ao
padrão o **grupo inteiro** incoerente. Com avisos o LCD mostra `CFG: erro N`
por 2 s. A configuração usada vai à Serial e, a cada boot, ao `LOG.CSV` como
linhas `#` (use `comment='#'` no pandas).

- **Exige recompilar:** `ADXL_FS`, `ADXL_N`, `ADXL_BW_RATE`, CS do SD, baud, nome
  dos arquivos, timeout do watchdog, tamanho do LCD, tempos de mensagem, `RPM_AMOSTRAS` (média móvel do RPM, potência de 2, padrão 4).
- **Não exige:** tudo em `docs/CONFIG.md` (chave, unidade, padrão, intervalo,
  descrição e se afeta a baseline da vibração).
- Trocou motor? Informe também `rpm_*` e `corr_*`: os limites são explícitos,
  não derivados do nominal. Mudou RPM nominal, redução ou bandas da FFT: a
  baseline da EEPROM deixa de valer (assinatura) e é recalibrada sozinha.
- `exemplo/CONFIG.TXT` e `docs/CONFIG.md` saem da tabela:
  `g++ -I teste -I BancadaPreditiva teste/teste_config.cpp BancadaPreditiva/Configuracao.cpp BancadaPreditiva/LogicaAvaliacao.cpp -o /tmp/tc && /tmp/tc --gerar`
- Teste do cálculo de RPM: `g++ -I teste -I BancadaPreditiva teste/teste_rpm.cpp -o /tmp/teste_rpm && /tmp/teste_rpm`

## Ligação elétrica

Cuidados vindos do cabeçalho do código antigo:

- **Conversor de nível 3,3 V↔5 V no I2C do ADXL345**: o ADXL345 opera em 3,3 V; o Arduino Mega usa 5 V no barramento I2C. Um conversor bidirecional é obrigatório para evitar danos ao sensor.
- **Jumper JD-VCC dos relés**: o módulo de relés geralmente tem um jumper JD-VCC que isola a alimentação dos relés da alimentação do optoacoplador. Para isolamento correto, remova o jumper e alimente o pino JD-VCC com uma fonte externa de 5 V (comum com o GND do Arduino).
- **Ligar o Arduino com o motor desligado**: antes de energizar o Arduino, certifique-se de que o motor está desligado. Isso evita transientes na partida e leituras incorretas nos sensores durante o boot.

## Calibração da vibração

1. Calibre só com o motor **saudável**, na montagem final e com o sensor fixo.
2. Aguarde o `C:` sair do display (ou a mensagem na Serial).
3. Refaça a calibração com o comando `c` sempre que mexer em fixação, posição do sensor, base ou acoplamento.

## Como validar cada passo

1. Abra `BancadaPreditiva.ino` no Arduino IDE — as abas dos módulos
   aparecem automaticamente ao lado do sketch principal.
2. Compile (Verificar).
3. Grave na bancada real e confirme que o comportamento é idêntico
   ao anterior (torre Andon, LCD, RPM, gravação no SD).
4. `git add -A && git commit -m "Passo N: ..."`.
