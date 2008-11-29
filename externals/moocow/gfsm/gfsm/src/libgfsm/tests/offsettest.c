#include <gfsm.h>
#include <glib.h>
#include <stdio.h>

static const gulong count_test
//= 1048576UL //== 2^20
//= 16777216UL //== 2^24
//= 33554432UL //==2^25
= 268435456UL //==2^28
//= 4294967295UL //== 2^32-1
;

//======================================================================
// Basic bench subs

inline gfsmLabelId get_lower(gfsmArc *a) { return a->lower; }
inline gfsmLabelId get_upper(gfsmArc *a) { return a->upper; }
inline gfsmLabelId get_label_offset(gfsmArc *a, gint offset) {
  return *((gfsmLabelId*)G_STRUCT_MEMBER_P(&a,offset));
}

//======================================================================
// Bench: literal: lower

double bench_literal_lower(gfsmArc *a) {
  gfsmLabelId l;
  GTimer *timer = g_timer_new();
  gulong i;
  double elapsed;
  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    l = get_lower(a);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);
  g_timer_destroy(timer);
  return elapsed;
}

//======================================================================
// Bench: offset

double bench_offset(gfsmArc *a, gint lab_offset) {
  gfsmLabelId l;
  GTimer *timer = g_timer_new();
  gulong i;
  double elapsed;
  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    l = get_label_offset(a,lab_offset);
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);
  g_timer_destroy(timer);
  return elapsed;
}

//======================================================================
// Bench: offset: lower

#define bench_offset_lower(a) bench_offset((a),G_STRUCT_OFFSET(gfsmArc, lower))


//======================================================================
// MAIN
int main(int argc, char **argv) {
  gfsmArc a = {0,1,2,3,4.5};
  double elapsed_literal, elapsed_offset, count_dbl=count_test;

  printf("G_STRUCT_OFFSET(gfsmArc, lower)=%d: *()=%d\n",
	 G_STRUCT_OFFSET(gfsmArc, lower),
	 *((gfsmLabelId*)G_STRUCT_MEMBER_P(&a,G_STRUCT_OFFSET(gfsmArc, lower)))
	 );

  printf("G_STRUCT_OFFSET(gfsmArc, upper)=%d: *()=%d\n",
	 G_STRUCT_OFFSET(gfsmArc, upper),
	 *((gfsmLabelId*)G_STRUCT_MEMBER_P(&a,G_STRUCT_OFFSET(gfsmArc, upper)))
	 );

  printf("G_STRUCT_OFFSET(gfsmArc, weight)=%d: *()=%g\n",
	 G_STRUCT_OFFSET(gfsmArc, weight),
	 *((gfsmWeight*)G_STRUCT_MEMBER_P(&a,G_STRUCT_OFFSET(gfsmArc, weight)))
	 );

  //-- bench
  elapsed_literal = bench_literal_lower(&a);
  elapsed_literal = bench_literal_lower(&a);
  //
  elapsed_offset  = bench_offset_lower(&a);
  elapsed_offset  = bench_offset_lower(&a);
  //
  //
  fprintf(stderr, "%16s: %.2f sec, %ld iters, %.2e iter/sec\n",
	  "literal", elapsed_literal, count_test, count_dbl/elapsed_literal);
  fprintf(stderr, "%16s: %.2f sec, %ld iters, %.2e iter/sec\n",
	  "offset", elapsed_offset, count_test, count_dbl/elapsed_offset);

  return 0;
}
