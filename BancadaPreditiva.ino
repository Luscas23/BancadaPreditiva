// ================================================================
//   BANCADA DE MANUTENÇÃO PREDITIVA DE MOTORES ELÉTRICOS
//   Microcontrolador : Arduino Mega 2560
//   Display          : LCD 20x4 I2C
//   Sensores         : PT100+MAX31865 | ACS712-5A | KY-003 | 2x SW-420
//   Andon            : Torre de sinalização (Verde / Amarelo / Vermelho)
//   Autor            : Lucas Altruda Salce
//   TCC              : Engenharia Mecatrônica
//   Versão           : 6.0
//   Alimentação      : Power Bank 5V/2A 5.000mAh via USB (sem PC)
//
//   Histórico de melhorias (v4, v5, v6): ver CHANGELOG.md
//   Estrutura modular em andamento: ver CHANGELOG.md / README.md
// ================================================================

// ----------------------------------------------------------------
//  BIBLIOTECAS
// ----------------------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MAX31865.h>
#include <avr/wdt.h>             // Watchdog Timer (melhoria 4)
#include <SD.h>                  // Gravação em cartão SD (v6) — já vem com a IDE Arduino

#include "Andon.h"                // Módulo extraído — passo 2
#include "SensorCorrente.h"       // Módulo extraído — passo 3

// ----------------------------------------------------------------
//  PINOS — Arduino Mega 2560
// ----------------------------------------------------------------
#define PIN_SW420_1        2     // Vibração faixa 1  (INT0 — defeito)
#define PIN_SW420_2        3     // Vibração faixa 2  (INT1 — grave)
#define PIN_HALL           18    // RPM Hall KY-003   (INT5)
// PIN_ACS712 agora vive em SensorCorrente.h

#define PIN_MAX31865_CS    10    // PT100 CS (SPI: SCK=52 MISO=50 MOSI=51)
#define PIN_SD_CS          4     // Cartão SD CS — mesmo barramento SPI do PT100 (v6)

// Pinos do Andon agora vivem em Andon.h

// ----------------------------------------------------------------
//  PT100 / MAX31865
// ----------------------------------------------------------------
Adafruit_MAX31865 pt100 = Adafruit_MAX31865(PIN_MAX31865_CS);
#define PT100_RNOM         100.0
#define PT100_RREF         430.0

// Melhoria 5 — Média móvel temperatura (últimas 5 leituras)
#define MEDIA_MOVEL_N      5
float bufferTemp[MEDIA_MOVEL_N] = {0};
int   indexTemp                 = 0;
bool  bufferPreenchido          = false;

// ----------------------------------------------------------------
//  ACS712-5A — defines e lerCorrente() agora em SensorCorrente.h/.cpp
// ----------------------------------------------------------------

// ----------------------------------------------------------------
//  RPM
// ----------------------------------------------------------------
#define PULSOS_POR_VOLTA   1
#define INTERVALO_RPM_MS   1000

volatile unsigned long pulsos    = 0;
unsigned long ultimoCalculoRPM   = 0;
float rpmAtual                   = 0.0;

// Melhoria 6 — Estado do motor
typedef enum {
  MOTOR_PARADO,        // RPM = 0 desde o início
  MOTOR_ACELERANDO,    // RPM > 0 mas ainda abaixo do setpoint
  MOTOR_OPERANDO,      // RPM dentro da faixa nominal
  MOTOR_PAROU          // RPM era > 0 e voltou a 0 (parada inesperada)
} EstadoMotor;

EstadoMotor estadoMotor     = MOTOR_PARADO;
bool        motorJaGirou    = false;

// ----------------------------------------------------------------
//  VIBRAÇÃO
// ----------------------------------------------------------------
volatile bool vibr1         = false;
volatile bool vibr2         = false;
#define DEBOUNCE_MS          50
unsigned long ultimoDebounce1 = 0;
unsigned long ultimoDebounce2 = 0;

