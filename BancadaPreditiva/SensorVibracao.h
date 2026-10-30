#ifndef SENSOR_VIBRACAO_H
#define SENSOR_VIBRACAO_H

#include <Arduino.h>
#include "SensorRPM.h"   // EstadoMotor
#include "config.h"      // ADXL_FS, ADXL_N e demais constantes de compilação

// ----------------------------------------------------------------
//  Vibração — acelerômetro ADXL345 por I2C (SDA=20, SCL=21, endereço em cfg).
//  INT1 não é usado. Wire fica em 100 kHz (o PCF8574 do LCD divide
//  o barramento), o que dá ~1,3 ms por amostra e limita ADXL_FS a 400.
// ----------------------------------------------------------------
#define ADXL_DEVID_ESPERADO    0xE5

#define ADXL_REG_DEVID         0x00
#define ADXL_REG_BW_RATE       0x2C
#define ADXL_REG_POWER_CTL     0x2D
#define ADXL_REG_DATA_FORMAT   0x31
#define ADXL_REG_DATAX0        0x32
#define ADXL_REG_FIFO_CTL      0x38
#define ADXL_REG_FIFO_STATUS   0x39

#define ADXL_DATA_FORMAT       0x09   // FULL_RES | faixa ±4 g
#define ADXL_FIFO_CTL          0x80   // modo STREAM
#define ADXL_POWER_MEASURE     0x08
#define ADXL_FIFO_TAM          32

#define ADXL_ESCALA_FFT        (2.0f / ADXL_N) // magnitude da FFT -> amplitude de pico

// Baseline na EEPROM: byte mágico + assinatura da configuração (configAssinaturaVib()).
// Assinatura diferente = EEPROM tratada como virgem (recalibra).
#define VIB_EEPROM_ENDERECO    0
#define VIB_EEPROM_VALIDO      0xA5

struct VibResultado {
  float freqDominante;   // Hz, maior pico entre cfg.adxlFminHz e cfg.adxlFmaxHz
  float amp1X;           // g, máximo do espectro na banda de rpm/60
  float amp2X;           // g, banda de 2*rpm/60
  float ampAlta;         // g, banda cfg.adxlAltaMinHz..cfg.adxlFmaxHz
};

// Valores gravados na coluna vib_calib do CSV
typedef enum {
  VIB_CALIBRANDO      = 0,
  VIB_CALIBRADO       = 1,
  VIB_BASELINE_EEPROM = 2
} VibCalib;

// Razão amplitude/baseline do pior entre 1X, 2X e ALTA, filtrada por
// mediana das últimas 3 janelas. 0 com motor parado ou sem baseline.
extern float    vibRazao;
extern bool     erroSensorVib;   // cfg.vibFalhasErro janelas seguidas falharam
extern VibCalib vibCalib;

// Configura o ADXL345 e carrega a baseline da EEPROM, se válida.
// false = sem resposta no I2C (não trava o boot).
bool vibracaoInit();

// Fase 1, motor PARADO: DEVID, gravidade e piso de ruído (8 janelas).
// 'pisoRuido' sai em g por bin e fica guardado no módulo.
bool verificarADXL345(float &pisoRuido);

// Coleta uma janela de ADXL_N amostras (~0,32 s), calcula o espectro
// RSS dos 3 eixos e preenche o resultado. false = janela descartada
// (overrun do FIFO ou erro de I2C); 'r' fica inalterado.
bool vibracaoLerJanela(float rpm, VibResultado &r);

// Acumula a baseline quando o motor está estável. Chamar uma vez por
// ciclo, depois de avaliarEstado(), com o 'erros' e a janela desse ciclo.
void vibracaoCalibrar(float rpm, EstadoMotor estado, int erros, bool okVib, const VibResultado &vib);

// Apaga a baseline da EEPROM e volta a calibrar.
void vibracaoRecalibrar();

#endif
