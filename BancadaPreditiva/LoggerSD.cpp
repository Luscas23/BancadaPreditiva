#include <Arduino.h>
#include <SD.h>
#include <avr/wdt.h>
#include "LoggerSD.h"
#include "Configuracao.h"
#include "config.h"
#include "SensorRPM.h"   // rpmAtual, estadoMotor

static unsigned long inicioLeituraMs    = 0;

bool sdDisponivel = false;

// Cartão é recurso secundário. Cada gravação abre, escreve e fecha (flush
// imediato): protege contra perda de dados se faltar energia.
bool loggerSDMontar() {
  return SD.begin(PIN_SD_CS);
}

// Linhas '#': notas ignoradas por pandas (comment='#'). A configuração é anotada
// a cada boot: o log único recebe vários ensaios.
bool loggerSDInit() {
  bool novo = !SD.exists(NOME_ARQ_LOG);
  File arquivo = SD.open(NOME_ARQ_LOG, FILE_WRITE);
  if (!arquivo) return false;
  if (novo) {
    arquivo.println(F("# temp_C = nan indica falha do sensor PT100 (fault do MAX31865) - nao e uma leitura real"));
    arquivo.println(F("tempo_s,temp_C,corrente_A,rpm,erros,estado_motor,estado_andon,freq_vib_hz,vib_1x_g,vib_2x_g,vib_alta_g,vib_razao,vib_calib"));
  }
  arquivo.println(F("# --- boot: configuracao usada ---"));
  configImprimir(arquivo, "# ", false);
  arquivo.close();
  return true;
}

// Lê CONFIG.TXT linha a linha (buffer na pilha). 0 também se o arquivo não existe.
int configCarregarSD(Print &log) {
  File f = SD.open(NOME_ARQ_CFG);
  if (!f) return 0;
  LinhaCfg l = {};
  int avisos = 0;
  while (f.available()) {
    wdt_reset();
    avisos += configAlimentar(l, f.read(), log);
  }
  avisos += configAlimentar(l, '\n', log);   // última linha sem '\n'
  f.close();
  return avisos;
}

void loggerSDIniciarTempo() {
  inicioLeituraMs = millis();
}

void imprimirLinha(Print &p, char sep, float temperatura, float corrente, EstadoAndon estado,
                   int erros, const VibResultado &vib) {
  p.print(temperatura, 2);                p.print(sep);
  p.print(corrente, 3);                   p.print(sep);
  p.print((int)rpmAtual);                 p.print(sep);
  p.print(erros);                         p.print(sep);
  p.print(motorParaTexto(estadoMotor));   p.print(sep);
  p.print(andonParaTexto(estado));        p.print(sep);
  p.print(vib.freqDominante, 1);          p.print(sep);
  p.print(vib.amp1X, 4);                  p.print(sep);
  p.print(vib.amp2X, 4);                  p.print(sep);
  p.print(vib.ampAlta, 4);                p.print(sep);
  p.print(vibRazao, 2);                   p.print(sep);
  p.println((int)vibCalib);
}

void gravarLeituraSD(float temperatura, float corrente, EstadoAndon estado, int erros,
                     const VibResultado &vib) {
  if (!sdDisponivel) return;

  File arquivo = SD.open(NOME_ARQ_LOG, FILE_WRITE);
  if (!arquivo) {
    sdDisponivel = false;  // cartão removido/corrompido: o LCD passa a mostrar '!'
    return;
  }

  arquivo.print((millis() - inicioLeituraMs) / 1000.0, 1); arquivo.print(',');
  imprimirLinha(arquivo, ',', temperatura, corrente, estado, erros, vib);
  arquivo.close();
}
