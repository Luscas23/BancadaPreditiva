#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "Configuracao.h"
#include "config.h"

Config cfg;

enum { T_U8, T_HEX, T_NIVEL, T_U16, T_U32, T_F };   // T_U8..T_NIVEL ocupam 1 byte
enum { G_PINOS, G_RELE, G_I2C, G_MOTOR, G_RPM, G_CORR, G_TEMP, G_VIB, G_TEMPOS, G_TCRT,
       G_ACS, G_PT, G_BANDAS, G_VIBCAL };

// Uma tabela alimenta leitura, validação de faixa, padrões, impressão e documentação.
// I(chave, tipo, grupo, campo, padrão, mín, máx, unidade, descrição, afeta baseline)
#define I(n, t, g, c, d, mn, mx, u, ds, bl) \
  { n, u, ds, t, g, bl, offsetof(Config, c), d, mn, mx }

static const ConfigItem TAB[] PROGMEM = {
  I("pino_rele_verde",    T_U8,  G_PINOS, pinReleVerde,    PAD_PIN_RELE_VERDE,    2, 53, "pino", "Pino do rele da luz verde", 0),
  I("pino_rele_amarelo",  T_U8,  G_PINOS, pinReleAmarelo,  PAD_PIN_RELE_AMARELO,  2, 53, "pino", "Pino do rele da luz amarela", 0),
  I("pino_rele_vermelho", T_U8,  G_PINOS, pinReleVermelho, PAD_PIN_RELE_VERMELHO, 2, 53, "pino", "Pino do rele da luz vermelha", 0),
  I("pino_tcrt",          T_U8,  G_PINOS, pinTcrt,         PAD_PIN_TCRT,          2, 53, "pino", "TCRT5000 (so 2,3,18,19)", 0),
  I("pino_acs712",        T_U8,  G_PINOS, pinAcs,          PAD_PIN_ACS,          54, 69, "pino", "ACS712 analogico (A0=54)", 0),
  I("pino_cs_max31865",   T_U8,  G_PINOS, pinMaxCs,        PAD_PIN_MAX_CS,        2, 53, "pino", "CS do MAX31865", 0),
  I("rele_ativo_em",      T_NIVEL, G_RELE, releAtivoEm,    PAD_RELE_ATIVO_EM,     0, 1, "-", "Nivel que liga o rele (LOW/HIGH)", 0),
  I("i2c_lcd",            T_HEX, G_I2C,  enderecoLcd,      PAD_I2C_LCD,        0x08, 0x77, "-", "Endereco I2C do LCD", 0),
  I("i2c_adxl345",        T_HEX, G_I2C,  enderecoAdxl,     PAD_I2C_ADXL,       0x08, 0x77, "-", "Endereco I2C do ADXL345", 0),
  I("motor_corrente_nominal_a", T_F, G_MOTOR, motorCorrenteA, PAD_MOTOR_CORRENTE_A, 0.1, 100, "A", "Corrente nominal do motor", 0),
  I("motor_tensao_v",     T_U16, G_MOTOR, motorTensaoV,    PAD_MOTOR_TENSAO_V,    1, 1000, "V", "Tensao nominal (informativo)", 0),
  I("motor_potencia_w",   T_U16, G_MOTOR, motorPotenciaW,  PAD_MOTOR_POTENCIA_W,  1, 60000, "W", "Potencia nominal (informativo)", 0),
  I("motor_reducao",      T_U8,  G_MOTOR, reducao,         PAD_MOTOR_REDUCAO,     1, 255, "-", "Reducao motor -> eixo do TCRT", 1),
  I("motor_rpm_nominal",  T_U16, G_RPM,  rpmNominal,       PAD_MOTOR_RPM,         1, 20000, "RPM", "RPM nominal (centro da faixa ideal)", 1),
  I("rpm_tolerancia",     T_U16, G_RPM,  rpmTolerancia,    PAD_RPM_TOLERANCIA,    1, 20000, "RPM", "Faixa ideal = nominal +- isto", 0),
  I("rpm_grave_min",      T_U16, G_RPM,  rpmGraveMin,      PAD_RPM_GRAVE_MIN,     1, 20000, "RPM", "Abaixo disso: grave", 0),
  I("rpm_grave_max",      T_U16, G_RPM,  rpmGraveMax,      PAD_RPM_GRAVE_MAX,     1, 20000, "RPM", "Acima disso: grave", 0),
  I("corr_ruim_a",        T_F,   G_CORR, corrRuimA,        PAD_CORR_RUIM_A,       0.01, 100, "A", "Acima disso: ruim", 0),
  I("corr_grave_a",       T_F,   G_CORR, corrGraveA,       PAD_CORR_GRAVE_A,      0.01, 100, "A", "Acima disso: grave", 0),
  I("corr_parado_a",      T_F,   G_CORR, corrParadoA,      PAD_CORR_PARADO_A,     0.01, 100, "A", "Abaixo disso: motor parado", 0),
  I("temp_ruim_c",        T_F,   G_TEMP, tempRuimC,        PAD_TEMP_RUIM_C,       0, 300, "C", "Acima disso: ruim", 0),
  I("temp_grave_c",       T_F,   G_TEMP, tempGraveC,       PAD_TEMP_GRAVE_C,      0, 300, "C", "Acima disso: grave", 0),
  I("vib_mult_ruim",      T_F,   G_VIB,  vibMultRuim,      PAD_VIB_MULT_RUIM,     0.1, 100, "x", "Razao amplitude/baseline: ruim", 0),
  I("vib_mult_grave",     T_F,   G_VIB,  vibMultGrave,     PAD_VIB_MULT_GRAVE,    0.1, 100, "x", "Razao amplitude/baseline: grave", 0),
  I("erros_para_grave",   T_U8,  G_TEMPOS, errosParaGrave, PAD_ERROS_PARA_GRAVE,  1, 10, "-", "Erros somados que viram GRAVE", 0),
  I("carencia_partida_ms", T_U16, G_TEMPOS, carenciaPartidaMs, PAD_CARENCIA_PARTIDA_MS, 0, 60000, "ms", "Apos a partida corrente/RPM nao contam", 0),
  I("rpm_zero_ruim_ms",   T_U16, G_TEMPOS, rpmZeroRuimMs,  PAD_RPM_ZERO_RUIM_MS,  0, 60000, "ms", "Corrente com RPM 0 por mais: ruim", 0),
  I("rpm_timeout_sem_pulso_ms", T_U16, G_TEMPOS, rpmTimeoutMs, PAD_RPM_TIMEOUT_MS, 100, 60000, "ms", "Sem pulso por mais: RPM = 0", 0),
  I("andon_histerese_ciclos", T_U8, G_TEMPOS, andonHisterese, PAD_ANDON_HISTERESE, 1, 100, "ciclo", "Ciclos melhores para o Andon melhorar", 0),
  I("tcrt_pulso_min_us",  T_U32, G_TCRT, pulsoMinUs,       PAD_TCRT_PULSO_MIN_US, 1000, 10000000, "us", "Pulso mais proximo que isso e ruido", 0),
  I("tcrt_marcas_por_volta", T_U8, G_TCRT, tcrtMarcas,     PAD_TCRT_MARCAS,       1, 16, "-", "Marcas refletivas por volta", 0),
  I("acs_sensibilidade_v_a", T_F, G_ACS, acsSens,          PAD_ACS_SENS,          0.01, 1, "V/A", "Sensibilidade do ACS712", 0),
  I("acs_vcc_v",          T_F,   G_ACS,  acsVcc,           PAD_ACS_VCC,           1, 6, "V", "Alimentacao / referencia do ADC", 0),
  I("acs_amostras",       T_U16, G_ACS,  acsAmostras,      PAD_ACS_AMOSTRAS,      10, 2000, "-", "Amostras por leitura (~5 ciclos da rede)", 0),
  I("acs_intervalo_us",   T_U16, G_ACS,  acsIntervaloUs,   PAD_ACS_INTERVALO_US,  0, 10000, "us", "Pausa entre amostras", 0),
  I("acs_piso_zero_a",    T_F,   G_ACS,  acsPisoA,         PAD_ACS_PISO_A,        0, 1, "A", "Abaixo disso a corrente vale 0", 0),
  I("acs_adc_min",        T_U16, G_ACS,  acsAdcMin,        PAD_ACS_ADC_MIN,       0, 1023, "-", "Sanidade do boot: ADC minimo", 0),
  I("acs_adc_max",        T_U16, G_ACS,  acsAdcMax,        PAD_ACS_ADC_MAX,       0, 1023, "-", "Sanidade do boot: ADC maximo", 0),
  I("pt100_fios",         T_U8,  G_PT,   ptFios,           PAD_PT_FIOS,           2, 4, "-", "Fios do PT100", 0),
  I("pt100_rref_ohm",     T_F,   G_PT,   ptRref,           PAD_PT_RREF,           100, 10000, "ohm", "Resistor de referencia do MAX31865", 0),
  I("pt100_rnom_ohm",     T_F,   G_PT,   ptRnom,           PAD_PT_RNOM,           10, 1000, "ohm", "Resistencia nominal do PT100", 0),
  I("adxl_fmin_hz",       T_U8,  G_BANDAS, adxlFminHz,     PAD_ADXL_FMIN_HZ,      1, 100, "Hz", "Inicio da banda de analise", 1),
  I("adxl_fmax_hz",       T_U8,  G_BANDAS, adxlFmaxHz,     PAD_ADXL_FMAX_HZ,      10, 250, "Hz", "Fim da banda de analise", 1),
  I("adxl_alta_min_hz",   T_U8,  G_BANDAS, adxlAltaMinHz,  PAD_ADXL_ALTA_MIN_HZ,  5, 250, "Hz", "Inicio da banda ALTA", 1),
  I("adxl_banda_bins",    T_U8,  G_BANDAS, adxlBandaBins,  PAD_ADXL_BANDA_BINS,   0, 8, "bin", "Tolerancia em bins de 1X/2X", 1),
  I("adxl_janelas_piso",  T_U8,  G_VIBCAL, adxlJanelasPiso, PAD_ADXL_JANELAS_PISO, 1, 64, "-", "Janelas para medir o piso de ruido", 0),
  I("adxl_fator_piso",    T_F,   G_VIBCAL, adxlFatorPiso,  PAD_ADXL_FATOR_PISO,   0.1, 100, "x", "Multiplicador do piso de ruido", 0),
  I("vib_falhas_erro",    T_U8,  G_VIBCAL, vibFalhasErro,  PAD_VIB_FALHAS_ERRO,   1, 50, "-", "Janelas invalidas seguidas = erro", 0),
  I("baseline_janelas",   T_U8,  G_VIBCAL, baselineJanelas, PAD_BASELINE_JANELAS, 1, 64, "-", "Janelas acumuladas na baseline", 0),
  I("baseline_ciclos_estaveis", T_U8, G_VIBCAL, baselineCiclos, PAD_BASELINE_CICLOS, 0, 250, "ciclo", "Ciclos estaveis antes de acumular", 0),
  I("baseline_tol_rpm",   T_F,   G_VIBCAL, baselineTolRpm, PAD_BASELINE_TOL_RPM,  0.001, 1, "frac", "Variacao maxima do RPM ao calibrar", 0),
};
#undef I

