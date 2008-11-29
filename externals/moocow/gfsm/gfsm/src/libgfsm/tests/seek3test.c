#include <gfsm.h>
#include <stdio.h>
#include <stdlib.h>

/*======================================================================
 * Globals
 */
const char *prog = "seek2test";

gfsmStateId qid_test = 0;
guint  out_degree_test;
gulong count_test_max = 
//1024UL     //==2^10
//1048576UL  //==2^20
2097152UL  //==2^21
//4194304UL  //==2^22
//16777216UL //==2^24
//67108864UL //==2^26
//268435456UL //==2^28
;
gulong count_test=0; //-- count_test_max/out_degree

/*======================================================================
 * Label population
 */

//--------------------------------------------------------------
// globals
GRand *grand = NULL;
const guint32 grand_seed = 42;
#define GRAND_USE_SEED 1
//#undef GRAND_USE_SEED

const guint32 n_labels = 128;
const guint32 n_states = 8192;

GArray *seekus   = NULL; /*-- lab = g_array_index(seekus,i); 1<=i<=count_test --*/
GArray *seekfrom = NULL; /*-- qid = g_array_index(seekus,i); 1<=i<=count_test --*/

//--------------------------------------------------------------
// random_label()
gfsmLabelId random_label(void) {
  if (!grand) {
    grand = g_rand_new();
#ifdef GRAND_USE_SEED
    g_rand_set_seed(grand,grand_seed);
#endif
  }
  return g_rand_int_range(grand,0,n_labels);
}

//--------------------------------------------------------------
// populate_seek_labels()
void populate_seek_labels(void) {
  int i;
  gfsmLabelId lab;
  seekus = g_array_sized_new(FALSE,TRUE,sizeof(gfsmLabelId),count_test_max);
  for (i=0; i < count_test_max; i++) {
    lab = random_label();
    g_array_append_val(seekus,lab);
  }
}

//--------------------------------------------------------------
// random_state()
gfsmStateId random_state(void) {
  if (!grand) { grand = g_rand_new_with_seed(grand_seed); }
  return g_rand_int_range(grand,0,n_states);
}

//--------------------------------------------------------------
// populate_seek_states()
void populate_seek_states(void) {
  int i;
  seekfrom = g_array_sized_new(FALSE,TRUE,sizeof(gfsmStateId),count_test_max);
  for (i=0; i < count_test_max; i++) {
    gfsmStateId qid = random_state();
    g_array_append_val(seekfrom,qid);
  }
}


/*======================================================================
 * bench_seek_vanilla()
 */
double bench_seek_vanilla(gfsmAutomaton *fsm) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId, i);
    gfsmLabelId lab = g_array_index(seekus,  gfsmLabelId, i);
    gfsmArcIter ai;
    ary->len=0;
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      if (a->lower != lab) continue;
      g_ptr_array_add(ary, a);
    }
    gfsm_arciter_close(&ai);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_sorted()
 */
double bench_seek_sorted(gfsmAutomaton *fsm) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId,i);
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcIter ai;
    ary->len=0;
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      if (a->lower < lab) continue;
      if (a->lower > lab) break;
      g_ptr_array_add(ary, a);
    }
    gfsm_arciter_close(&ai);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_tabx_vanilla()
 */
double bench_seek_tabx_vanilla(gfsmArcTableIndex *tabx) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId,i);
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcRange range;
    ary->len=0;
    for (gfsm_arcrange_open_table_index(&range,tabx,qid); gfsm_arcrange_ok(&range); gfsm_arcrange_next(&range)) {
      gfsmArc *a = gfsm_arcrange_arc(&range);
      if (a->lower != lab) continue;
      g_ptr_array_add(ary, a);
    }
    gfsm_arcrange_close(&range);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_tabx_sorted() : linear search
 */
double bench_seek_tabx_sorted(gfsmArcTableIndex *tabx) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId,i);
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcRange range;
    ary->len=0;
    for (gfsm_arcrange_open_table_index(&range,tabx,qid); gfsm_arcrange_ok(&range); gfsm_arcrange_next(&range)) {
      gfsmArc *a = gfsm_arcrange_arc(&range);
      if (a->lower < lab) continue;
      if (a->lower > lab) break;
      g_ptr_array_add(ary, a);
    }
    gfsm_arcrange_close(&range);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_tabx_sorted_2() : linear search (v2) [identical to v1]
 */
