#include <stdio.h>
#include <limits.h>

int main() {
  float a_float = 3.14f;    // 小数位数 6, 7~8，表示范围 +-10^-37 ~ 10^37

  double a_double = 3.14;   // 15~16

  printf("size of float: %llu\n", sizeof(float));
  printf("size of double: %llu\n", sizeof(double));

  float lat = 39.90815f;
  printf("%f", 39.908156f - lat);

  float money = 3.14f; // error, never use float or double to describe money
  return 0;
}