// Melhoria 7 — Tempo desde última vibração
unsigned long ultimaVibr1Ms = 0;
unsigned long ultimaVibr2Ms = 0;

// ----------------------------------------------------------------
//  SETPOINTS E TOLERÂNCIAS
// ----------------------------------------------------------------
float setpointTemp    = 60.0;
float toleranciaTemp  = 20.0;
#define TEMP_GRAVE     100.0

// Motor 127V: setpointCorr=2.9, toleranciaCorr=0.4, CORR_GRAVE=4.0
// Motor 220V: setpointCorr=1.4, toleranciaCorr=0.2, CORR_GRAVE=2.0
float setpointCorr    = 2.9;
float toleranciaCorr  = 0.4;
#define CORR_GRAVE     4.0

float setpointRPM     = 1740.0;
float toleranciaRPM   = 140.0;
#define RPM_GRAVE_MIN  1400.0
#define RPM_GRAVE_MAX  2000.0

// ----------------------------------------------------------------
//  ESTADOS DO SISTEMA
// ----------------------------------------------------------------
typedef enum {
  ESTADO_INIT,
  ESTADO_VERIFICANDO,
  ESTADO_AGUARDANDO,
  ESTADO_LENDO
} EstadoSistema;

EstadoSistema estadoAtual = ESTADO_INIT;

// ----------------------------------------------------------------
//  LCD 20x4 I2C
// ----------------------------------------------------------------
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ----------------------------------------------------------------
//  VARIÁVEIS DE LEITURA
// ----------------------------------------------------------------
float temperatura  = 0.0;
float corrente     = 0.0;
bool  erroSensor   = false;

// ----------------------------------------------------------------
//  GRAVAÇÃO SD (v6)
// ----------------------------------------------------------------
const char NOME_ARQUIVO_LOG[] = "LOG.CSV";
bool sdDisponivel        = false;  // false = cartão ausente/falhou, bancada continua sem gravar
unsigned long inicioLeituraMs = 0; // referência de tempo: zerada quando a Fase 3 começa

bool okDisplay  = false;
bool okPT100    = false;
bool okACS712   = false;
bool okHall     = false;
bool okSW420_1  = false;
bool okSW420_2  = false;
bool okAndon    = false;
bool okSD       = false;   // v6 — informativo; falha aqui NÃO bloqueia o funcionamento da bancada

// Estado lógico do SW-420 em repouso (detectado na verificação)
// Melhoria 3 — adaptativo conforme modelo do sensor
int sw420_1_repouso = LOW;
int sw420_2_repouso = LOW;

// ================================================================
//  INTERRUPÇÕES
// ================================================================
void ISR_hall() {
  pulsos++;
}

void ISR_vibr1() {
  unsigned long agora = millis();
  if (agora - ultimoDebounce1 > DEBOUNCE_MS) {
    vibr1           = true;
    ultimaVibr1Ms   = agora;   // Melhoria 7
    ultimoDebounce1 = agora;
  }
}

void ISR_vibr2() {
  unsigned long agora = millis();
  if (agora - ultimoDebounce2 > DEBOUNCE_MS) {
    vibr2           = true;
    ultimaVibr2Ms   = agora;   // Melhoria 7
    ultimoDebounce2 = agora;
  }
}

// ================================================================
//  FUNÇÕES AUXILIARES
// ================================================================

// setAndon() e piscarAndon() agora em Andon.h/.cpp

