#include <stdio.h>
int main(void) {
  int x=0, y=-1;
  int z = x||y;
  printf("x||y=%d\n", z);
  return 0;
}
