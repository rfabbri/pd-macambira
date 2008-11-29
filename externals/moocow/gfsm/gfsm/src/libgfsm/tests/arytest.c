#include <glib.h>
#include <stdio.h>

int main (void) {
  GPtrArray *a;
  gpointer p;
  g_mem_set_vtable(glib_mem_profiler_table);

  printf("<PROF:1>--------\n");


  a = g_ptr_array_sized_new(128);
  g_ptr_array_set_size(a,1024);
  p = g_ptr_array_free(a,TRUE);
  printf("p=ptr_array_free()=%p\n", p);

  printf("<CHUNKS:1>--------\n");
  //g_mem_chunk_info();

  //printf("<CHUNKS:2>--------\n");
  g_blow_chunks();
  //g_mem_chunk_info();

  printf("<PROF:2>--------\n");
  g_mem_profile();
  return 0;
}