uint8_t configNumItens() { return sizeof TAB / sizeof TAB[0]; }
void configItem(uint8_t i, ConfigItem &it) { memcpy_P(&it, &TAB[i], sizeof it); }

float configValor(const Config &c, const ConfigItem &it) {
  const uint8_t *p = (const uint8_t *)&c + it.off;
  switch (it.tipo) {
    case T_F:   { float f;    memcpy(&f, p, 4); return f; }
    case T_U32: { uint32_t v; memcpy(&v, p, 4); return (float)v; }
    case T_U16: { uint16_t v; memcpy(&v, p, 2); return v; }
    default:    return *p;
  }
}

static void gravar(Config &c, const ConfigItem &it, float v) {
  uint8_t *p = (uint8_t *)&c + it.off;
  switch (it.tipo) {
    case T_F:   memcpy(p, &v, 4); break;
    case T_U32: { uint32_t x = (uint32_t)v; memcpy(p, &x, 4); break; }
    case T_U16: { uint16_t x = (uint16_t)v; memcpy(p, &x, 2); break; }
    default:    *p = (uint8_t)v;
  }
}

static void preencherPadrao(Config &c) {
  memset(&c, 0, sizeof c);
  ConfigItem it;
  for (uint8_t i = 0; i < configNumItens(); i++) { configItem(i, it); gravar(c, it, it.def); }
}

