#include <stdio.h>

// 直接在代码中进行（文本）替换，而不是先求值、再传参，因此要用括号以支持表达式。
// 大量使用会使代码大幅度增长。
// 只能使用无副作用的表达式（i++ 这种就有副作用）。
// 参数无类型，可当作重载使用。
#define MAX(a, b) (a) > (b) ? (a) : (b)

#define IS_HEX_CHARACTER(ch) \
((ch) >= '0' && (ch) <= '9') || \
((ch) >= 'A' && (ch) <= 'F') || \
((ch) >= 'a' && (ch) <= 'f')



int Max(int a, int b) {
  return a > b ? a : b;
}

int main() {
  int max = MAX(1.0, 3);
  int max2 = MAX(1, MAX(3, 4));

  int max3 = Max(1, Max(3, 4));

  int max4 = MAX(max++, 5);

  printf("max2: %d\n", max2);

  printf("is A a hex character? %d\n", IS_HEX_CHARACTER('A'));
  return 0;
}
