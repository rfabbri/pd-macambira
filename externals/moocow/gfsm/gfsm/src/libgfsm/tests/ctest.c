#include <stdio.h>

typedef struct testme_s {
  int i : 1;
  int j : 1;
  int k : 1;
} testme_t;

int main (void) {
  int x,y,z;
  float f = +inf;
  printf("sizeof(testme_t)=%u\n", sizeof(testme_t));

  z = (x=42,y=24,17);
  printf ("z = (x=42,y=24) = %d\n", z);
  printf("f = %g\n", f);
  return 0;
}