void configPadrao() { preencherPadrao(cfg); }

// ---------------- Leitura ----------------

static char *trim(char *s) {
  while (isspace((uint8_t)*s)) s++;
  char *e = s + strlen(s);
  while (e > s && isspace((uint8_t)e[-1])) *--e = 0;
  return s;
}

int configAplicarLinha(char *l, Print &log) {
  l = trim(l);
  if (!*l || *l == '#') return 0;
  char *eq = strchr(l, '=');
  if (!eq) { log.println(F("CFG: linha sem '=' ignorada")); return 1; }
  *eq = 0;
  char *chave = trim(l), *val = trim(eq + 1);

  ConfigItem it;
  uint8_t i;
  for (i = 0; i < configNumItens(); i++) { configItem(i, it); if (!strcmp(it.nome, chave)) break; }
  if (i == configNumItens()) { log.print(F("CFG: chave desconhecida: ")); log.println(chave); return 1; }

  float v = 0;
  bool ok;
  char *fim;
  if (it.tipo == T_NIVEL) {
    ok = true;
    if      (!strcasecmp(val, "LOW"))  v = LOW;
    else if (!strcasecmp(val, "HIGH")) v = HIGH;
    else ok = false;
  } else {
    v  = it.tipo == T_F ? (float)strtod(val, &fim) : (float)strtol(val, &fim, 0);
    ok = fim != val && !*fim;
  }
  if (!ok) { log.print(F("CFG: valor invalido: ")); log.println(chave); return 1; }
  if (!(v >= it.min && v <= it.max)) {   // !(...) também rejeita NaN
    log.print(F("CFG: fora da faixa: ")); log.println(chave); return 1;
  }
  gravar(cfg, it, v);
  return 0;
}

