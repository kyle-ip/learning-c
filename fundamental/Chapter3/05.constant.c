#include <stdio.h>

#define COLOR_RED 0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE 0x0000FF

int main() {
  // const <type> readonly variable
  const int kRed = 0xFF0000;
  const int kGreen = 0x00FF00;
  const int kBlue = 0x0000FF;
  printf("kRed: %d\n", kRed);

  int *p_k_red = &kRed;
  *p_k_red = 0;
  printf("kRed: %d\n", kRed);

  // macro
  printf("COLOR_RED: %d\n", COLOR_RED);

#undef COLOR_RED

  // 字面量 literal
  3;
  3u;
  3l;
  3.f;
  3.9;
  'c';
  "cs";
  L'中';
  L"中国";

  // 硬编码 hard code
  int background_color = COLOR_GREEN;
  return 0;
}
