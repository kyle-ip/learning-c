#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  // 1. random number
  // [0, 0x7fff]
  srand(time(NULL));
  int magic_number = rand();
  // 2. loop
  while (1) {
    int user_input;
    puts("Please input a number: ");
    scanf("%d", &user_input);
    if (user_input > magic_number) {
      puts("Your number is bigger!");
    } else if (user_input < magic_number) {
      puts("Your number is smaller!");
    } else {
      puts("Yes! You got it!");
      break;
    }
  }
  return 0;
}
