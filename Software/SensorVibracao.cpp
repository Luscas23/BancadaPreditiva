#include <Wire.h>
#include <EEPROM.h>
#include <arduinoFFT.h>
#include "SensorVibracao.h"

float    vibRazao      = 0.0;
bool     erroSensorVib = false;
VibCalib vibCalib      = VIB_CALIBRANDO;

static int16_t raw[3][ADXL_N];
static float   vReal[ADXL_N];
static float   vImag[ADXL_N];
static float   espectro[ADXL_N / 2];
static ArduinoFFT<float> fft(vReal, vImag, ADXL_N, ADXL_FS);

static float        pisoRuido = 0.0;
static float        baseline[3];
static uint8_t      falhas = 0;
static float        histRazao[3];
static uint8_t      idxHist = 0;

static uint8_t ciclosEstaveis   = 0;
static uint8_t janelasAcumuladas = 0;
static float   somaAmp[3];
static float   rpmMedia = 0.0;

// ---------------- I2C ----------------

static bool escreverReg(uint8_t reg, uint8_t valor) {
  Wire.beginTransmission(ADXL_ENDERECO);
  Wire.write(reg);
  Wire.write(valor);
  return Wire.endTransmission() == 0;
}

static bool lerRegs(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(ADXL_ENDERECO);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)ADXL_ENDERECO, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

// -1 = erro de I2C
static int8_t entradasFifo() {
  uint8_t e;
  if (!lerRegs(ADXL_REG_FIFO_STATUS, &e, 1)) return -1;
  return e & 0x3F;
}

static bool lerAmostra(int16_t s[3]) {
  uint8_t b[6];
  if (!lerRegs(ADXL_REG_DATAX0, b, 6)) return false;
  for (uint8_t i = 0; i < 3; i++)
    s[i] = (int16_t)(((uint16_t)b[2 * i + 1] << 8) | b[2 * i]);
  return true;
}

// Lê as entradas informadas por ENTRIES sem repetir a consulta a cada
// amostra, para alcançar o relógio do sensor.
static bool drenarFifo() {
  int16_t descarte[3];
  for (uint8_t t = 0; t < 16; t++) {
    int8_t e = entradasFifo();
    if (e < 0) return false;
    if (e == 0) return true;
    while (e--) if (!lerAmostra(descarte)) return false;
  }
  return false;
}

static bool lerAmostraFifo(int16_t s[3]) {
  unsigned long t0 = micros();
  int8_t e;
  while ((e = entradasFifo()) == 0) {
    if (micros() - t0 > ADXL_TIMEOUT_AMOSTRA_US) return false;
  }
  if (e < 0 || e >= ADXL_FIFO_TAM) return false;   // erro ou overrun
  return lerAmostra(s);
}

// ---------------- Janela e espectro ----------------

static bool coletarJanela() {
  if (!drenarFifo()) return false;
  for (uint16_t i = 0; i < ADXL_N; i++) {
    int16_t s[3];
    if (!lerAmostraFifo(s)) return false;
    raw[0][i] = s[0];
    raw[1][i] = s[1];
    raw[2][i] = s[2];
  }
  return true;
}

// RSS dos 3 eixos, cada um sem a média (remove a gravidade).
static void calcularEspectro() {
  for (uint8_t k = 0; k < ADXL_N / 2; k++) espectro[k] = 0.0;

  for (uint8_t eixo = 0; eixo < 3; eixo++) {
    long soma = 0;
    for (uint16_t i = 0; i < ADXL_N; i++) soma += raw[eixo][i];
    float media = (float)soma / ADXL_N;

    for (uint16_t i = 0; i < ADXL_N; i++) {
      vReal[i] = (raw[eixo][i] - media) * ADXL_G_POR_LSB;
      vImag[i] = 0.0;
    }
    fft.windowing(FFTWindow::Hann, FFTDirection::Forward, true);
    fft.compute(FFTDirection::Forward);
    fft.complexToMagnitude();

    for (uint8_t k = 0; k < ADXL_N / 2; k++) espectro[k] += vReal[k] * vReal[k];
  }

  for (uint8_t k = 0; k < ADXL_N / 2; k++)
    espectro[k] = sqrtf(espectro[k]) * ADXL_ESCALA_FFT;
}

