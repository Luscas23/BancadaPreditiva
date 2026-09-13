# Bancada de Manutenção Preditiva de Motores Elétricos

TCC — Engenharia Mecatrônica | Lucas Altruda Salce

Arduino Mega 2560 + LCD 20x4 I2C + PT100/MAX31865 + ACS712-5A +
sensor Hall KY-003 + 2x SW-420 + torre Andon + cartão SD.

## Status da refatoração modular

Reestruturando o projeto de um único `.ino` monolítico para módulos
por responsabilidade, sem alterar o comportamento da bancada.
Progresso: **Passo 8/10 concluído**. Veja `CHANGELOG.md` para o
histórico completo de cada passo.

## Estrutura atual (abas do Arduino IDE)

```
BancadaPreditiva/
├── BancadaPreditiva.ino   — orquestra setup()/loop() e as fases 1-3
├── Andon.h / .cpp         — torre de sinalização
├── SensorCorrente.h / .cpp — leitura RMS do ACS712
├── SensorPT100.h / .cpp   — leitura + média móvel + fault do PT100
├── SensorVibracao.h / .cpp — ISRs do SW-420 + debounce + timestamps
├── SensorRPM.h / .cpp     — ISR Hall + cálculo de RPM + EstadoMotor
├── LogicaAvaliacao.h / .cpp — contarErros()/avaliarEstado(), puro C++
├── Display.h / .cpp       — objeto lcd + atualizarDisplay()
├── LoggerSD.h / .cpp      — gravação em cartão SD (LOG.CSV)
├── CHANGELOG.md
└── README.md
```

## Como validar cada passo

1. Abra `BancadaPreditiva.ino` no Arduino IDE — as abas dos módulos
   aparecem automaticamente ao lado do sketch principal.
2. Compile (Verificar).
3. Grave na bancada real e confirme que o comportamento é idêntico
   ao anterior (torre Andon, LCD, RPM, gravação no SD).
4. `git add -A && git commit -m "Passo N: ..."`.

## Próximos passos

Veja a seção "Próximos passos" no final do `CHANGELOG.md`.
