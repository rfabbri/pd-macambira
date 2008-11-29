#include <stdio.h>

int main (void) {
  int x = 0;

  x = 1, printf("foo\n"), printf("bar\n"), x=2, printf("%d\n", x);
  return 0;
}