// ----------------------------------------------------------------
//  MELHORIA 2+1 — Contagem de erros unificada
//  Recebe snapshots das flags de vibração capturados atomicamente
//  no loop() para evitar leitura inconsistente entre avaliação e display
//  Nota: risco residual baixo (ciclo de 500 ms), mas documentado no TCC
// ----------------------------------------------------------------
int contarErros(bool &grave, bool snap_vibr1, bool snap_vibr2) {
  grave    = false;
  int erros = 0;

  // Vibração grave — nível crítico imediato
  if (snap_vibr2) { grave = true; return 4; }

  // Temperatura
  if (temperatura > TEMP_GRAVE) { grave = true; return 4; }
  float minTemp = setpointTemp - toleranciaTemp;
  float maxTemp = setpointTemp + toleranciaTemp;
  if (temperatura < minTemp || temperatura > maxTemp) erros++;

  // Corrente
  if (corrente > CORR_GRAVE) { grave = true; return 4; }
  float minCorr = setpointCorr - toleranciaCorr;
  float maxCorr = setpointCorr + toleranciaCorr;
  if (corrente < minCorr || corrente > maxCorr) erros++;

  // RPM — só avalia se motor já girou (melhoria 6)
  if (motorJaGirou && rpmAtual > 0) {
    if (rpmAtual < RPM_GRAVE_MIN || rpmAtual > RPM_GRAVE_MAX) {
      grave = true; return 4;
    }
    float minRPM = setpointRPM - toleranciaRPM;
    float maxRPM = setpointRPM + toleranciaRPM;
    if (rpmAtual < minRPM || rpmAtual > maxRPM) erros++;
  }

  // Vibração faixa 1
  if (snap_vibr1) erros++;

  return erros;
}

// ----------------------------------------------------------------
//  Avalia estado do Andon usando contagem unificada
// ----------------------------------------------------------------
EstadoAndon avaliarEstado(int &erros, bool snap_vibr1, bool snap_vibr2) {
  bool grave = false;
  erros = contarErros(grave, snap_vibr1, snap_vibr2);

  if (grave || erros >= 3) return ANDON_GRAVE;
  if (erros >= 1)          return ANDON_DEFEITO;
  return ANDON_BOM;
}

// ----------------------------------------------------------------
//  MELHORIA 4 — Watchdog: reinicia o Arduino se travar por >8s
// ----------------------------------------------------------------
void iniciarWatchdog() {
  wdt_enable(WDTO_8S);
}

// ----------------------------------------------------------------
//  V6 — GRAVAÇÃO EM CARTÃO SD
//  Tratada como recurso secundário: se o cartão falhar ou for
//  removido, a bancada continua monitorando o motor normalmente.
//  Cada chamada abre, escreve e fecha o arquivo (flush imediato) —
//  protege contra perda de dados se a bancada perder energia
//  (alimentação agora vem do power bank, não de fonte controlada).
// ----------------------------------------------------------------
bool iniciarSD() {
  pinMode(53, OUTPUT);  // Mantém o SS de hardware do Mega como saída

  if (!SD.begin(PIN_SD_CS)) {
    return false;
  }

  // Cria o arquivo com cabeçalho apenas se ainda não existir
  if (!SD.exists(NOME_ARQUIVO_LOG)) {
    File arquivo = SD.open(NOME_ARQUIVO_LOG, FILE_WRITE);
    if (!arquivo) return false;
    arquivo.println(F("tempo_s,temp_C,corrente_A,rpm,erros,estado_motor,estado_andon"));
    arquivo.close();
  }
  return true;
}

void gravarLeituraSD(EstadoAndon estado, int erros) {
  if (!sdDisponivel) return;  // recurso desabilitado — não tenta nem trava o ciclo

  File arquivo = SD.open(NOME_ARQUIVO_LOG, FILE_WRITE);
  if (!arquivo) {
    sdDisponivel = false;  // cartão removido/corrompido — desliga gravação, bancada continua
    return;
  }

  arquivo.print((millis() - inicioLeituraMs) / 1000.0, 1); arquivo.print(F(","));
  arquivo.print(temperatura, 2);                            arquivo.print(F(","));
  arquivo.print(corrente, 3);                               arquivo.print(F(","));
  arquivo.print((int)rpmAtual);                             arquivo.print(F(","));
  arquivo.print(erros);                                     arquivo.print(F(","));
  switch (estadoMotor) {
    case MOTOR_PARADO:     arquivo.print(F("PARADO"));     break;
    case MOTOR_ACELERANDO: arquivo.print(F("ACELERANDO")); break;
    case MOTOR_OPERANDO:   arquivo.print(F("OPERANDO"));   break;
    case MOTOR_PAROU:      arquivo.print(F("PAROU"));      break;
  }
  arquivo.print(F(","));
  switch (estado) {
    case ANDON_BOM:     arquivo.println(F("BOM"));     break;
    case ANDON_DEFEITO: arquivo.println(F("DEFEITO")); break;
    case ANDON_GRAVE:   arquivo.println(F("GRAVE"));   break;
  }

  arquivo.close();  // fecha = grava (flush) no cartão imediatamente
}

