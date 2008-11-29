#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

void print_array(const char *label, GArray *a) {
  int i;
  printf("Array %s: {", label);
  for (i=0; i < a->len; i++) {
    printf(" %d", g_array_index(a,int,i));
  }
  printf(" }\n");
}

gint compare_int(int *x, int *y) {
  return (*x)-(*y);
  //return (*x < *y ? -1 : (*x > *y ? 1 : 0));
}

int main (int argc, char **argv) {
  GArray *a1, *a2;
  int i, val;

  a1 = g_array_new(FALSE, TRUE, sizeof(int));
  a2 = g_array_new(FALSE, TRUE, sizeof(int));
  for (i=1; i < argc; i++) {
    val = strtol(argv[i],NULL,10);
    g_array_append_val(a1,val);
  }
  print_array("a1", a1);

  //-- insert(0,0)
  val = 0;
  g_array_insert_val(a1,0,val);
  printf("--\n");
  print_array("a1/insert(0,0)",a1);

  //-- copy a1 into a2
  g_array_append_vals(a2,a1->data,a1->len);
  print_array("a2", a2);
  printf("--\n");

  //-- move elements of a1 up one notch
  /*-- NOT ok
  g_array_insert_vals(a1, 1, a1->data, a1->len-1);
  g_array_index(a1,int,0) = 0;
  */

  /*-- ok */
  i = 0;
  g_array_insert_val(a1,0,i);
  print_array("a1/moved", a1);
  printf("--\n");

  //-- sort
  //g_array_sort(a1,NULL); //-- NOT ok
  g_array_sort(a1,(GCompareFunc)compare_int); //-- ok
  print_array("a1/sorted", a1);

  g_array_free(a1,TRUE);
  g_array_free(a2,TRUE);

  return 0;
}
