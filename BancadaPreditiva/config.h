#ifndef CONFIG_H
#define CONFIG_H

// Camada 1: padrões de fábrica. O CONFIG.TXT no SD sobrescreve os PAD_* na
// inicialização (chaves em Configuracao.cpp). Os demais só mudam recompilando.

// ===== PINAGEM [exec] =====
#define PAD_PIN_RELE_VERDE      22     // pino do relé da luz verde
#define PAD_PIN_RELE_AMARELO    24     // pino do relé da luz amarela
#define PAD_PIN_RELE_VERMELHO   26     // pino do relé da luz vermelha
#define PAD_PIN_TCRT            18     // TCRT5000 (precisa de interrupção: 2,3,18,19)
#define PAD_PIN_ACS             A0     // ACS712 (analógico, A0 = 54)
#define PAD_PIN_MAX_CS          10     // CS do MAX31865 (SPI: SCK=52 MISO=50 MOSI=51)

// ===== RELE [exec] =====
#define PAD_RELE_ATIVO_EM       LOW    // nível que liga o relé

// ===== I2C [exec] =====
#define PAD_I2C_LCD             0x27   // endereço do LCD
#define PAD_I2C_ADXL            0x53   // endereço do ADXL345
#define LCD_COLS                16     // colunas do LCD (layout fixo em Display.cpp)
#define LCD_LINHAS              2      // linhas do LCD
#define I2C_TIMEOUT_US          3000   // timeout do Wire (us)

// ===== MOTOR [exec] =====
#define PAD_MOTOR_CORRENTE_A    2.9    // corrente nominal (A)
#define PAD_MOTOR_TENSAO_V      127    // tensão nominal (V), informativo
#define PAD_MOTOR_POTENCIA_W    325    // potência nominal (W), informativo
#define PAD_MOTOR_RPM           1740   // RPM nominal = setpoint da faixa ideal
#define PAD_MOTOR_REDUCAO       1      // redução motor -> TCRT (1 = TCRT no eixo do motor)

// ===== LIMITES [exec] =====
#define PAD_RPM_TOLERANCIA      140    // ideal = nominal +- isto (RPM)
#define PAD_RPM_GRAVE_MIN       1400   // abaixo disso: grave (RPM)
#define PAD_RPM_GRAVE_MAX       2000   // acima disso: grave (RPM)
#define PAD_CORR_RUIM_A         3.3    // acima: ruim (A)
#define PAD_CORR_GRAVE_A        4.0    // acima: grave (A)
#define PAD_CORR_PARADO_A       0.3    // abaixo: motor parado (A)
#define PAD_TEMP_RUIM_C         80.0   // acima: ruim (C)
#define PAD_TEMP_GRAVE_C        100.0  // acima: grave (C)
#define PAD_VIB_MULT_RUIM       2.0    // razão amplitude/baseline: ruim
#define PAD_VIB_MULT_GRAVE      4.0    // razão amplitude/baseline: grave
#define PAD_ERROS_PARA_GRAVE    3      // erros somados que viram GRAVE

// ===== TEMPOS [exec] =====
#define PAD_CARENCIA_PARTIDA_MS 5000   // após a partida, corrente e RPM não contam
#define PAD_RPM_ZERO_RUIM_MS    3000   // corrente com RPM 0 por mais que isso = ruim
#define PAD_RPM_TIMEOUT_MS      500    // sem pulso por mais que isso: RPM = 0 (~15 voltas a 1740 RPM)
#define PAD_ANDON_HISTERESE     3      // ciclos melhores seguidos para melhorar o Andon
#define LOOP_SEM_JANELA_MS      500    // pausa do loop sem janela de vibração válida
#define FALHA_MSG_MS            1500   // tempo da mensagem de falha de sensor no LCD
#define CFG_MSG_MS              2000   // tempo da mensagem de aviso de config no LCD
#define WDT_TIMEOUT             WDTO_8S // timeout do watchdog

