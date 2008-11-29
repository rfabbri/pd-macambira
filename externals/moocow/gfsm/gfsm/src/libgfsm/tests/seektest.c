#include <gfsm.h>
#include <gfsmIndexed2.h>
#include <stdio.h>
#include <stdlib.h>
#include "labprobs.h"

/*======================================================================
 * Globals
 */
const char *prog = "seektest";

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
 * indexed_lower_lb()
 */
gfsmArcId indexed_lower_lb(gfsmIndexedAutomaton *fsm,
			   gfsmLabelId lab,
			   gfsmArcId aid_lo,
			   gfsmArcId aid_hi)

{
  /*
  gfsmArcId aid_mid;
  gfsmArc   *a;
  */

  //if (!gfsm_indexed_automaton_has_state(fsm,qid)) return gfsmNoArc;
  /*
  aid_lo = g_array_index(fsm->state_first_arc, gfsmArcId, qid);
  aid_hi = g_array_index(fsm->state_first_arc, gfsmArcId, qid+1);
  */

  while (aid_lo < aid_hi) {
    gfsmArcId aid_mid = (aid_lo+aid_hi)/2;
    gfsmArc        *a = &g_array_index(fsm->arcs, gfsmArc, g_array_index(fsm->arcix_lower, gfsmArcId, aid_mid));
    if (a->lower < lab) { aid_lo = aid_mid+1; }
    else                { aid_hi = aid_mid; }
  }
  //return aid_lo <= aid_hi ? aid_lo : gfsmNoArc;
  return aid_lo;
}


/*======================================================================
 * bench_seek_indexed()
 */
#ifndef SEEK_INDEXED_BINSEARCH_CUTOFF
//#define SEEK_INDEXED_BINSEARCH_CUTOFF 0
//#define SEEK_INDEXED_BINSEARCH_CUTOFF 4
//#define SEEK_INDEXED_BINSEARCH_CUTOFF 8
//#define SEEK_INDEXED_BINSEARCH_CUTOFF 16
//#define SEEK_INDEXED_BINSEARCH_CUTOFF 32
#define SEEK_INDEXED_BINSEARCH_CUTOFF 64
#endif
double bench_seek_indexed(gfsmIndexedAutomaton *fsm) {
#if 1
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(gfsm_indexed_automaton_out_degree(fsm,qid_test));
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = qid_test;
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcId aid_lo = g_array_index(fsm->state_first_arc, gfsmArcId, qid);
    gfsmArcId aid_hi = g_array_index(fsm->state_first_arc, gfsmArcId, qid+1);
    gfsmArcId aid;
    gfsmArc *a;
    ary->len=0;
    if (aid_hi-aid_lo >= SEEK_INDEXED_BINSEARCH_CUTOFF) {
      for (aid=indexed_lower_lb(fsm,lab,aid_lo,aid_hi); aid<aid_hi; aid++) {
	a = &g_array_index(fsm->arcs, gfsmArc, g_array_index(fsm->arcix_lower, gfsmArcId, aid));
	if (a->lower!=lab) break;
	g_ptr_array_add(ary, a);
      }
    } else {
      for (a=((gfsmArc*)fsm->arcs->data)+aid_lo; a < ((gfsmArc*)fsm->arcs->data)+aid_hi; a++) {
	if (a->lower==lab) g_ptr_array_add(ary,a);
      }
    }
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
#else
  return 1e38; //-- dummy
#endif
}


/*======================================================================
 * indexed2_lower_lb()
 */
gfsmArcId indexed2_lower_lb(gfsmIndexedAutomaton2 *fsm,
			    gfsmLabelId lab,
			    gfsmArcId aid_lo,
			    gfsmArcId aid_hi)

{
  while (aid_lo < aid_hi) {
    gfsmArcId aid_mid = (aid_lo+aid_hi)/2;
    gfsmArc        *a = g_ptr_array_index(fsm->arcix_lower, aid_mid);
    if (a->lower < lab) { aid_lo = aid_mid+1; }
    else                { aid_hi = aid_mid; }
  }
  //return aid_lo <= aid_hi ? aid_lo : gfsmNoArc;
  return aid_lo;
}


/*======================================================================
 * bench_seek_indexed2()
 */
double bench_seek_indexed2(gfsmIndexedAutomaton2 *fsm) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(gfsm_indexed_automaton2_out_degree(fsm,qid_test));
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid  = qid_test;
    gfsmLabelId lab  = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcId aid_lo = g_array_index(fsm->state_first_arc, gfsmArcId, qid);
    gfsmArcId aid_hi = g_array_index(fsm->state_first_arc, gfsmArcId, qid+1);
    gfsmArcId aid;
    gfsmArc **app;
    ary->len=0;
    if (aid_hi-aid_lo >= SEEK_INDEXED_BINSEARCH_CUTOFF) {
      for (app = ((gfsmArc**)(fsm->arcix_lower->pdata)) + indexed2_lower_lb(fsm,lab,aid_lo,aid_hi);
	   app < ((gfsmArc**)(fsm->arcix_lower->pdata)) + aid_hi && (*app)->lower==lab;
	   app++)
	{
	  g_ptr_array_add(ary, (*app));
	}
    } else {
      for (app = ((gfsmArc**)(fsm->arcix_lower->pdata)) + aid_lo;
	   app < ((gfsmArc**)(fsm->arcix_lower->pdata)) + aid_hi;
	   app++)
	{
	  if ((*app)->lower==lab) g_ptr_array_add(ary,(*app));
	}
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
  gfsmIndexedAutomaton *xfsm=NULL;
  double elapsed_indexed;
  gfsmIndexedAutomaton2 *xfsm2=NULL;
  double elapsed_indexed2;

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

  //-- load/create: indexed
  fprintf(stderr, "%s: indexing... ", prog); fflush(stderr);
  xfsm = gfsm_automaton_to_indexed(fsm,NULL);
  fprintf(stderr, "indexed.\n"); fflush(stderr);

  //-- load/create: indexed2
  fprintf(stderr, "%s: indexing(2)... ", prog); fflush(stderr);
  xfsm2 = gfsm_automaton_to_indexed2(fsm,NULL);
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
    elapsed_indexed = bench_seek_indexed(xfsm);
    report_column("indexed", elapsed_indexed);

    //-- benchmark: indexed2
    elapsed_indexed2 = bench_seek_indexed2(xfsm2);
    report_column("indexed2", elapsed_indexed2);

    report_end_row();
  }

  //-- gnuplot output
  report_gnuplot();

  //-- cleanup
  if (fsm) gfsm_automaton_free(fsm);
#ifdef BENCH_SORTED
  if (fsm_sorted) gfsm_automaton_free(fsm_sorted);
#endif
  if (xfsm) gfsm_indexed_automaton_free(xfsm);

  return 0;
}
