# Bancada Preditiva — Refatoração Modular

Projeto de TCC (Engenharia Mecatrônica): bancada de manutenção preditiva
de motores elétricos, em refatoração de um único `.ino` monolítico para
uma estrutura modular em abas do Arduino IDE.

## Estrutura atual

```
BancadaPreditiva/
├── BancadaPreditiva.ino    // orquestração: setup(), loop(), Fases 1-3, SD
├── Andon.h / .cpp           // torre de sinalização (Passo 2)
├── SensorCorrente.h / .cpp  // ACS712 (Passo 3)
├── SensorPT100.h / .cpp     // PT100 + MAX31865 + média móvel (Passo 4)
├── SensorVibracao.h / .cpp  // 2x SW-420 (Passo 5)
├── SensorRPM.h / .cpp       // Hall + EstadoMotor (Passo 6)
├── LogicaAvaliacao.h / .cpp // contarErros()/avaliarEstado(), sem hardware (Passo 7)
├── CHANGELOG.md
└── README.md
```

## Progresso da modularização

- [x] Passo 1 — Baseline congelada em Git
- [x] Passo 2 — Módulo Andon
- [x] Passo 3 — Módulo SensorCorrente
- [x] Passo 4 — Módulo SensorPT100
- [x] Passo 5 — Módulo SensorVibracao
- [x] Passo 6 — Módulo SensorRPM
- [x] Passo 7 — Módulo LogicaAvaliacao (lógica pura, testável)
- [ ] Passo 8 — Extrair Display e LoggerSD (SD ainda está no `.ino`)
- [ ] Passo 9 — Enxugar `main.ino` (o que sobrar deve ser só orquestração)
- [ ] Passo 10 — Implementar as novas mudanças do projeto

## Como validar cada passo

1. Abra `BancadaPreditiva.ino` no Arduino IDE — os `.h`/`.cpp` aparecem
   como abas ao lado do sketch principal.
2. Compile (Verificar).
3. Se possível, grave na bancada real e confirme que o comportamento é
   idêntico ao da baseline v6.0.
4. `git add -A && git commit -m "Passo 7: extrai módulo LogicaAvaliacao"`

## Por que LogicaAvaliacao importa para o TCC

É o único módulo que não depende de `<Arduino.h>`. Recebe tudo (leituras e
limites) por parâmetro via `LeituraAtual`/`LimitesAvaliacao` e devolve o
estado do Andon — puro cálculo. Isso permite, no futuro, escrever testes
automatizados da regra de negócio sem precisar da bancada ligada, o que é
um ponto forte de rigor de engenharia para o trabalho.
