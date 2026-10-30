#ifndef SENSOR_VIBRACAO_H
#define SENSOR_VIBRACAO_H

#include <Arduino.h>
#include "SensorRPM.h"
#include "config.h"

#define ADXL_DEVID_ESPERADO    0xE5

#define ADXL_REG_DEVID         0x00
#define ADXL_REG_BW_RATE       0x2C
#define ADXL_REG_POWER_CTL     0x2D
#define ADXL_REG_DATA_FORMAT   0x31
#define ADXL_REG_DATAX0        0x32
#define ADXL_REG_FIFO_CTL      0x38
#define ADXL_REG_FIFO_STATUS   0x39

#define ADXL_DATA_FORMAT       0x09
#define ADXL_FIFO_CTL          0x80
#define ADXL_POWER_MEASURE     0x08
#define ADXL_FIFO_TAM          32

#define ADXL_ESCALA_FFT        (2.0f / ADXL_N)

#define VIB_EEPROM_ENDERECO    0
#define VIB_EEPROM_VALIDO      0xA5

struct VibResultado {
  float freqDominante;
  float amp1X;
  float amp2X;
  float ampAlta;
};

typedef enum {
  VIB_CALIBRANDO      = 0,
  VIB_CALIBRADO       = 1,
  VIB_BASELINE_EEPROM = 2
} VibCalib;

extern float    vibRazao;
extern bool     erroSensorVib;
extern VibCalib vibCalib;

bool vibracaoInit();

bool verificarADXL345(float &pisoRuido);

bool vibracaoLerJanela(float rpm, VibResultado &r);

void vibracaoCalibrar(float rpm, EstadoMotor estado, int erros, bool okVib, const VibResultado &vib);

void vibracaoRecalibrar();

#endif
