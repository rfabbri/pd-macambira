#include <glib.h>
#include <stdio.h>

typedef struct {
  guint32 i1 : 1;
  guint32 i2 : 31;
} testme;

int main (void) {

  printf("guint32:%u ; testme=%u\n", sizeof(guint32), sizeof(testme));

  return 0;
}
