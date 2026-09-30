#pragma once
// Nachbildung der ESPHome-Display-API für den Vergleichstest auf dem PC.
// Jeder Zeichenbefehl wird als Textzeile protokolliert; Textbreiten sind deterministisch.
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace esphome {

struct Color {
  int v;
  constexpr Color(int value = 0) : v(value) {}
  constexpr Color(int r, int, int, int) : v(r) {}  // Graustufen: protokolliert als c85 / c170
};
inline std::string to_string(int v) { return std::to_string(v); }
inline std::string to_string(unsigned v) { return std::to_string(v); }
inline std::string to_string(long v) { return std::to_string(v); }
inline std::string to_string(unsigned long v) { return std::to_string(v); }

namespace display {
enum class TextAlign { BASELINE_LEFT, BASELINE_RIGHT, BASELINE_CENTER, CENTER };
static const Color COLOR_ON{1};
static const Color COLOR_OFF{0};

class BaseFont {
 public:
  const char *name;
  int size;
};

inline int utf8_len(const char *s) {
  int n = 0;
  for (; *s; s++)
    if ((*s & 0xC0) != 0x80) n++;
  return n;
}

class Display {
 public:
  std::vector<std::string> log;
  void rec(const char *fmt, ...) {
    char b[512];
    va_list a;
    va_start(a, fmt);
    vsnprintf(b, sizeof b, fmt, a);
    va_end(a);
    log.emplace_back(b);
  }
  void print(int x, int y, BaseFont *f, Color c, TextAlign al, const char *t) {
    rec("print %d,%d %s c%d a%d [%s]", x, y, f->name, c.v, (int) al, t);
  }
  void print(int x, int y, BaseFont *f, TextAlign al, const char *t) { print(x, y, f, COLOR_ON, al, t); }
  void printf(int x, int y, BaseFont *f, TextAlign al, const char *fmt, ...) {
    char b[512];
    va_list a;
    va_start(a, fmt);
    vsnprintf(b, sizeof b, fmt, a);
    va_end(a);
    print(x, y, f, COLOR_ON, al, b);
  }
  void printf(int x, int y, BaseFont *f, Color c, TextAlign al, const char *fmt, ...) {
    char b[512];
    va_list a;
    va_start(a, fmt);
    vsnprintf(b, sizeof b, fmt, a);
    va_end(a);
    print(x, y, f, c, al, b);
  }
  void line(int x1, int y1, int x2, int y2, Color c = COLOR_ON) { rec("line %d,%d %d,%d c%d", x1, y1, x2, y2, c.v); }
  void rectangle(int x, int y, int w, int h, Color c = COLOR_ON) { rec("rect %d,%d %dx%d c%d", x, y, w, h, c.v); }
  void filled_rectangle(int x, int y, int w, int h, Color c = COLOR_ON) {
    rec("frect %d,%d %dx%d c%d", x, y, w, h, c.v);
  }
  void filled_triangle(int a, int b, int c, int d, int e, int f, Color col = COLOR_ON) {
    rec("ftri %d,%d %d,%d %d,%d c%d", a, b, c, d, e, f, col.v);
  }
  void get_text_bounds(int x, int y, const char *t, BaseFont *f, TextAlign, int *x1, int *y1, int *w, int *h) {
    *x1 = x;
    *y1 = y;
    *w = utf8_len(t) * f->size * 6 / 10;
    *h = f->size;
  }
};
}  // namespace display

using namespace display;
}  // namespace esphome

// Uhrzeit wie esphome::ESPTime (nur die genutzten Felder)
struct MockTime {
  bool valid = true;
  int year = 2026, month = 9, day_of_month = 30, day_of_week = 4, hour = 10, minute = 5, second = 0;
  bool is_valid() const { return valid; }
};
