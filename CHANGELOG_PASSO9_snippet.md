## Passo 9 — Orquestrador fino (telas de boot/Fase 1/Fase 2 movidas para Display)

**Como esta é uma sessão nova**, não tenho acesso ao seu `CHANGELOG.md` e `README.md`
locais (só ao `.ino` no estado do Passo 8 e ao código original v6.0). Cole o bloco
abaixo no seu `CHANGELOG.md` existente, na sequência dos passos anteriores.

### O que mudou
- **Display.h / Display.cpp** — ganharam 15 novas funções, uma para cada tela que
  antes era desenhada diretamente no `.ino` (boot, as 8 sub-etapas da Fase 1 de
  verificação de periféricos, e as 2 telas da Fase 2 de countdown). Texto e posição
  de cursor (`setCursor`) idênticos aos originais — só o "dono" do `lcd.print` mudou.
- **BancadaPreditiva.ino** — `verificarPerifericos()`, `countdown45s()` e `setup()`
  não têm mais nenhuma chamada a `lcd.` diretamente; viraram só orquestração:
  checagem de hardware → chamada a uma função `display...()` → `delay()`.
- **Includes** — removidos `Wire.h`, `LiquidCrystal_I2C.h`, `Adafruit_MAX31865.h` e
  `SD.h` do `.ino`: nenhum deles é usado diretamente aqui, cada um já mora dentro do
  módulo que realmente precisa (`Display`, `SensorPT100`, `LoggerSD`). `avr/wdt.h`
  permanece, pois `wdt_disable()/wdt_enable()/wdt_reset()` continuam chamados
  diretamente no `.ino`.
- Nenhuma regra de negócio ou temporização mudou: mesma ordem de `delay()`, mesmos
  textos, mesmas posições — apenas reposicionamento de código.

### Checagem estática feita
- Confirmado que não sobrou nenhum `lcd.` fora de `Display.cpp` no projeto.
- Toda função `display...()` chamada no `.ino` está declarada em `Display.h` e
  implementada em `Display.cpp` (nenhuma órfã dos dois lados).

### Para integrar no seu repositório local
1. Copie `Display.h` e `Display.cpp`, substituindo os atuais.
2. Substitua `BancadaPreditiva.ino`.
3. Compile no Arduino IDE — deve compilar e se comportar exatamente igual.
4. Grave na bancada real e confirme visualmente que as telas de boot, verificação
   de periféricos e countdown aparecem no LCD exatamente como antes (mesmo texto,
   mesmo alinhamento, mesmo ritmo).
5. `git add -A && git commit -m "Passo 9: .ino vira orquestrador fino - telas movidas para Display, includes limpos"`

### Próximo passo
Com isso, restam só duas coisas do plano original de 10 passos:
- **Passo 10**: escrever os testes de `LogicaAvaliacao` em `test/test_logica_avaliacao.cpp`
  (só faz sentido pra valer se vocês migrarem para PlatformIO — no Arduino IDE dá pra
  fazer uma versão simplificada, um `.ino` de teste à parte que chama `avaliarEstado()`
  com casos fixos e imprime PASS/FAIL no Serial).
- Implementar as **novas mudanças** que motivaram a refatoração desde o início — me
  diga quais são que eu já encaixo no módulo certo.
