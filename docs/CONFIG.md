# Configuracao (gerado por teste/teste_config.cpp --gerar)

| Chave | Unidade | Padrao | Intervalo | Descricao | Afeta baseline |
|---|---|---|---|---|---|
| `pino_rele_verde` | pino | 22 | 2 .. 53 | Pino do rele da luz verde | nao |
| `pino_rele_amarelo` | pino | 24 | 2 .. 53 | Pino do rele da luz amarela | nao |
| `pino_rele_vermelho` | pino | 26 | 2 .. 53 | Pino do rele da luz vermelha | nao |
| `pino_tcrt` | pino | 18 | 2 .. 53 | TCRT5000 (so 2,3,18,19) | nao |
| `pino_acs712` | pino | 54 | 54 .. 69 | ACS712 analogico (A0=54) | nao |
| `pino_cs_max31865` | pino | 10 | 2 .. 53 | CS do MAX31865 | nao |
| `rele_ativo_em` | - | LOW | LOW .. HIGH | Nivel que liga o rele (LOW/HIGH) | nao |
| `i2c_lcd` | - | 0x27 | 0x8 .. 0x77 | Endereco I2C do LCD | nao |
| `i2c_adxl345` | - | 0x53 | 0x8 .. 0x77 | Endereco I2C do ADXL345 | nao |
| `motor_corrente_nominal_a` | A | 2.900 | 0.100 .. 100.000 | Corrente nominal do motor | nao |
| `motor_tensao_v` | V | 127 | 1 .. 1000 | Tensao nominal (informativo) | nao |
| `motor_potencia_w` | W | 325 | 1 .. 60000 | Potencia nominal (informativo) | nao |
| `motor_reducao` | - | 26 | 1 .. 255 | Reducao motor -> eixo do TCRT | sim |
| `motor_rpm_nominal` | RPM | 1740 | 1 .. 20000 | RPM nominal (centro da faixa ideal) | sim |
| `rpm_tolerancia` | RPM | 140 | 1 .. 20000 | Faixa ideal = nominal +- isto | nao |
| `rpm_grave_min` | RPM | 1400 | 1 .. 20000 | Abaixo disso: grave | nao |
| `rpm_grave_max` | RPM | 2000 | 1 .. 20000 | Acima disso: grave | nao |
| `corr_ruim_a` | A | 3.300 | 0.010 .. 100.000 | Acima disso: ruim | nao |
| `corr_grave_a` | A | 4.000 | 0.010 .. 100.000 | Acima disso: grave | nao |
| `corr_parado_a` | A | 0.300 | 0.010 .. 100.000 | Abaixo disso: motor parado | nao |
| `temp_ruim_c` | C | 80.000 | 0.000 .. 300.000 | Acima disso: ruim | nao |
| `temp_grave_c` | C | 100.000 | 0.000 .. 300.000 | Acima disso: grave | nao |
| `vib_mult_ruim` | x | 2.000 | 0.100 .. 100.000 | Razao amplitude/baseline: ruim | nao |
| `vib_mult_grave` | x | 4.000 | 0.100 .. 100.000 | Razao amplitude/baseline: grave | nao |
| `erros_para_grave` | - | 3 | 1 .. 10 | Erros somados que viram GRAVE | nao |
| `carencia_partida_ms` | ms | 5000 | 0 .. 60000 | Apos a partida corrente/RPM nao contam | nao |
| `rpm_zero_ruim_ms` | ms | 3000 | 0 .. 60000 | Corrente com RPM 0 por mais: ruim | nao |
| `rpm_timeout_sem_pulso_ms` | ms | 2000 | 100 .. 60000 | Sem pulso por mais: RPM = 0 | nao |
| `andon_histerese_ciclos` | ciclo | 3 | 1 .. 100 | Ciclos melhores para o Andon melhorar | nao |
| `tcrt_pulso_min_us` | us | 200000 | 1000 .. 10000000 | Pulso mais proximo que isso e ruido | nao |
| `tcrt_marcas_por_volta` | - | 1 | 1 .. 16 | Marcas refletivas por volta | nao |
| `acs_sensibilidade_v_a` | V/A | 0.185 | 0.010 .. 1.000 | Sensibilidade do ACS712 | nao |
| `acs_vcc_v` | V | 5.000 | 1.000 .. 6.000 | Alimentacao / referencia do ADC | nao |
| `acs_amostras` | - | 400 | 10 .. 2000 | Amostras por leitura (~5 ciclos da rede) | nao |
| `acs_intervalo_us` | us | 100 | 0 .. 10000 | Pausa entre amostras | nao |
| `acs_piso_zero_a` | A | 0.050 | 0.000 .. 1.000 | Abaixo disso a corrente vale 0 | nao |
| `acs_adc_min` | - | 350 | 0 .. 1023 | Sanidade do boot: ADC minimo | nao |
| `acs_adc_max` | - | 680 | 0 .. 1023 | Sanidade do boot: ADC maximo | nao |
| `pt100_fios` | - | 3 | 2 .. 4 | Fios do PT100 | nao |
| `pt100_rref_ohm` | ohm | 430.000 | 100.000 .. 10000.000 | Resistor de referencia do MAX31865 | nao |
| `pt100_rnom_ohm` | ohm | 100.000 | 10.000 .. 1000.000 | Resistencia nominal do PT100 | nao |
| `adxl_fmin_hz` | Hz | 6 | 1 .. 100 | Inicio da banda de analise | sim |
| `adxl_fmax_hz` | Hz | 180 | 10 .. 250 | Fim da banda de analise | sim |
| `adxl_alta_min_hz` | Hz | 120 | 5 .. 250 | Inicio da banda ALTA | sim |
| `adxl_banda_bins` | bin | 2 | 0 .. 8 | Tolerancia em bins de 1X/2X | sim |
| `adxl_janelas_piso` | - | 8 | 1 .. 64 | Janelas para medir o piso de ruido | nao |
| `adxl_fator_piso` | x | 5.000 | 0.100 .. 100.000 | Multiplicador do piso de ruido | nao |
| `vib_falhas_erro` | - | 3 | 1 .. 50 | Janelas invalidas seguidas = erro | nao |
| `baseline_janelas` | - | 8 | 1 .. 64 | Janelas acumuladas na baseline | nao |
| `baseline_ciclos_estaveis` | ciclo | 25 | 0 .. 250 | Ciclos estaveis antes de acumular | nao |
| `baseline_tol_rpm` | frac | 0.050 | 0.001 .. 1.000 | Variacao maxima do RPM ao calibrar | nao |
