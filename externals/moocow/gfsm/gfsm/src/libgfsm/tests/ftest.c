#include <stdio.h>
#include <glib.h>

int main (int argc, char **argv) {
  float f, f2;
  gpointer p;
  int i;

  for (i=1; i<argc; i++) {
    sscanf(argv[i], "%f", &f);
    p  = (gpointer)(*((int*)(&f)));
    f2 = *((float*)(&p));
    printf("argv[i=%d]='%s' ; f=%g ; f->p=%p ; p->f=%g\n", i, argv[i], f, p, f2);
  }

  return 0;
}