// ----------------------------------------------------------------
//  MELHORIA 5 — Média móvel temperatura
// ----------------------------------------------------------------
float mediaMovelTemp(float novaLeitura) {
  bufferTemp[indexTemp] = novaLeitura;
  indexTemp = (indexTemp + 1) % MEDIA_MOVEL_N;
  if (indexTemp == 0) bufferPreenchido = true;

  int n    = bufferPreenchido ? MEDIA_MOVEL_N : indexTemp;
  float soma = 0;
  for (int i = 0; i < n; i++) soma += bufferTemp[i];
  return soma / n;
}

// ----------------------------------------------------------------
//  LEITURA DE TEMPERATURA PT100
// ----------------------------------------------------------------
float lerTemperatura() {
  float t       = pt100.temperature(PT100_RNOM, PT100_RREF);
  uint8_t fault = pt100.readFault();
  if (fault) {
    pt100.clearFault();
    erroSensor = true;
    return -999.0;
  }
  erroSensor = false;
  return mediaMovelTemp(t);   // Melhoria 5
}

// lerCorrente() agora em SensorCorrente.h/.cpp (leitura RMS)

// ----------------------------------------------------------------
//  CÁLCULO DE RPM + Melhoria 6 (estado do motor)
// ----------------------------------------------------------------
void calcularRPM() {
  unsigned long agora    = millis();
  unsigned long intervalo = agora - ultimoCalculoRPM;
  if (intervalo >= INTERVALO_RPM_MS) {
    noInterrupts();
    unsigned long p = pulsos;
    pulsos = 0;
    interrupts();

    float novoRPM = (p / (float)PULSOS_POR_VOLTA) * (60000.0 / intervalo);

    // Melhoria 6 — detecta estado do motor
    if (novoRPM > 0) {
      motorJaGirou = true;
      float minRPM = setpointRPM - toleranciaRPM;
      float maxRPM = setpointRPM + toleranciaRPM;
      if (novoRPM >= minRPM && novoRPM <= maxRPM) estadoMotor = MOTOR_OPERANDO;
      else                                          estadoMotor = MOTOR_ACELERANDO;
    } else {
      if (motorJaGirou) estadoMotor = MOTOR_PAROU;  // parada inesperada
      else              estadoMotor = MOTOR_PARADO;
    }

    rpmAtual         = novoRPM;
    ultimoCalculoRPM = agora;
  }
}