// ===== TCRT [exec] =====
#define PAD_TCRT_PULSO_MIN_US   10000  // pulso mais próximo que isso é ruído (us); aceita até ~6000 RPM
#define RPM_AMOSTRAS            4      // períodos na média móvel do RPM (potência de 2; exige recompilar)
#define PAD_TCRT_MARCAS         1      // marcas refletivas por volta do eixo

// ===== ACS712 =====
#define PAD_ACS_SENS            0.185  // sensibilidade (V/A), ACS712-5A [exec]
#define PAD_ACS_VCC             5.0    // alimentação / referência do ADC (V) [exec]
#define PAD_ACS_AMOSTRAS        400    // amostras por leitura (~84 ms = 5 ciclos de 60 Hz) [exec]
#define PAD_ACS_INTERVALO_US    100    // pausa entre amostras (us) [exec]
#define PAD_ACS_PISO_A          0.05   // abaixo disso a corrente vale 0 (A) [exec]
#define PAD_ACS_ADC_MIN         350    // sanidade do boot: ADC em repouso, mínimo [exec]
#define PAD_ACS_ADC_MAX         680    // sanidade do boot: ADC em repouso, máximo [exec]
#define ADC_MAX                 1023.0 // fundo de escala do ADC (hardware)

// ===== PT100 [exec] =====
#define PAD_PT_FIOS             3      // fios do PT100 (2, 3 ou 4)
#define PAD_PT_RREF             430.0  // resistor de referência do MAX31865 (ohm)
#define PAD_PT_RNOM             100.0  // resistência nominal do PT100 a 0 C (ohm)

// ===== VIBRACAO =====
// Compilação: definem arrays e o registro BW_RATE do ADXL345.
#define ADXL_FS                 400    // taxa de amostragem (Hz); Wire a 100 kHz limita a 400
#define ADXL_N                  128    // amostras por janela da FFT (potência de 2)
#define ADXL_BW_RATE            0x0C   // registro BW_RATE: 400 Hz (muda junto com ADXL_FS)
#define ADXL_G_POR_LSB          0.0039f // full-res: ~3,9 mg/LSB
#define ADXL_TIMEOUT_AMOSTRA_US 20000UL // espera máxima por uma amostra do FIFO
#define ADXL_DRENAR_TENTATIVAS  16     // consultas ao FIFO ao esvaziá-lo
#define ADXL_GRAV_MIN           0.7f   // sanidade do boot: gravidade mínima (g)
#define ADXL_GRAV_MAX           1.3f   // sanidade do boot: gravidade máxima (g)
#define BASELINE_RPM_ALFA       0.2f   // peso da média móvel do RPM na calibração
// Execução [exec]
#define PAD_ADXL_FMIN_HZ        6      // início da banda de análise (Hz) [baseline]
#define PAD_ADXL_FMAX_HZ        180    // fim da banda de análise (Hz) [baseline]
#define PAD_ADXL_ALTA_MIN_HZ    120    // início da banda ALTA (Hz) [baseline]
#define PAD_ADXL_BANDA_BINS     2      // tolerância em bins em torno de 1X/2X [baseline]
#define PAD_ADXL_JANELAS_PISO   8      // janelas para medir o piso de ruído no boot
#define PAD_ADXL_FATOR_PISO     5.0    // multiplicador do piso de ruído
#define PAD_VIB_FALHAS_ERRO     3      // janelas inválidas seguidas = erro do sensor
#define PAD_BASELINE_JANELAS    8      // janelas acumuladas para a baseline
#define PAD_BASELINE_CICLOS     25     // ciclos estáveis antes de acumular a baseline
#define PAD_BASELINE_TOL_RPM    0.05   // variação máxima do RPM (fração) para calibrar

// ===== SERIAL_SD =====
#define SERIAL_BAUD             115200 // baud da Serial
#define PIN_SD_CS               4      // CS do SD (mesmo barramento SPI do PT100)
#define NOME_ARQ_LOG            "LOG.CSV"    // log de leituras
#define NOME_ARQ_CFG            "CONFIG.TXT" // configuração opcional no SD
#define CFG_LINHA_MAX           64     // tamanho máximo de uma linha do CONFIG.TXT
#define CMD_RECALIBRAR          'c'    // comando Serial que recalibra a vibração

#endif