double bench_seek_tabx_sorted_2(gfsmArcTableIndex *tabx) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId,i);
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcRange range;
    ary->len=0;
    for (gfsm_arcrange_open_table_index(&range,tabx,qid); range.min<range.max; ++range.min) {
      if (range.min->lower < lab) continue;
      if (range.min->lower > lab) break;
      g_ptr_array_add(ary, range.min);
    }
    gfsm_arcrange_close(&range);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_tabx_seek_lib() : binary search: library function
 */
inline void gfsm_arcrange_seek_lower(gfsmArcRange *range, gfsmLabelId find)
{
  g_assert(range != NULL);
  while (gfsm_arcrange_ok(range) && gfsm_arcrange_arc(range)->lower < find)
    gfsm_arcrange_next(range);
}

double bench_seek_tabx_seek_lib(gfsmArcTableIndex *tabx) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId,i);
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcRange range;
    ary->len=0;
    for (gfsm_arcrange_open_table_index(&range,tabx,qid), gfsm_arcrange_seek_lower(&range,lab);
	 gfsm_arcrange_ok(&range);
	 gfsm_arcrange_next(&range))
      {
	gfsmArc *a = gfsm_arcrange_arc(&range);
	if (a->lower > lab) break;
	g_ptr_array_add(ary, a);
      }
    gfsm_arcrange_close(&range);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_tabx_bsearch_inl() : binary search: inline function
 */
#define BSEARCH_CUTOFF 16
static inline gfsmArc *bsearch_lower(gfsmArc *min, gfsmArc *max, gfsmLabelId find)
{
  while (min < max) {
    gint diff = max-min;
    if (diff < BSEARCH_CUTOFF) {
      do {
	if (min->lower >= find) break;
	min++;
      } while (min < max);
      return min;
    }
    else {
      gfsmArc *mid = min + diff/2;
      if (mid->lower < find) min = mid+1;
      else                   max = mid;
    }
  }
  return min;
}

double bench_seek_tabx_bsearch_inl(gfsmArcTableIndex *tabx) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId,i);
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcRange range;
    ary->len=0;
    for (gfsm_arcrange_open_table_index(&range,tabx,qid), range.min=bsearch_lower(range.min,range.max,lab);
	 gfsm_arcrange_ok(&range);
	 gfsm_arcrange_next(&range))
      {
	gfsmArc *a = gfsm_arcrange_arc(&range);
	if (a->lower > lab) break;
	g_ptr_array_add(ary, a);
      }
    gfsm_arcrange_close(&range);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_tabx_bsearch_func() : binary search: inline function
 */
static void bsearch_range_func(gfsmArcRange *range, gfsmLabelId find)
{
  gfsmArc *min=range->min, *max=range->max;
  while (min < max) {
    gfsmArc *mid = min + (max-min)/2;
    if (mid->lower < find) min = mid+1;
    else                   max = mid;
  }
  range->min = min;
}

double bench_seek_tabx_bsearch_func(gfsmArcTableIndex *tabx) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(out_degree_test);
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = g_array_index(seekfrom,gfsmStateId,i);
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcRange range;
    ary->len=0;
    for (gfsm_arcrange_open_table_index(&range,tabx,qid), bsearch_range_func(&range,lab);
	 gfsm_arcrange_ok(&range);
	 gfsm_arcrange_next(&range))
      {
	gfsmArc *a = gfsm_arcrange_arc(&range);
	if (a->lower > lab) break;
	g_ptr_array_add(ary, a);
      }
    gfsm_arcrange_close(&range);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}


/*======================================================================
 * Report
 */
GString *dat_header=NULL;
GString *dat_data=NULL;
gint     dat_row=0;
gint     dat_col=0;

void report_new_row(void) {
  fprintf(stderr, "%s: n_states=%u, n_labels=%u, out_degree=%u\n", prog, n_states, n_labels, out_degree_test);
  //
  //-- save data for gnuplot output
  dat_col=0;
  if (!dat_header) dat_header = g_string_new("");
  if (!dat_data)   dat_data   = g_string_new("");
  if (dat_row==0) {
    g_string_append(dat_header,"#1:out_deg");
  }
  g_string_append_printf(dat_data, "%u", out_degree_test);
}

