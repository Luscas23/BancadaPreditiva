# Changelog — Bancada de Manutenção Preditiva de Motores Elétricos

Histórico de versões extraído do cabeçalho do código-fonte (`BancadaPreditiva.ino`)
e convertido para este arquivo a partir da v6.0, congelada como baseline em
13/09/2026 antes do início da refatoração modular.

## [6.0] - Baseline (código monolítico)
### Adicionado
- Gravação das leituras em cartão SD (módulo SPI).
- Arquivo `LOG.CSV`: `tempo_s,temp,corrente,rpm,erros,estadoMotor,estadoAndon` — 1 linha por ciclo.
- Falha no SD não trava a bancada: monitoramento do motor é a função principal;
  gravação é tratada como recurso secundário tolerante a falha.

## [5.0]
### Corrigido
- SW-420: interrupção trocada de `CHANGE` para `RISING` (evitava duplo disparo por vibração).
- `lerCorrente()` reescrita para calcular RMS, correto para corrente alternada
  (a média simples tendia a zero em CA).
- Verificação do sensor Hall corrigida: com `INPUT_PULLUP`, repouso = LOW.
- Snapshots das flags de vibração agora capturados atomicamente e passados
  como parâmetro para `contarErros()` / `avaliarEstado()`, evitando leitura
  inconsistente entre avaliação e exibição no display.

## [4.0]
### Adicionado
- Watchdog Timer (reinicia o Arduino se travar por mais de 8s).
- Média móvel nas leituras do PT100 (últimas 5 leituras).
- Detecção do estado do motor (parado / acelerando / operando / parou).
- Contagem de erros unificada (`contarErros` / `avaliarEstado`).
- Tempo desde a última vibração exibido no display.
- Tratamento mais robusto do sensor SW-420 (detecção do nível de repouso).

---

## Como usar este arquivo daqui para frente

A cada mudança relevante, adicione uma nova seção no topo, no formato:

```
## [Não lançado] ou [x.y] - AAAA-MM-DD
### Adicionado / Corrigido / Alterado / Removido
- Descrição objetiva da mudança e, se fizer sentido, o motivo.
```

Isso substitui o antigo hábito de acumular o histórico dentro do comentário
no topo do `.ino`, que ficava cada vez mais longo e difícil de acompanhar.
