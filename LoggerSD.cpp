#include <Arduino.h>
#include <SD.h>
#include "LoggerSD.h"
#include "SensorRPM.h"   // rpmAtual, estadoMotor, EstadoMotor
#include "Destino.h"     // DESTINO_SD, usado no log do cartão

static const char    NOME_ARQUIVO_LOG[] = "LOG.CSV";
static unsigned long inicioLeituraMs    = 0;

bool sdDisponivel = false;

// ================================================================
//  GRAVAÇÃO EM CARTÃO SD
//  Tratada como recurso secundário: se o cartão falhar ou for
//  removido, a bancada continua monitorando o motor normalmente.
//  Cada chamada abre, escreve e fecha o arquivo (flush imediato) —
//  protege contra perda de dados se a bancada perder energia.
// ================================================================
bool loggerSDInit() {
  pinMode(53, OUTPUT);  // Mantém o SS de hardware do Mega como saída

  if (!SD.begin(PIN_SD_CS)) {
    return false;
  }

  if (!SD.exists(NOME_ARQUIVO_LOG)) {
    File arquivo = SD.open(NOME_ARQUIVO_LOG, FILE_WRITE);
    if (!arquivo) return false;
    arquivo.println(F("tempo_s,temp_C,corrente_A,rpm,erros,estado_motor,estado_andon"));
    arquivo.close();
  }
  return true;
}

void loggerSDIniciarTempo() {
  inicioLeituraMs = millis();
}

void gravarLeituraSD(float temperatura, float corrente, EstadoAndon estado, int erros) {
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
  arquivo.print(motorParaTexto(estadoMotor, DESTINO_SD)); arquivo.print(F(","));
  arquivo.println(andonParaTexto(estado, DESTINO_SD));

  arquivo.close();  // fecha = grava (flush) no cartão imediatamente
}
