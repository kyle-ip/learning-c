#include <stdio.h>
#include <stdarg.h>

void Printlnf(const char *format, ...) {
  // 读取变长参数。
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

// "Hello ""world" ==> "Hello world"
// __FILE__       文件名
// __LINE__       行号
// __FUNCTION__   函数


// #define PRINTLNF(format, ...) printf(format"\n", ##__VA_ARGS__)

// (../05.printlnf.c:20) main :
#define PRINTLNF(format, ...) printf("("__FILE__":%d) %s : "format"\n",__LINE__, __FUNCTION__, ##__VA_ARGS__)

#define PRINT_INT(value) PRINTLNF(#value": %d", value)

int main() {

  int value = 2;
  Printlnf("Hello World! %d", value);
  PRINTLNF("Hello World! %d", value);
  PRINTLNF("Hello World!");

  // value: 2
  PRINT_INT(value);
  int x = 3;
  PRINT_INT(x);

  // 3 + 4: 7
  PRINT_INT(3 + 4);
  return 0;
}