int configAlimentar(LinhaCfg &l, char c, Print &log) {
  if (c == '\r') return 0;
  if (c != '\n') {
    if (l.n < sizeof l.b - 1) l.b[l.n++] = c; else l.longa = true;
    return 0;
  }
  int r = 0;
  if (l.longa) { if (l.b[0] != '#') { log.println(F("CFG: linha longa ignorada")); r = 1; } }   // comentário longo é inofensivo
  else if (l.n) { l.b[l.n] = 0; r = configAplicarLinha(l.b, log); }
  l.n = 0; l.longa = false;
  return r;
}

// ---------------- Validação cruzada ----------------

static int reverter(uint8_t g, const Config &d, Print &log, const __FlashStringHelper *msg) {
  ConfigItem it;
  for (uint8_t i = 0; i < configNumItens(); i++) {
    configItem(i, it);
    if (it.grupo == g) gravar(cfg, it, configValor(d, it));
  }
  log.print(F("CFG: ")); log.print(msg); log.println(F(" - grupo volta ao padrao"));
  return 1;
}

int configValidar(Print &log) {
  Config d;
  preencherPadrao(d);
  int n = 0;

  const uint8_t p[] = { cfg.pinReleVerde, cfg.pinReleAmarelo, cfg.pinReleVermelho,
                        cfg.pinTcrt, cfg.pinAcs, cfg.pinMaxCs, PIN_SD_CS };
  bool ruim = digitalPinToInterrupt(cfg.pinTcrt) == NOT_AN_INTERRUPT;
  for (uint8_t i = 0; i < 7; i++) {
    // Reservados: Serial (0,1), I2C (20,21), SPI (50-53)
    if (i < 6 && (p[i] < 2 || p[i] == 20 || p[i] == 21 || (p[i] >= 50 && p[i] <= 53))) ruim = true;
    for (uint8_t j = i + 1; j < 7; j++) if (p[i] == p[j]) ruim = true;
  }
  if (ruim) n += reverter(G_PINOS, d, log, F("pino repetido, reservado ou TCRT sem interrupcao"));

  if (cfg.enderecoLcd == cfg.enderecoAdxl) n += reverter(G_I2C, d, log, F("enderecos I2C iguais"));

  if (!(cfg.corrParadoA < cfg.corrRuimA && cfg.corrRuimA < cfg.corrGraveA))
    n += reverter(G_CORR, d, log, F("corrente: parado < ruim < grave"));
  if (!(cfg.tempRuimC < cfg.tempGraveC)) n += reverter(G_TEMP, d, log, F("temperatura: ruim < grave"));
  if (!(cfg.vibMultRuim < cfg.vibMultGrave)) n += reverter(G_VIB, d, log, F("vibracao: ruim < grave"));

  long ideMin = (long)cfg.rpmNominal - cfg.rpmTolerancia, ideMax = (long)cfg.rpmNominal + cfg.rpmTolerancia;
  if (!(cfg.rpmGraveMin < ideMin && ideMax < cfg.rpmGraveMax))
    n += reverter(G_RPM, d, log, F("RPM: grave min < ideal < grave max"));

  if (!(cfg.acsAdcMin < cfg.acsAdcMax)) n += reverter(G_ACS, d, log, F("ACS712: adc min < max"));

  if (!(cfg.adxlFminHz < cfg.adxlAltaMinHz && cfg.adxlAltaMinHz < cfg.adxlFmaxHz &&
        (int)cfg.adxlFmaxHz * ADXL_N / ADXL_FS <= ADXL_N / 2 - 2))
    n += reverter(G_BANDAS, d, log, F("bandas FFT: fmin < alta < fmax < Nyquist"));

  // Só avisos: o resto continua valendo
  float periodoUs = 60e6f * cfg.reducao / cfg.rpmNominal / cfg.tcrtMarcas;
  if (!(periodoUs > 2.0f * cfg.pulsoMinUs)) {
    log.println(F("CFG: AVISO periodo do TCRT proximo de tcrt_pulso_min_us (RPM seria filtrado)")); n++;
  }
  float f1 = cfg.rpmNominal / 60.0f;
  if (!(f1 > cfg.adxlFminHz && f1 < ADXL_FS / 2.0f)) {
    log.println(F("CFG: *** ATENCAO: 1X fora da banda da FFT, analise 1X/2X sem sentido ***")); n++;
  }
  return n;
}

