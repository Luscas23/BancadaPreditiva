#include <assert.h>
#include <string>
#include "Configuracao.h"
#include "config.h"

struct Saida : Print {
  std::string s;
  size_t write(uint8_t c) override { s += (char)c; return 1; }
};

static Config padrao;
static void zerar() { configPadrao(); padrao = cfg; }
static int linha(const char *t, Print &o) { char b[CFG_LINHA_MAX * 2]; snprintf(b, sizeof b, "%s", t); return configAplicarLinha(b, o); }
static int texto(const char *t, Print &o) {
  LinhaCfg l = {}; int a = 0;
  for (; *t; t++) a += configAlimentar(l, *t, o);
  return a + configAlimentar(l, '\n', o);
}
static bool igual(const Config &a, const Config &b) { return !memcmp(&a, &b, sizeof a); }
static std::string ler(const char *arq) {
  FILE *f = fopen(arq, "rb"); if (!f) return "";
  std::string r; int c; while ((c = fgetc(f)) != EOF) r += (char)c; fclose(f); return r;
}

static std::string exemplo() { Saida o; configPadrao(); configImprimir(o, "", true); return o.s; }

static std::string markdown() {
  Saida o; ConfigItem it;
  o.print("# Configuracao (gerado por teste/teste_config.cpp --gerar)\n\n"
          "| Chave | Unidade | Padrao | Intervalo | Descricao | Afeta baseline |\n|---|---|---|---|---|---|\n");
  for (uint8_t i = 0; i < configNumItens(); i++) {
    configItem(i, it);
    o.print("| `"); o.print(it.nome); o.print("` | "); o.print(it.un); o.print(" | ");
    configImprimirValor(o, it, it.def); o.print(" | ");
    configImprimirValor(o, it, it.min); o.print(" .. "); configImprimirValor(o, it, it.max);
    o.print(" | "); o.print(it.desc); o.print(it.baseline ? " | sim |\n" : " | nao |\n");
  }
  return o.s;
}

