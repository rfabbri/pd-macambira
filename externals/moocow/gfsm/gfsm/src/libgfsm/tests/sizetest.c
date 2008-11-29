#include <stdio.h>

typedef struct {
  int x1 : 1;
  int x2 : 1;
  int x3 : 30;
} tstruc;

int main (void) {
  int i;
  tstruc ts;

  printf("sizeof(int)=%ld ; sizeof(tstruc)=%ld\n", sizeof(int), sizeof(tstruc));
  printf("sizeof(float)=%ld, sizeof(void*)=%ld\n", sizeof(float), sizeof(void*));
  return 0;
}
