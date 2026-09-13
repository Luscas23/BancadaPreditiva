# Bancada de Manutenção Preditiva de Motores Elétricos

TCC — Engenharia Mecatrônica — Lucas Altruda Salce

## Hardware
- Microcontrolador: Arduino Mega 2560
- Display: LCD 20x4 I2C
- Sensores: PT100 + MAX31865 | ACS712-5A | KY-003 (Hall/RPM) | 2x SW-420 (vibração)
- Sinalização: Torre Andon (Verde / Amarelo / Vermelho)
- Alimentação: Power Bank 5V/2A 5.000mAh via USB (sem PC)
- Registro: cartão SD (módulo SPI), arquivo `LOG.CSV`

## Status do projeto
A versão `v6.0` (arquivo único `BancadaPreditiva.ino`) está congelada como
**baseline** neste repositório. A partir dela, o código está sendo
reorganizado em módulos (sensores, lógica de decisão, display, Andon,
gravação em SD) para facilitar a manutenção e a adição de novas
funcionalidades sem o risco de regressão que o arquivo único vinha causando.

**Módulos já extraídos (v6.1):**
- ✅ `Andon.h` / `Andon.cpp` — torre de sinalização

**Ainda no `.ino` principal (próximos a extrair):**
- `SensorCorrente`, `SensorPT100`, `SensorVibracao`, `SensorRPM`
- `LogicaAvaliacao` (`contarErros` / `avaliarEstado`)
- `Display` (LCD 20x4)
- `LoggerSD` (gravação em cartão SD)

Acompanhe as mudanças em [CHANGELOG.md](CHANGELOG.md).

## Estrutura do sketch (abas do Arduino IDE)
Cada arquivo `.h`/`.cpp` nesta pasta aparece como uma aba separada quando
você abre `BancadaPreditiva.ino` no Arduino IDE — não precisa fazer nada
além de manter todos os arquivos na mesma pasta do `.ino`.

| Arquivo | Responsabilidade |
|---|---|
| `BancadaPreditiva.ino` | Orquestração: `setup()`, `loop()`, fases, lógica ainda não extraída |
| `Andon.h` / `Andon.cpp` | Torre de sinalização (Verde/Amarelo/Vermelho) |

## Como abrir no Arduino IDE
Esta pasta é um sketch válido do Arduino IDE: o nome da pasta
(`BancadaPreditiva`) é igual ao nome do arquivo principal
(`BancadaPreditiva.ino`), como a IDE exige.

1. Abra o Arduino IDE.
2. `Arquivo > Abrir...` e selecione `BancadaPreditiva.ino` dentro desta pasta
   (as abas `Andon.h` e `Andon.cpp` aparecem automaticamente no topo).
3. Instale as bibliotecas necessárias, se ainda não estiverem instaladas
   (via `Sketch > Incluir Biblioteca > Gerenciar Bibliotecas`):
   - `LiquidCrystal_I2C`
   - `Adafruit_MAX31865`
   - `SD` (já vem com a IDE)
4. Selecione a placa `Arduino Mega 2560` e a porta correta.
5. Compile e grave normalmente — o comportamento deve ser idêntico ao da v6.0.

## Bibliotecas usadas
- `Wire.h`
- `LiquidCrystal_I2C.h`
- `Adafruit_MAX31865.h`
- `avr/wdt.h`
- `SD.h`
