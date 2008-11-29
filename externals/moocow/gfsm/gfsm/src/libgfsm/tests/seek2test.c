#include <gfsm.h>
#include <stdio.h>
#include <stdlib.h>
#include "labprobs.h"

/*======================================================================
 * Globals
 */
const char *prog = "seek2test";

const char *fsmfile   = "tagh-chopped.gfst";
//const char *xfsmfile  = "tagh-lo.gfstx";

gfsmStateId qid_test = 0;
guint       out_degree_test = 0;
gulong count_test = 
//1024
//1048576
4194304
//16777216
;

//#define BENCH_SORTED 1
#undef BENCH_SORTED

/*======================================================================
 * bench_seek_vanilla()
 */
double bench_seek_vanilla(gfsmAutomaton *fsm) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(gfsm_automaton_out_degree(fsm,qid_test));
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = qid_test;
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcIter ai;
    ary->len=0;
    for (gfsm_arciter_open(&ai,fsm,qid), gfsm_arciter_seek_lower(&ai,lab);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,lab))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	if (fsm->flags.sort_mode==gfsmASMLower && a->lower!=lab) break;
	g_ptr_array_add(ary, a);
      }
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * bench_seek_labx()
 */
#ifndef SEEK_LABX_BINSEARCH_CUTOFF
#define SEEK_LABX_BINSEARCH_CUTOFF 0
//#define SEEK_LABX_BINSEARCH_CUTOFF 4
//#define SEEK_LABX_BINSEARCH_CUTOFF 8
//#define SEEK_LABX_BINSEARCH_CUTOFF 16
//#define SEEK_LABX_BINSEARCH_CUTOFF 32
//#define SEEK_LABX_BINSEARCH_CUTOFF 64
//#define SEEK_LABX_BINSEARCH_CUTOFF 128
//#define SEEK_LABX_BINSEARCH_CUTOFF 256
#endif

double bench_seek_labx(gfsmArcLabelIndex *labx) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(gfsm_arc_label_index_out_degree(labx,qid_test));
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = qid_test;
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcRange range;
    gfsmArc *a;
    ary->len=0;
    gfsm_arcrange_open_label_index(&range,labx,qid);
    if ((range.max - range.min) <= SEEK_LABX_BINSEARCH_CUTOFF) {
      for ( ; gfsm_arcrange_ok(&range); gfsm_arcrange_next(&range) ) {
	a = gfsm_arcrange_arc(&range);
	if (a->lower<lab) continue;
	if (a->lower>lab) break;
	g_ptr_array_add(ary, a);
      }
    } else {
      for (gfsm_arcrange_seek_lower(&range,lab); gfsm_arcrange_ok(&range); gfsm_arcrange_next(&range)) {
	a = gfsm_arcrange_arc(&range);
	if (a->lower!=lab) break;
	g_ptr_array_add(ary, a);
      }
    }
    //gfsm_arcrange_close(&range);
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
  fprintf(stderr, "%s: qid=%u, out_degree=%u\n", prog, qid_test, out_degree_test);
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
  fprintf(stderr, "BENCH[%16s]: %ld iters in %.2g sec: %.2g iters/sec\n",
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
int main(int argc, char **argv)
{
  char *qid_str="0";
  gfsmError *err=NULL;
  int argi;
  //
  gfsmAutomaton *fsm=NULL;
  double elapsed_vanilla;
#ifdef BENCH_SORTED
  gfsmAutomaton *fsm_sorted=NULL;
  double elapsed_sorted;
#endif
  gfsmArcLabelIndex *labx=NULL;
  double elapsed_labx;

  //-- sanity check
  if (argc < 2) {
    fprintf(stderr, "Usage: %s [QID(s)...]\n", prog);
    exit(1);
  }

  //-- load probabilities & initialize
  load_label_probs();
  populate_seek_labels();

  //-- report
  fprintf(stderr, "%s: count=%lu\n", prog, count_test);
  fflush(stderr);

  //-- load/create: vanilla
  fprintf(stderr, "%s: loading vanilla automaton '%s'... ", prog, fsmfile); fflush(stderr);
  fsm = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fsm,fsmfile,&err)) {
    fprintf(stderr,"%s: load failed for '%s': %s\n", prog, fsmfile, (err ? err->message : "?"));
    exit(3);
  }
  fsm->flags.sort_mode = gfsmASMNone;
  fprintf(stderr, "loaded.\n"); fflush(stderr);

#ifdef BENCH_SORTED
  //-- load/create: sorted
  fprintf(stderr, "%s: sorting... ", prog); fflush(stderr);
  fsm_sorted = gfsm_automaton_clone(fsm);
  gfsm_automaton_arcsort(fsm_sorted,gfsmASMLower);
  fprintf(stderr, "sorted.\n"); fflush(stderr);
#endif

  //-- load/create: labx
  fprintf(stderr, "%s: indexing to gfsmArcLabelIndex... ", prog); fflush(stderr);
  labx = gfsm_automaton_to_arc_label_index_lower(fsm,NULL);
  fprintf(stderr, "indexed.\n"); fflush(stderr);

  //-- main loop
  for (argi=1; argi < argc; argi++) {
    qid_str  = argv[argi];
    qid_test = strtol(qid_str,NULL,0);
    out_degree_test = gfsm_automaton_out_degree(fsm,qid_test);

    report_new_row();

    //-- benchmark: vanilla
    elapsed_vanilla = bench_seek_vanilla(fsm);
    report_column("vanilla", elapsed_vanilla);

#ifdef BENCH_SORTED
    //-- benchmark: vanilla+sorted
    gfsm_automaton_arcsort(fsm,gfsmASMLower);
    elapsed_sorted  = bench_seek_vanilla(fsm);
    report_column("sorted", elapsed_sorted);
#endif

    //-- benchmark: indexed
    elapsed_labx = bench_seek_labx(labx);
    report_column("labx", elapsed_labx);

    report_end_row();
  }

  //-- gnuplot output
  report_gnuplot();

  //-- cleanup
  if (fsm) gfsm_automaton_free(fsm);
#ifdef BENCH_SORTED
  if (fsm_sorted) gfsm_automaton_free(fsm_sorted);
#endif
  if (labx) gfsm_arc_label_index_free(labx);

  return 0;
}
