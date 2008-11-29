#include <glib.h>
#include <gfsm.h>

int main (void)
{
  gfsmSet *set;
  GSList  *setl;
  GPtrArray  *setary;
  g_mem_set_vtable(glib_mem_profiler_table);

  set = gfsm_set_new(gfsm_uint_compare);
  gfsm_set_insert(set,(gpointer)2);

  //gfsm_set_clear(set);

  /*
  fprintf(stderr,"**** set=");
  gfsm_set_print_uint(set,stderr);
  fprintf(stderr,"\n");
  */
  //setl = gfsm_set_to_slist(set);
  //g_slist_free(setl);
  /*
  setl = g_slist_prepend(NULL,(gpointer)2);
  g_slist_free(setl);
  */
  setary = g_ptr_array_sized_new(gfsm_set_size(set));
  gfsm_set_to_ptr_array(set,setary);
  g_ptr_array_free(setary,TRUE);

  gfsm_set_free(set);

  g_blow_chunks();
  g_mem_profile();
  return 0;
}