// ================================================================
//  FASE 1 — VERIFICAÇÃO DE PERIFÉRICOS
// ================================================================
void verificarPerifericos() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));
  lcd.setCursor(0, 1); lcd.print(F("Aguarde...          "));
  delay(1000);

  // Display
  okDisplay = true;
  lcd.setCursor(0, 1); lcd.print(F("Display.........OK  "));
  delay(500);

  // PT100
  float testTemp = pt100.temperature(PT100_RNOM, PT100_RREF);
  uint8_t fault  = pt100.readFault();
  okPT100 = (fault == 0 && testTemp > -200.0 && testTemp < 500.0);
  pt100.clearFault();
  lcd.setCursor(0, 2); lcd.print(F("PT100..........."));
  lcd.print(okPT100 ? F("OK  ") : F("ERRO"));
  delay(500);

  // ACS712 — verificação agora encapsulada em SensorCorrente
  okACS712 = verificarACS712();
  lcd.setCursor(0, 3); lcd.print(F("ACS712.........."));
  lcd.print(okACS712 ? F("OK  ") : F("ERRO"));
  delay(800);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));

  // Hall
  okHall = (digitalRead(PIN_HALL) == LOW); // Corrigido: com INPUT_PULLUP, repouso é LOW (HIGH = ausente ou ímã passando)
  lcd.setCursor(0, 1); lcd.print(F("Sensor Hall....."));
  lcd.print(okHall ? F("OK  ") : F("ERRO"));
  delay(500);

  // Melhoria 3 — SW-420: detecta estado de repouso do sensor
  sw420_1_repouso = digitalRead(PIN_SW420_1);
  sw420_2_repouso = digitalRead(PIN_SW420_2);
  okSW420_1 = true;  // Sensor presente e lido com sucesso
  okSW420_2 = true;
  lcd.setCursor(0, 2); lcd.print(F("Vibr. SW1.......OK  "));
  delay(500);
  lcd.setCursor(0, 3); lcd.print(F("Vibr. SW2.......OK  "));
  delay(500);

  // Andon
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Verificando sistema "));
  lcd.setCursor(0, 1); lcd.print(F("Andon torre.....    "));
  setAndon(ANDON_GRAVE);   delay(300);
  setAndon(ANDON_DEFEITO); delay(300);
  setAndon(ANDON_BOM);     delay(300);
  setAndon(ANDON_GRAVE);
  okAndon = true;
  lcd.setCursor(16, 1); lcd.print(F("OK  "));
  delay(500);

  // V6 — Cartão SD (informativo: falha aqui não impede o resumo final)
  okSD = iniciarSD();
  sdDisponivel = okSD;
  lcd.setCursor(0, 2); lcd.print(F("Cartao SD........"));
  lcd.print(okSD ? F("OK  ") : F("ERRO"));
  delay(500);

  // Resumo
  bool tudo_ok = okDisplay && okPT100 && okACS712 &&
                 okHall && okSW420_1 && okSW420_2 && okAndon;

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("=== RESULTADO ===   "));
  lcd.setCursor(0, 1); lcd.print(tudo_ok ? F("Todos OK!           ")
                                         : F("Erros detectados!   "));
  if (!tudo_ok) {
    lcd.setCursor(0, 2);
    String falhas = "";
    if (!okPT100)  falhas += "PT100 ";
    if (!okACS712) falhas += "ACS ";
    if (!okHall)   falhas += "HALL ";
    falhas += "                ";
    lcd.print(falhas.substring(0, 20));
  }
  lcd.setCursor(0, 3); lcd.print(F("Iniciando em 45s... "));

  if (tudo_ok) setAndon(ANDON_BOM);
  else { piscarAndon(ANDON_DEFEITO, 3, 300); setAndon(ANDON_DEFEITO); }
}

// ================================================================
//  FASE 2 — COUNTDOWN 45 SEGUNDOS
// ================================================================
void countdown45s() {
  unsigned long inicio  = millis();
  unsigned long duracao = 45000UL;

  while (millis() - inicio < duracao) {
    wdt_reset();  // Melhoria 4 — alimenta watchdog durante countdown
    unsigned long restante = (duracao - (millis() - inicio)) / 1000;
    lcd.setCursor(0, 3);
    lcd.print(F("Aguardando: "));
    if (restante < 10) lcd.print(F(" "));
    lcd.print(restante);
    lcd.print(F("s   "));
    delay(500);
  }

  lcd.clear();
  lcd.setCursor(2, 1); lcd.print(F("INICIANDO LEITURA"));
  lcd.setCursor(4, 2); lcd.print(F("DO MOTOR..."));
  piscarAndon(ANDON_BOM, 3, 200);
  setAndon(ANDON_BOM);
  delay(1500);
  lcd.clear();
}

