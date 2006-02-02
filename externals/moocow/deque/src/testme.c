#include <stdio.h>

#define FOO 42
#define BAR(x) x + FOO

int main (void) {
  printf("BAR(1) = %d\n", BAR(1));
  return 0;
}