static bool janelaEspectro() {
  if (!coletarJanela()) return false;
  calcularEspectro();
  return true;
}

static float maxBanda(int lo, int hi) {
  if (lo < 1) lo = 1;                       // bin 0 é DC
  if (hi > ADXL_N / 2 - 1) hi = ADXL_N / 2 - 1;
  float m = 0.0;
  for (int k = lo; k <= hi; k++) if (espectro[k] > m) m = espectro[k];
  return m;
}

static float ampHarmonica(float hz) {
  int centro = (int)(hz * ADXL_N / ADXL_FS + 0.5f);
  return maxBanda(centro - ADXL_BANDA_BINS, centro + ADXL_BANDA_BINS);
}

static float freqDominante() {
  int pico = ADXL_BIN_MIN;
  for (int k = ADXL_BIN_MIN + 1; k <= ADXL_BIN_MAX; k++)
    if (espectro[k] > espectro[pico]) pico = k;

  float a = espectro[pico - 1], b = espectro[pico], c = espectro[pico + 1];
  float d = a - 2.0f * b + c;
  float delta = (d < 0.0f) ? constrain(0.5f * (a - c) / d, -0.5f, 0.5f) : 0.0f;
  return (pico + delta) * ADXL_FS / ADXL_N;
}

// ---------------- Razão contra a baseline ----------------

static float baselineEfetiva(uint8_t b) {
  return fmaxf(baseline[b], pisoRuido);
}

static float mediana3(float a, float b, float c) {
  return fmaxf(fminf(a, b), fminf(fmaxf(a, b), c));
}

static void atualizarRazao(float rpm, const VibResultado &r) {
  float razao = 0.0;
  if (rpm > 0.0 && vibCalib != VIB_CALIBRANDO) {
    razao = fmaxf(r.amp1X / baselineEfetiva(0),
            fmaxf(r.amp2X / baselineEfetiva(1),
                  r.ampAlta / baselineEfetiva(2)));
  }
  histRazao[idxHist] = razao;
  idxHist = (idxHist + 1) % 3;
  vibRazao = mediana3(histRazao[0], histRazao[1], histRazao[2]);
}

// ---------------- Baseline na EEPROM ----------------

struct BaselineEEPROM {
  float   b[3];
  uint8_t valido;
};

static void salvarBaseline() {
  BaselineEEPROM e;
  for (uint8_t i = 0; i < 3; i++) e.b[i] = baseline[i];
  e.valido = VIB_EEPROM_VALIDO;
  EEPROM.put(VIB_EEPROM_ENDERECO, e);
}

static bool carregarBaseline() {
  BaselineEEPROM e;
  EEPROM.get(VIB_EEPROM_ENDERECO, e);
  if (e.valido != VIB_EEPROM_VALIDO) return false;
  for (uint8_t i = 0; i < 3; i++) baseline[i] = e.b[i];
  return true;
}

static void reiniciarCalibracao() {
  ciclosEstaveis    = 0;
  janelasAcumuladas = 0;
  for (uint8_t i = 0; i < 3; i++) somaAmp[i] = 0.0;
}

static void imprimirBaseline() {
  Serial.print(baseline[0], 4); Serial.print(F(" "));
  Serial.print(baseline[1], 4); Serial.print(F(" "));
  Serial.println(baseline[2], 4);
}

// ---------------- API ----------------

bool vibracaoInit() {
  Wire.begin();   // 100 kHz: não usar setClock(400000), o PCF8574 do LCD não suporta
#ifdef WIRE_HAS_TIMEOUT
  Wire.setWireTimeout(3000, true);
#endif

  if (carregarBaseline()) {
    vibCalib = VIB_BASELINE_EEPROM;
    Serial.print(F("Baseline de vibracao recarregada da EEPROM (1X 2X ALTA, g): "));
    imprimirBaseline();
  }

  return escreverReg(ADXL_REG_BW_RATE,     ADXL_BW_RATE)
      && escreverReg(ADXL_REG_DATA_FORMAT, ADXL_DATA_FORMAT)
      && escreverReg(ADXL_REG_FIFO_CTL,    ADXL_FIFO_CTL)
      && escreverReg(ADXL_REG_POWER_CTL,   ADXL_POWER_MEASURE);
}