// ================================================================
//  FASE 3 — DISPLAY 20x4
//
//  Linha 0: BANCADA PREDITIVA [BOM/DEF/GRV]
//  Linha 1: T: XX.XC   I: X.XXA
//  Linha 2: RPM:XXXX  E:X [estado motor]
//  Linha 3: VIB1:XXs  VIB2:XXs
// ================================================================
void atualizarDisplay(EstadoAndon estado, int erros) {

  // Linha 0 — título + estado
  lcd.setCursor(0, 0);
  lcd.print(F("BANCADA PREDITIVA   "));
  lcd.setCursor(17, 0);
  switch (estado) {
    case ANDON_BOM:    lcd.print(F("BOM")); break;
    case ANDON_DEFEITO:lcd.print(F("DEF")); break;
    case ANDON_GRAVE:  lcd.print(F("GRV")); break;
  }

  // Linha 1 — Temperatura e Corrente
  lcd.setCursor(0, 1);
  lcd.print(F("T:"));
  if (erroSensor) {
    lcd.print(F("ERRO  "));
  } else {
    if (temperatura >= 0.0 && temperatura < 100.0) lcd.print(F(" "));
    lcd.print(temperatura, 1);
    lcd.print(F("C "));
  }
  lcd.setCursor(10, 1);
  lcd.print(F("I:"));
  lcd.print(corrente, 2);
  lcd.print(F("A  "));

  // Linha 2 — RPM + erros + estado motor (melhoria 6)
  lcd.setCursor(0, 2);
  lcd.print(F("RPM:"));
  lcd.print((int)rpmAtual);
  lcd.print(F("  "));
  lcd.setCursor(10, 2);
  lcd.print(F("E:"));
  lcd.print(erros);
  lcd.print(F(" "));
  switch (estadoMotor) {
    case MOTOR_PARADO:     lcd.print(F("PAR")); break;
    case MOTOR_ACELERANDO: lcd.print(F("ACE")); break;
    case MOTOR_OPERANDO:   lcd.print(F("OPE")); break;
    case MOTOR_PAROU:      lcd.print(F("!!!"));  break;
  }

  // Linha 3 — Melhoria 7: tempo desde última vibração
  // Melhoria 1 corrigida: flags lidas ANTES de serem zeradas
  unsigned long agora = millis();
  lcd.setCursor(0, 3);

  if (vibr1 || vibr2) {
    // Vibração ativa neste ciclo
    lcd.print(vibr2 ? F("VIB:GRAVE           ")
                    : F("VIB:DEFEITO         "));
  } else {
    // Mostra há quantos segundos foi a última vibração
    lcd.print(F("V1:"));
    if (ultimaVibr1Ms == 0) {
      lcd.print(F("---s "));
    } else {
      unsigned long s1 = (agora - ultimaVibr1Ms) / 1000;
      if (s1 > 999) s1 = 999;
      if (s1 < 10)  lcd.print(F("  "));
      else if (s1 < 100) lcd.print(F(" "));
      lcd.print(s1);
      lcd.print(F("s "));
    }
    lcd.setCursor(10, 3);
    lcd.print(F("V2:"));
    if (ultimaVibr2Ms == 0) {
      lcd.print(F("---s "));
    } else {
      unsigned long s2 = (agora - ultimaVibr2Ms) / 1000;
      if (s2 > 999) s2 = 999;
      if (s2 < 10)  lcd.print(F("  "));
      else if (s2 < 100) lcd.print(F(" "));
      lcd.print(s2);
      lcd.print(F("s "));
    }
  }

  // Melhoria 1 corrigida — limpa flags APÓS avaliação e exibição
  vibr1 = false;
  vibr2 = false;
}

