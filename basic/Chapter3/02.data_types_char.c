#include <stdio.h>
#include <limits.h>

int main() {
  // 字符集 ASCII 127
  char a = 'a';           // 97
  char char_1 = '1';      // 49
  char char_0 = '0';      // 48

  char i = 0;             // \0, NULL

  // 字面量 literal
  // \n : newline
  // \b : backspace
  // \r : return
  // \t : table
  // \' : ' 字符字面量
  // \" : " 字符串字面量

  // 八进制、十六进制
  char char_1_escape_oct = '\61';
  char char_1_escape_hex = '\x31';
  char newline = '\n';

  printf("char a: %d\n", a);
  printf("char 1: %d\n", char_1);
  printf("char 'i': %d\n", i);

  printf("char 1: %c\n", char_1);
  printf("char 1: %c\n", char_1_escape_oct);
  printf("char 1: %c\n", char_1_escape_hex);

  // Unicode CJK Code point.
  // C95 新增宽字符
  wchar_t zhong = L'中';
  wchar_t zhong_hex = L'\u4E2D';
  printf("中: %d\n", zhong);
  printf("中: %d\n", zhong_hex);

  // 字符串
  char *string = "中";
  return 0;
}