bool verificarADXL345(float &piso) {
  piso = 0.0;
  pisoRuido = 0.0;

  uint8_t id;
  if (!lerRegs(ADXL_REG_DEVID, &id, 1) || id != ADXL_DEVID_ESPERADO) return false;

  int16_t s[3];
  if (!drenarFifo() || !lerAmostraFifo(s)) return false;
  float x = s[0] * ADXL_G_POR_LSB, y = s[1] * ADXL_G_POR_LSB, z = s[2] * ADXL_G_POR_LSB;
  float g = sqrtf(x * x + y * y + z * z);
  if (g < ADXL_GRAV_MIN || g > ADXL_GRAV_MAX) return false;

  float soma = 0.0;
  unsigned long t0 = micros();
  for (uint8_t j = 0; j < ADXL_JANELAS_PISO; j++) {
    if (!janelaEspectro()) return false;
    for (int k = ADXL_BIN_MIN; k <= ADXL_BIN_MAX; k++) soma += espectro[k];
  }
  unsigned long janelaUs = (micros() - t0) / ADXL_JANELAS_PISO;

  pisoRuido = soma / (ADXL_JANELAS_PISO * (ADXL_BIN_MAX - ADXL_BIN_MIN + 1)) * ADXL_FATOR_PISO;
  piso = pisoRuido;

  Serial.print(F("ADXL345: janela "));  Serial.print(janelaUs / 1000.0, 1);
  Serial.print(F(" ms, piso "));        Serial.print(pisoRuido, 5);
  Serial.println(F(" g"));
  return true;
}

bool vibracaoLerJanela(float rpm, VibResultado &r) {
  if (!janelaEspectro()) {
    if (falhas < VIB_FALHAS_ERRO) falhas++;
    erroSensorVib = (falhas >= VIB_FALHAS_ERRO);
    return false;
  }
  falhas        = 0;
  erroSensorVib = false;

  r.freqDominante = freqDominante();
  r.amp1X         = ampHarmonica(rpm / 60.0f);
  r.amp2X         = ampHarmonica(2.0f * rpm / 60.0f);
  r.ampAlta       = maxBanda(ADXL_BIN_ALTA_MIN, ADXL_BIN_MAX);

  atualizarRazao(rpm, r);
  return true;
}

void vibracaoCalibrar(float rpm, EstadoMotor estado, int erros, bool okVib, const VibResultado &vib) {
  if (vibCalib != VIB_CALIBRANDO) return;

  if (!okVib || estado != MOTOR_OPERANDO || erros != 0) {
    reiniciarCalibracao();
    return;
  }
  if (ciclosEstaveis == 0) rpmMedia = rpm;
  if (fabsf(rpm - rpmMedia) > BASELINE_TOL_RPM * rpmMedia) {
    reiniciarCalibracao();
    return;
  }
  rpmMedia += (rpm - rpmMedia) * BASELINE_RPM_ALFA;

  if (ciclosEstaveis < BASELINE_CICLOS_ESTAVEIS) {
    ciclosEstaveis++;
    return;
  }

  somaAmp[0] += vib.amp1X;
  somaAmp[1] += vib.amp2X;
  somaAmp[2] += vib.ampAlta;
  if (++janelasAcumuladas < BASELINE_JANELAS) return;

  for (uint8_t i = 0; i < 3; i++) baseline[i] = somaAmp[i] / BASELINE_JANELAS;
  salvarBaseline();
  vibCalib = VIB_CALIBRADO;
  Serial.print(F("Baseline de vibracao calibrada (1X 2X ALTA, g): "));
  imprimirBaseline();
}

void vibracaoRecalibrar() {
  EEPROM.update(VIB_EEPROM_ENDERECO + offsetof(BaselineEEPROM, valido), 0);
  vibCalib = VIB_CALIBRANDO;
  reiniciarCalibracao();
  Serial.println(F("Recalibrando vibracao"));
}