// ================================================================
//  SETUP
// ================================================================
void setup() {
  // Melhoria 4 — desabilita watchdog residual de reset anterior
  wdt_disable();

  Serial.begin(9600);
  Serial.println(F("=== BANCADA PREDITIVA v6.0 ==="));

  // Saídas
  andonInit();          // Módulo Andon — passo 2
  setAndon(ANDON_GRAVE);

  // Entradas
  pinMode(PIN_SW420_1, INPUT);
  pinMode(PIN_SW420_2, INPUT);
  pinMode(PIN_HALL,    INPUT_PULLUP);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print(F("  BANCADA PREDITIVA "));
  lcd.setCursor(0, 1); lcd.print(F("  MOTORES ELETRICOS "));
  lcd.setCursor(0, 2); lcd.print(F("   TCC - ENGENHARIA "));
  lcd.setCursor(0, 3); lcd.print(F("   Iniciando...     "));
  delay(2000);

  // PT100
  pt100.begin(MAX31865_2WIRE);

  // Interrupções
  attachInterrupt(digitalPinToInterrupt(PIN_HALL),    ISR_hall,  RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_SW420_1), ISR_vibr1, RISING); // Corrigido: CHANGE causava duplo disparo por vibração
  attachInterrupt(digitalPinToInterrupt(PIN_SW420_2), ISR_vibr2, RISING); // Corrigido: CHANGE causava duplo disparo por vibração
  ultimoCalculoRPM = millis();

  // Fases de inicialização
  estadoAtual = ESTADO_VERIFICANDO;
  verificarPerifericos();

  estadoAtual = ESTADO_AGUARDANDO;
  countdown45s();

  estadoAtual = ESTADO_LENDO;
  inicioLeituraMs = millis();  // v6 — referência t=0 do CSV: começo da Fase 3, não do boot

  // Melhoria 4 — ativa watchdog apenas após inicialização completa
  iniciarWatchdog();

  Serial.println(F("Temp(C)\tCorr(A)\tRPM\tErros\tMotor\tEstado"));
  if (sdDisponivel) Serial.println(F("Gravando em LOG.CSV no cartao SD"));
  else              Serial.println(F("Cartao SD indisponivel - gravacao desabilitada"));
}

// ================================================================
//  LOOP PRINCIPAL
// ================================================================
void loop() {

  wdt_reset();  // Melhoria 4 — alimenta watchdog a cada ciclo

  // Leituras
  temperatura = lerTemperatura();
  corrente    = lerCorrente();     // agora em SensorCorrente.cpp
  calcularRPM();

  // Captura estado das flags atomicamente ANTES de qualquer avaliação
  // Corrigido (❻): snapshots agora são passados para contarErros/avaliarEstado
  noInterrupts();
  bool vibr1_snapshot = vibr1;
  bool vibr2_snapshot = vibr2;
  interrupts();

  // Avaliação unificada (melhoria 2) — usa snapshots, não as flags voláteis
  int erros = 0;
  EstadoAndon estado = avaliarEstado(erros, vibr1_snapshot, vibr2_snapshot);

  // Atualiza Andon
  setAndon(estado);

  // Atualiza display (flags zeradas dentro, após exibição — melhoria 1)
  atualizarDisplay(estado, erros);

  // Log Serial
  Serial.print(temperatura, 2); Serial.print(F("\t"));
  Serial.print(corrente, 3);    Serial.print(F("\t"));
  Serial.print((int)rpmAtual);  Serial.print(F("\t"));
  Serial.print(erros);          Serial.print(F("\t"));
  switch (estadoMotor) {
    case MOTOR_PARADO:     Serial.print(F("PARADO\t"));     break;
    case MOTOR_ACELERANDO: Serial.print(F("ACELERANDO\t")); break;
    case MOTOR_OPERANDO:   Serial.print(F("OPERANDO\t"));   break;
    case MOTOR_PAROU:      Serial.print(F("PAROU!!!\t"));   break;
  }
  switch (estado) {
    case ANDON_BOM:    Serial.println(F("BOM"));     break;
    case ANDON_DEFEITO:Serial.println(F("DEFEITO"));  break;
    case ANDON_GRAVE:  Serial.println(F("GRAVE"));   break;
  }

  // V6 — Grava a mesma leitura no cartão SD (se disponível)
  gravarLeituraSD(estado, erros);

  delay(500);
}
