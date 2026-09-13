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
**baseline** neste repositório. A partir daqui, o código está sendo
reorganizado em módulos (sensores, lógica de decisão, display, Andon,
gravação em SD) para facilitar a manutenção e a adição de novas
funcionalidades sem o risco de regressão que o arquivo único vinha causando.

Acompanhe as mudanças em [CHANGELOG.md](CHANGELOG.md).

## Como abrir no Arduino IDE
Esta pasta é um sketch válido do Arduino IDE: o nome da pasta
(`BancadaPreditiva`) é igual ao nome do arquivo principal
(`BancadaPreditiva.ino`), como a IDE exige.

1. Abra o Arduino IDE.
2. `Arquivo > Abrir...` e selecione `BancadaPreditiva.ino` dentro desta pasta.
3. Instale as bibliotecas necessárias, se ainda não estiverem instaladas
   (via `Sketch > Incluir Biblioteca > Gerenciar Bibliotecas`):
   - `LiquidCrystal_I2C`
   - `Adafruit_MAX31865`
   - `SD` (já vem com a IDE)
4. Selecione a placa `Arduino Mega 2560` e a porta correta.
5. Compile e grave normalmente.

## Bibliotecas usadas
- `Wire.h`
- `LiquidCrystal_I2C.h`
- `Adafruit_MAX31865.h`
- `avr/wdt.h`
- `SD.h`