int main(int argc, char **) {
  if (argc > 1) {
    FILE *f = fopen("exemplo/CONFIG.TXT", "wb"); fputs(exemplo().c_str(), f); fclose(f);
    f = fopen("docs/CONFIG.md", "wb"); fputs(markdown().c_str(), f); fclose(f);
    puts("gerado"); return 0;
  }
  Saida o;

  zerar();
  assert(linha("# comentario", o) == 0 && linha("", o) == 0 && linha("   ", o) == 0);
  assert(igual(cfg, padrao));
  assert(linha("  pino_rele_verde   =   30  ", o) == 0 && cfg.pinReleVerde == 30);
  assert(linha("i2c_lcd = 0x3F", o) == 0 && cfg.enderecoLcd == 0x3F);
  assert(linha("rele_ativo_em = HIGH", o) == 0 && cfg.releAtivoEm == HIGH);
  assert(linha("rele_ativo_em = low", o) == 0 && cfg.releAtivoEm == LOW);
  assert(linha("corr_ruim_a = 3.5", o) == 0 && cfg.corrRuimA == 3.5f);
  assert(linha("chave_inexistente = 1", o) == 1);
  assert(linha("pino_rele_amarelo = 99", o) == 1 && cfg.pinReleAmarelo == PAD_PIN_RELE_AMARELO);
  assert(linha("pino_rele_amarelo = abc", o) == 1 && cfg.pinReleAmarelo == PAD_PIN_RELE_AMARELO);
  assert(linha("rele_ativo_em = MEIO", o) == 1 && cfg.releAtivoEm == LOW);
  assert(linha("corr_ruim_a = nan", o) == 1 && cfg.corrRuimA == 3.5f);
  assert(linha("sem igual", o) == 1);
  zerar();
  assert(texto("pino_rele_verde = 30\r\nlixo = 1\r\ntemp_ruim_c = 70", o) == 1);
  assert(cfg.pinReleVerde == 30 && cfg.tempRuimC == 70);
  zerar();
  std::string longa(CFG_LINHA_MAX + 20, 'a');
  assert(texto(("# " + longa + "\n").c_str(), o) == 0);
  assert(texto((longa + "\ntemp_ruim_c = 70\n").c_str(), o) == 1 && cfg.tempRuimC == 70);

  zerar(); assert(configValidar(o) == 0 && igual(cfg, padrao));
  zerar(); cfg.pinReleAmarelo = cfg.pinReleVerde = 30; cfg.pinTcrt = 3;
  assert(configValidar(o) == 1 && igual(cfg, padrao));
  zerar(); cfg.pinTcrt = 5;
  assert(configValidar(o) == 1 && cfg.pinTcrt == PAD_PIN_TCRT);
  zerar(); cfg.pinTcrt = 20;
  assert(configValidar(o) == 1 && cfg.pinTcrt == PAD_PIN_TCRT);
  zerar(); cfg.pinMaxCs = PIN_SD_CS;
  assert(configValidar(o) == 1 && cfg.pinMaxCs == PAD_PIN_MAX_CS);
  zerar(); cfg.enderecoAdxl = cfg.enderecoLcd;
  assert(configValidar(o) == 1 && igual(cfg, padrao));
  zerar(); cfg.corrRuimA = 5.0f;
  assert(configValidar(o) == 1 && cfg.corrRuimA == padrao.corrRuimA);
  zerar(); cfg.tempRuimC = 120;
  assert(configValidar(o) == 1 && cfg.tempRuimC == 80);
  zerar(); cfg.vibMultRuim = 4.0f;
  assert(configValidar(o) == 1 && cfg.vibMultRuim == 2.0f);
  zerar(); cfg.rpmGraveMin = 1700;
  assert(configValidar(o) == 1 && cfg.rpmGraveMin == 1400);
  zerar(); cfg.adxlFmaxHz = 200;
  assert(configValidar(o) == 1 && cfg.adxlFmaxHz == 180);
  { Saida a; zerar(); cfg.adxlFminHz = 40;
    assert(configValidar(a) >= 1 && a.s.find("ATENCAO") != std::string::npos); }
  { Saida a; zerar(); cfg.pulsoMinUs = 20000;
    assert(configValidar(a) == 1 && a.s.find("TCRT") != std::string::npos && cfg.pulsoMinUs == 20000); }
  { Saida a; zerar(); cfg.rpmTimeoutMs = 150;
    assert(configValidar(a) == 1 && a.s.find("timeout") != std::string::npos); }

  zerar();
  const char *motor = "motor_rpm_nominal = 3600\nrpm_tolerancia = 280\nrpm_grave_min = 2800\n"
                      "rpm_grave_max = 4000\nmotor_corrente_nominal_a = 5\ncorr_ruim_a = 5.7\ncorr_grave_a = 6.9\n"
                      "tcrt_pulso_min_us = 5000\n";
  assert(texto(motor, o) == 0 && configValidar(o) == 0);
  LimitesAvaliacao L = limitesDeConfig();
  assert(L.setpointRPM == 3600 && L.rpmGraveMin == 2800 && L.corrGraveA == 6.9f);
  int e;
  LeituraAtual ok = {25, 5.0f, 3600, 0, false, false, 10000, 0};
  assert(avaliarEstado(ok, L, e) == ANDON_BOM);
  LeituraAtual rpmBaixo = {25, 5.0f, 3200, 0, false, false, 10000, 0};
  assert(avaliarEstado(rpmBaixo, L, e) == ANDON_DEFEITO && e == 1);
  LeituraAtual rpmPadrao = {25, 5.0f, 1740, 0, false, false, 10000, 0};
  assert(avaliarEstado(rpmPadrao, L, e) == ANDON_GRAVE);
  LeituraAtual corr = {25, 6.0f, 3600, 0, false, false, 10000, 0};
  assert(avaliarEstado(corr, L, e) == ANDON_DEFEITO);
  zerar(); linha("motor_rpm_nominal = 3600", o);
  assert(configValidar(o) >= 1 && cfg.rpmNominal == 1740);

  zerar(); uint16_t a0 = configAssinaturaVib();
  cfg.rpmNominal = 3600; assert(configAssinaturaVib() != a0);
  zerar(); cfg.adxlFminHz = 7;  assert(configAssinaturaVib() != a0);
  zerar(); cfg.adxlBandaBins = 3; assert(configAssinaturaVib() != a0);
  zerar(); cfg.reducao = 10;    assert(configAssinaturaVib() != a0);
  zerar(); cfg.corrRuimA = 3.5f; assert(configAssinaturaVib() == a0);

  std::string ex = ler("exemplo/CONFIG.TXT");
  assert(!ex.empty());
  assert(ex == exemplo());
  assert(ler("docs/CONFIG.md") == markdown());
  zerar();
  Saida ex_o;
  assert(texto(ex.c_str(), ex_o) == 0 && ex_o.s.empty() && igual(cfg, padrao));
  assert(configValidar(ex_o) == 0);

  puts("OK");
  return 0;
}
