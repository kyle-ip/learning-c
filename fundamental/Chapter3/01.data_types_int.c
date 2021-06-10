#include <stdio.h>
#include <limits.h>

int main() {
  // C 标准只限定 short 不低于 16 位，int 大于 short、long 大于 int。
  short short_int = 0;
  int i = 100;
  long long_int = 0;

  long long longlong_int = 0;

  unsigned int unsigned_int = 123;

  unsigned long unsigned_long = 111;

  printf("short int %d\n", short_int);
  printf("int in oct: %o\n", i);

  // d = decimal
  // x = hex
  // o = oct
  // hd%: short decimal
  // %d: decimal
  // %ld: long decimal
  // %lld: long long decimal
  // %hu: unsigned short decimal
  // \n: new line
  // size_t

  // 整型别名，在不同平台上类型不同。
  size_t size_of_int = sizeof(int);
  printf("short int: %llu\n", sizeof(short int));
  printf("int: %llu\n", sizeof(int));
  printf("long int: %llu\n", sizeof(long int));
  printf("long long: %llu\n", sizeof(long long int));

  printf("max int %d, min %d\n", INT_MAX, INT_MIN);
  printf("max int %ld, min %ld\n", LONG_MAX, LONG_MIN);
  printf("unsigned max int %u, unsigned min %d\n", UINT_MAX, 0);
  return 0;
}
