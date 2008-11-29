#include <gfsm.h>
#include <glib.h>

#define VTABLE_PROFILE 1
//#define VTABLE_LOCAL 1


//#define USE_ALLOCATORS 1
//#define DELAY_ALLOCATOR_FREE 1


//#define NITEMS 0
//#define NITEMS 1
//#define NITEMS 10
#define NITEMS 128
//#define NITEMS 256
//#define NITEMS 1024
//#define NITEMS 65536
//#define NITEMS 131072
//#define NITEMS 262144
//#define NITEMS 524288
//#define NITEMS 1048576


//#define NITERS 
//#define NITERS 1
//#define NITERS 10
#define NITERS 128
//#define NITERS 1024
//#define NITERS 65536
//#define NITERS 131072
//#define NITERS 262144
//#define NITERS 524288
//#define NITERS 1048576

//#define PRINT_CHUNK_INFO 1
//#define DO_PROFILE 1

#define DO_GMALLOC 1
#define GMALLOC_SIZE 45

#define DO_GNEW 1
#define GNEW_SIZE 70

#define DO_SLIST 1

#define DO_PTRARRAY 1
#define PTRARRAY_SIZE 128


/*--------------------------------------------------------------------
 * mem table
 */
gpointer my_malloc(gsize n_bytes)
{ return (gpointer)malloc(n_bytes); }

gpointer my_realloc(gpointer mem, gsize n_bytes)
{ return (gpointer)realloc(mem, n_bytes); }

void my_free(gpointer mem)
{ free(mem); }

GMemVTable my_vtable = 
  {
    my_malloc,
    my_realloc,
    my_free,
    NULL,
    NULL,
    NULL
  };

/*--------------------------------------------------------------------
 * operation macro
 */
#define MEMOP(code) \
  printf("%s\n", #code); \
  code;

#define ITEMOP(code) \
  for (i=0; i<NITEMS; i++) { code; }

/*--------------------------------------------------------------------
 * variables
 */
gpointer mallocp[NITERS];
gpointer newp[NITERS];
GSList *slist[NITERS];
GPtrArray *ptrarray[NITERS];

/*--------------------------------------------------------------------
 * MAIN
 */
int main(int argc, char **argv) {
  int i,j;


  //-- memory debugging
#if defined(VTABLE_PROFILE)
  g_mem_set_vtable(glib_mem_profiler_table);
#elif defined(VTABLE_LOCAL)
  g_mem_set_vtable(&my_vtable);
#endif

  //-- setup gfsm allocators
#if defined(USE_ALLOCATORS)
  MEMOP(gfsm_allocators_enable());
#endif


  //--------------- iteration
  for (j=0; j < NITERS; j++) {
    //-- alloc
#  ifdef DO_GMALLOC
    ITEMOP(mallocp[i]=g_malloc(GMALLOC_SIZE));
#  endif
#  ifdef DO_GNEW
    ITEMOP(newp[i]=g_new(char,GNEW_SIZE));
#  endif
#  ifdef DO_SLIST
    ITEMOP(slist[i]=g_slist_prepend(NULL,NULL));
#  endif
#  ifdef DO_PTRARRAY
    ITEMOP(ptrarray[i]=g_ptr_array_sized_new(PTRARRAY_SIZE));
#  endif

    //-- free
#  ifdef DO_GMALLOC
    ITEMOP(g_free(mallocp[i]));
#  endif
#  ifdef DO_GNEW
    ITEMOP(g_free(newp[i]));
#  endif
#  ifdef DO_SLIST
    ITEMOP(g_slist_free(slist[i]));
#  endif
#  ifdef DO_PTRARRAY
    ITEMOP(g_ptr_array_free(ptrarray[i],TRUE));
#  endif
  }


  //-- pop gfsm allocators
#if defined(USE_ALLOCATORS) && !defined(DELAY_ALLOCATOR_FREE)
  MEMOP(gfsm_allocators_free());
#endif

  //-- memory debugging
#if defined(PRINT_CHUNK_INFO) && defined(VTABLE_PROFILE)
  printf("\n<CHUNKS:1>--------\n");
  g_blow_chunks();
  g_mem_chunk_info();
#endif
  //
#ifdef VTABLE_PROFILE
  printf("\n<PROF:1>--------\n");
  g_blow_chunks();
  g_mem_profile();
#endif

  g_blow_chunks();

#if defined(USE_ALLOCATORS) && defined(DELAY_ALLOCATOR_FREE)
  MEMOP(gfsm_allocators_free());
#endif

  return 0;
}