void report_column(char *label, double elapsed) {
  double iters_per_sec = ((double)count_test)/elapsed;
  //
  //-- to stderr
  fprintf(stderr, "BENCH[%24s]: %ld iters in %5.3f sec: %7.2e iters/sec\n",
	  label, count_test, elapsed, iters_per_sec);
  fflush(stderr);
  //
  //-- to data strings
  if (dat_row==0) {
    g_string_append_printf(dat_header, "\t%d:%s_secs\t%d:%s_ips", (2*dat_col+2),label, (2*dat_col+3),label);
  }
  g_string_append_c(dat_data,'\t');
  g_string_append_printf(dat_data,"\t%g\t%g", elapsed,iters_per_sec);
  ++dat_col;
}

void report_end_row(void) {
  ++dat_row;
  g_string_append(dat_data,"\n");
}

void report_gnuplot(void) {
  fflush(stderr);
  printf("%s\n%s", dat_header->str, dat_data->str);
}


/*======================================================================
 * Main
 */
//#define BENCH_VANILLA       1
//#define BENCH_SORTED        1
//#define BENCH_TABX_VANILLA  1
#define BENCH_TABX_SORTED   1
//#define BENCH_TABX_SORTED_2 1
//#define BENCH_TABX_SEEK_LIB  1
#define BENCH_TABX_BSEARCH_FUNC  1
//#define BENCH_TABX_BSEARCH_INL  1
int main(int argc, char **argv)
{
  char *out_degree_str="32";
  int argi, arci, qi;
  //
  gfsmAutomaton *fsm=NULL;
  double elapsed_vanilla=0;
  //
  gfsmAutomaton *fsm_sorted=NULL;
  double elapsed_sorted=0;
  //
  gfsmArcTableIndex *tabx=NULL;
  double elapsed_tabx_vanilla=0;
  //
  gfsmArcTableIndex *tabx_sorted=NULL;
  double elapsed_tabx_sorted=0;
  //
  gfsmArcTableIndex *tabx_sorted_2=NULL;
  double elapsed_tabx_sorted_2=0;
  //
  gfsmArcTableIndex *tabx_seek_lib=NULL;
  double elapsed_tabx_seek_lib=0;
  //
  gfsmArcTableIndex *tabx_bsearch_func=NULL;
  double elapsed_tabx_bsearch_func=0;
  //
  gfsmArcTableIndex *tabx_bsearch_inl=NULL;
  double elapsed_tabx_bsearch_inl=0;

  //-- sanity check
  if (argc < 2) {
    fprintf(stderr, "Usage: %s [OUT_DEGREE(s)...]\n", prog);
    exit(1);
  }

  //-- initialize labels to seek
  populate_seek_labels();
  populate_seek_states();

  //-- report
  fprintf(stderr, "%s: count_test_max=%lu\n", prog, count_test_max);
  fflush(stderr);

  //-- create: vanilla
  fsm = gfsm_automaton_new();

  //-- main loop
  for (argi=1; argi < argc; argi++) {
    out_degree_str  = argv[argi];
    out_degree_test = strtol(out_degree_str,NULL,0);
    //count_test      = count_test_max / out_degree_test;
    count_test = count_test_max;

    //-- populate: vanilla
    gfsm_automaton_clear(fsm);
    gfsm_automaton_set_root(fsm,gfsm_automaton_ensure_state(fsm,0));
    gfsm_automaton_set_final_state_full(fsm,0,TRUE,fsm->sr->one);
    for (qi=1; qi < n_states; qi++) {
      gfsm_automaton_ensure_state(fsm,qi);
      for (arci=0; arci < out_degree_test; arci++) {
	gfsmLabelId lo = random_label();
	gfsmLabelId hi = random_label();
	gfsmWeight   w = arci + 1.0;
	gfsm_automaton_add_arc(fsm,qi,qi, lo,hi, w);
      }
    }

    //-------- bench
    report_new_row();

    //-- benchmark: vanilla (twice for cache optimization)
#ifdef BENCH_VANILLA
    elapsed_vanilla = bench_seek_vanilla(fsm);
    elapsed_vanilla = bench_seek_vanilla(fsm);
    report_column("vanilla", elapsed_vanilla);
#endif

#ifdef BENCH_SORTED
    //-- benchmark: vanilla+sorted
    fsm_sorted = gfsm_automaton_clone(fsm);
    gfsm_automaton_arcsort(fsm_sorted,gfsmASMLower);
    elapsed_sorted  = bench_seek_sorted(fsm_sorted);
    elapsed_sorted  = bench_seek_sorted(fsm_sorted);
    report_column("sorted", elapsed_sorted);
#endif

#ifdef BENCH_TABX_VANILLA
    //-- benchmark: table: vanilla
    tabx = gfsm_automaton_to_arc_table_index(fsm,tabx);
    elapsed_tabx_vanilla = bench_seek_tabx_vanilla(tabx);
    elapsed_tabx_vanilla = bench_seek_tabx_vanilla(tabx);
    report_column("tabx_vanilla", elapsed_tabx_vanilla);
#endif

#ifdef BENCH_TABX_SORTED
    //-- benchmark: table: sorted linear
    tabx_sorted = gfsm_automaton_to_arc_table_index(fsm,tabx_sorted);
    gfsm_arc_table_index_priority_sort(tabx_sorted, gfsmASP_LU, fsm->sr);
    elapsed_tabx_sorted = bench_seek_tabx_sorted(tabx_sorted);
    elapsed_tabx_sorted = bench_seek_tabx_sorted(tabx_sorted);
    report_column("tabx_sorted", elapsed_tabx_sorted);
#endif

#ifdef BENCH_TABX_SORTED_2
    //-- benchmark: table: sorted linear (v2)
    tabx_sorted_2 = gfsm_automaton_to_arc_table_index(fsm,tabx_sorted_2);
    gfsm_arc_table_index_priority_sort(tabx_sorted_2, gfsmASP_LU, fsm->sr);
    elapsed_tabx_sorted_2 = bench_seek_tabx_sorted_2(tabx_sorted_2);
    elapsed_tabx_sorted_2 = bench_seek_tabx_sorted_2(tabx_sorted_2);
    report_column("tabx_sorted_2", elapsed_tabx_sorted_2);
#endif

#ifdef BENCH_TABX_SEEK_LIB
    //-- benchmark: table: binary search: lib
    tabx_seek_lib = gfsm_automaton_to_arc_table_index(fsm,tabx_seek_lib);
    gfsm_arc_table_index_priority_sort(tabx_seek_lib, gfsmASP_LU, fsm->sr);
    elapsed_tabx_seek_lib = bench_seek_tabx_seek_lib(tabx_seek_lib);
    elapsed_tabx_seek_lib = bench_seek_tabx_seek_lib(tabx_seek_lib);
    report_column("tabx_seek_lib", elapsed_tabx_seek_lib);
#endif

#ifdef BENCH_TABX_BSEARCH_FUNC
    //-- benchmark: table: binary search: func
    tabx_bsearch_func = gfsm_automaton_to_arc_table_index(fsm,tabx_bsearch_func);
    gfsm_arc_table_index_priority_sort(tabx_bsearch_func, gfsmASP_LU, fsm->sr);
    elapsed_tabx_bsearch_func = bench_seek_tabx_bsearch_func(tabx_bsearch_func);
    elapsed_tabx_bsearch_func = bench_seek_tabx_bsearch_func(tabx_bsearch_func);
    report_column("tabx_bsearch_func", elapsed_tabx_bsearch_func);
#endif

#ifdef BENCH_TABX_BSEARCH_INL
    //-- benchmark: table: binary search: inline
    tabx_bsearch_inl = gfsm_automaton_to_arc_table_index(fsm,tabx_bsearch_inl);
    gfsm_arc_table_index_priority_sort(tabx_bsearch_inl, gfsmASP_LU, fsm->sr);
    elapsed_tabx_bsearch_inl = bench_seek_tabx_bsearch_inl(tabx_bsearch_inl);
    elapsed_tabx_bsearch_inl = bench_seek_tabx_bsearch_inl(tabx_bsearch_inl);
    report_column("tabx_bsearch_inl", elapsed_tabx_bsearch_inl);
#endif

    report_end_row();
  }

  //-- gnuplot output
  report_gnuplot();

  //-- cleanup
  if (fsm)         gfsm_automaton_free(fsm);
  if (fsm_sorted)  gfsm_automaton_free(fsm_sorted);
  if (tabx)        gfsm_arc_table_index_free(tabx);
  if (tabx_sorted) gfsm_arc_table_index_free(tabx_sorted);
  if (tabx_sorted_2) gfsm_arc_table_index_free(tabx_sorted_2);

  return 0;
}
