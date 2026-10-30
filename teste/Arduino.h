#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
typedef const char __FlashStringHelper;
#define F(x) (x)
#define PROGMEM
#define memcpy_P memcpy
#define LOW 0
#define HIGH 1
#define HEX 16
#define A0 54
#define NOT_AN_INTERRUPT 255
#define noInterrupts()
#define interrupts()
#define digitalPinToInterrupt(p) ((p) == 2 || (p) == 3 || ((p) >= 18 && (p) <= 21) ? (p) : NOT_AN_INTERRUPT)

class Print {
 public:
  virtual size_t write(uint8_t) = 0;
  void print(const char *s)       { while (*s) write(*s++); }
  void print(char c)              { write(c); }
  void print(long v)              { char b[24]; snprintf(b, sizeof b, "%ld", v); print(b); }
  void print(unsigned long v, int base = 10) { char b[24]; snprintf(b, sizeof b, base == 16 ? "%lX" : "%lu", v); print(b); }
  void print(double v, int d = 2) { char b[40]; snprintf(b, sizeof b, "%.*f", d, v); print(b); }
  void println()                  { write('\n'); }
  void println(const char *s)     { print(s); println(); }
};
