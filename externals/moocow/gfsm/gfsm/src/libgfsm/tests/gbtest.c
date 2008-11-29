#include <glib.h>
#include <stdio.h>

GArray *ary1;
GArray *ary2;
GArray *ary3;
guint   esize = 8;
guint   nelts = 128;

int main (void) {
  int i;

  ary1 = g_array_sized_new(FALSE, TRUE, esize, nelts);
  ary2 = g_array_sized_new(FALSE, TRUE, esize, nelts);
  ary3 = g_array_sized_new(FALSE, TRUE, esize, nelts);

  for (i=0; i < 128; i++) {
    g_array_free(ary2,TRUE);
    ary2 = g_array_sized_new(FALSE,TRUE,esize,nelts*i);
  }

  g_array_free(ary1,TRUE);
  g_array_free(ary2,TRUE);
  g_array_free(ary3,TRUE);
  
  return 0;
}
