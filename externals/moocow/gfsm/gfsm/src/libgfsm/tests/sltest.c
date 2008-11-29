#include <glib.h>
#include <stdio.h>

int main (void) {
  GSList *sl=NULL;
  GAllocator *myalloc=NULL;

  g_mem_set_vtable(glib_mem_profiler_table);

  //-- allocator hack
  myalloc = g_allocator_new("myAllocator", 128);
  g_slist_push_allocator(myalloc);

  sl = g_slist_prepend(NULL,(gpointer)2);
  g_slist_free(sl);

  //-- allocator hack
  g_slist_pop_allocator();
  g_allocator_free(myalloc);

  g_blow_chunks();
  g_mem_profile();
  
  return 0;
}
