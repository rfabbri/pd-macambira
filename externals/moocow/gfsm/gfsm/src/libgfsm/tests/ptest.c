#include <glib.h>
#include <stdio.h>

typedef struct _xstruc {
  int x;
  int y;
} xstruc;

int main (void) {
  xstruc xs = {42,24};
  xstruc *xsp = &xs;
  void   *vp  = xsp;
  char *s = NULL;
  char *s2;

  printf ("&xs    =%p  ; xsp    =%p ; vp    =%p\n", &xs, xsp, vp);
  printf ("&xs.x  =%p  ; &xs.y  =%p\n", &xs.x, &xs.y);
  printf ("&xsp->x=%p  ; &xsp->y=%p\n", &xsp->x, &xsp->y);
  printf ("(vp)->x=%p  ; (vp)->y=%p\n", &((xstruc*)vp)->x, &((xstruc*)vp)->y);

  printf("\n");
  printf("s=%p ; s2=%p\n", s, g_strdup(s));

  return 0;
}
