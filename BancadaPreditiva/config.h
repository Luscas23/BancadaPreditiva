#ifndef CONFIG_H
#define CONFIG_H

// PINAGEM [exec]
#define PAD_PIN_RELE_VERDE      22
#define PAD_PIN_RELE_AMARELO    24
#define PAD_PIN_RELE_VERMELHO   26
#define PAD_PIN_TCRT            18
#define PAD_PIN_ACS             A0
#define PAD_PIN_MAX_CS          10

// RELE [exec]
#define PAD_RELE_ATIVO_EM       LOW

// I2C [exec]
#define PAD_I2C_LCD             0x27
#define PAD_I2C_ADXL            0x53
#define LCD_COLS                16
#define LCD_LINHAS              2
#define I2C_TIMEOUT_US          3000

// MOTOR [exec]
#define PAD_MOTOR_CORRENTE_A    2.9
#define PAD_MOTOR_TENSAO_V      127
#define PAD_MOTOR_POTENCIA_W    325
#define PAD_MOTOR_RPM           1740
#define PAD_MOTOR_REDUCAO       1

// LIMITES [exec]
#define PAD_RPM_TOLERANCIA      140
#define PAD_RPM_GRAVE_MIN       1400
#define PAD_RPM_GRAVE_MAX       2000
#define PAD_CORR_RUIM_A         3.3
#define PAD_CORR_GRAVE_A        4.0
#define PAD_CORR_PARADO_A       0.3
#define PAD_TEMP_RUIM_C         80.0
#define PAD_TEMP_GRAVE_C        100.0
#define PAD_VIB_MULT_RUIM       2.0
#define PAD_VIB_MULT_GRAVE      4.0
#define PAD_ERROS_PARA_GRAVE    3

// TEMPOS [exec]
#define PAD_CARENCIA_PARTIDA_MS 5000
#define PAD_RPM_ZERO_RUIM_MS    3000
#define PAD_RPM_TIMEOUT_MS      500
#define PAD_ANDON_HISTERESE     3
#define PAD_ANDON_CONFIRMA_MS   2000
#define LOOP_SEM_JANELA_MS      500
#define FALHA_MSG_MS            1500
#define CFG_MSG_MS              2000
#define WDT_TIMEOUT             WDTO_8S

// TCRT [exec]
#define PAD_TCRT_PULSO_MIN_US   10000
#define RPM_AMOSTRAS            4
#define PAD_TCRT_MARCAS         1

// ACS712
#define PAD_ACS_SENS            0.185
#define PAD_ACS_VCC             5.0
#define PAD_ACS_AMOSTRAS        400
#define PAD_ACS_INTERVALO_US    100
#define PAD_ACS_PISO_A          0.05
#define PAD_ACS_ADC_MIN         350
#define PAD_ACS_ADC_MAX         680
#define ADC_MAX                 1023.0

// PT100 [exec]
#define PAD_PT_FIOS             3
#define PAD_PT_RREF             430.0
#define PAD_PT_RNOM             100.0

// VIBRACAO

#define ADXL_FS                 400
#define ADXL_N                  128
#define ADXL_BW_RATE            0x0C
#define ADXL_G_POR_LSB          0.0039f
#define ADXL_TIMEOUT_AMOSTRA_US 20000UL
#define ADXL_DRENAR_TENTATIVAS  16
#define ADXL_GRAV_MIN           0.7f
#define ADXL_GRAV_MAX           1.3f
#define BASELINE_RPM_ALFA       0.2f

#define PAD_ADXL_FMIN_HZ        6
#define PAD_ADXL_FMAX_HZ        180
#define PAD_ADXL_ALTA_MIN_HZ    120
#define PAD_ADXL_BANDA_BINS     2
#define PAD_ADXL_JANELAS_PISO   8
#define PAD_ADXL_FATOR_PISO     5.0
#define PAD_VIB_FALHAS_ERRO     3
#define PAD_BASELINE_JANELAS    8
#define PAD_BASELINE_CICLOS     25
#define PAD_BASELINE_TOL_RPM    0.05

// SERIAL_SD
#define SERIAL_BAUD             115200
#define PIN_SD_CS               4
#define NOME_ARQ_LOG            "LOG.CSV"
#define NOME_ARQ_CFG            "CONFIG.TXT"
#define CFG_LINHA_MAX           64
#define CMD_RECALIBRAR          'c'

#endif
