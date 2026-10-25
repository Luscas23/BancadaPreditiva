# Estado atual (v6.0)

- Sensores: PT100 3 fios (MAX31865, 430 ohm), ACS712-5A (RMS, offset da própria janela), TCRT5000 no eixo de saída da redução 1:26 (RPM do motor = período entre pulsos x 26), ADXL345 por I2C (FFT 1X/2X/ALTA contra baseline na EEPROM).
- Avaliação: temperatura e corrente só com limite superior (`#define`s em `LogicaAvaliacao.h`); RPM e vibração por faixa. 3 estados + torre apagada com motor parado (corrente < 0,3 A).
- Carência de 5 s após a partida: corrente e RPM não são avaliados. Corrente > 0,3 A com RPM 0 por mais de 3 s = RUIM.
- Torre Andon com relé ativo em LOW, apagada no boot; histerese: piora na hora, melhora após 3 ciclos.
- Saída: LCD 16x2 I2C (0x27; `!` na coluna 8 da linha 2 = SD indisponível), Serial 115200 e `LOG.CSV` no SD (só grava com janela de vibração válida; `nan` em temp_C = falha do PT100).
- Comando Serial `c` apaga a baseline de vibração e recalibra. Testes da lógica: `teste/teste_logica.cpp`.
- Histórico anterior (Hall, SW-420, LCD 20x4, passos 1-9 da refatoração) está no git.