// ---------------- Derivados ----------------

LimitesAvaliacao limitesDeConfig() {
  LimitesAvaliacao l = {
    (float)cfg.rpmNominal, (float)cfg.rpmTolerancia, (float)cfg.rpmGraveMin, (float)cfg.rpmGraveMax,
    cfg.vibMultRuim, cfg.vibMultGrave, cfg.tempRuimC, cfg.tempGraveC,
    cfg.corrRuimA, cfg.corrGraveA, cfg.corrParadoA,
    cfg.carenciaPartidaMs, cfg.rpmZeroRuimMs, cfg.errosParaGrave
  };
  return l;
}

static uint16_t crc16(const void *p, uint8_t n) {
  uint16_t c = 0xFFFF;
  const uint8_t *b = (const uint8_t *)p;
  while (n--) {
    c ^= *b++;
    for (uint8_t i = 0; i < 8; i++) c = (c & 1) ? (c >> 1) ^ 0xA001 : c >> 1;
  }
  return c;
}

uint16_t configAssinaturaVib() {
  const uint16_t s[] = { ADXL_FS, ADXL_N, cfg.adxlFminHz, cfg.adxlFmaxHz, cfg.adxlAltaMinHz,
                         cfg.adxlBandaBins, cfg.rpmNominal, cfg.reducao };
  return crc16(s, sizeof s);
}

// ---------------- Impressão ----------------

void configImprimirValor(Print &p, const ConfigItem &it, float v) {
  switch (it.tipo) {
    case T_F:     p.print(v, 3); break;
    case T_HEX:   p.print(F("0x")); p.print((unsigned long)v, HEX); break;
    case T_NIVEL: p.print(v ? F("HIGH") : F("LOW")); break;
    default:      p.print((long)v);
  }
}

void configImprimir(Print &p, const char *pre, bool doc) {
  ConfigItem it;
  for (uint8_t i = 0; i < configNumItens(); i++) {
    configItem(i, it);
    if (doc) {
      p.print(F("# ")); p.print(it.desc); p.print(F(" [")); p.print(it.un); p.print(F(", faixa "));
      configImprimirValor(p, it, it.min); p.print(F("..")); configImprimirValor(p, it, it.max);
      p.println(F("]"));
    }
    p.print(pre); p.print(it.nome); p.print(F(" = "));
    configImprimirValor(p, it, configValor(cfg, it));
    p.println();
  }
}
