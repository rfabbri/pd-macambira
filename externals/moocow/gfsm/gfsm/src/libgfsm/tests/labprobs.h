#include <gfsm.h>

guint32 grand_seed = 42;
const char *probfile = "tagh-probs.bin";

extern gulong count_test;
GArray *seekus   = NULL; /*-- lab = g_array_index(seekus,i); 1<=i<=count_test --*/

typedef struct {
  gfsmLabelId lab;
  double      prob;
} seekProb;
GArray  *labp=NULL;  /*-- g_array_index(probs,seekProb,i) = (lab,p(lab)) --*/

GRand *grand=NULL;

/*======================================================================
 * load_label_probs()
 */
void load_label_probs(void) {
  seekProb sp;
  double   total=0, tmp;
  FILE *f = fopen(probfile,"r");
  if (!f) {
    fprintf(stderr, "error: open failed for probability file '%s'\n", probfile);
    exit(1);
  }
  labp = g_array_sized_new(FALSE,TRUE,sizeof(seekProb),256);
  labp->len = 0;
  while ( !feof(f) ) {
    if (fread(&(sp.lab),  sizeof(gfsmLabelId), 1, f) != 1
	|| fread(&(sp.prob), sizeof(double), 1, f) != 1)
      {
	break;
      }
    tmp      = sp.prob;
    sp.prob += total;
    total   += tmp;
    g_array_append_val(labp,sp);

  }
  fclose(f);
  fprintf(stderr, "[info]: read probability distribution over %d labels from '%s'\n",
	  labp->len, probfile);
}

/*======================================================================
 * random_label()
 */
gfsmLabelId random_label(void) {
  double w;
  int i;
  if (!grand) { grand = g_rand_new_with_seed(grand_seed); }
  w = g_rand_double(grand);
  for (i=0; i < labp->len && w > g_array_index(labp,seekProb,i).prob; i++) { ; }
  if (i==labp->len) { --i; }
  return g_array_index(labp,seekProb,i).lab;
}

/*======================================================================
 * populate_seek_labels()
 */
void populate_seek_labels(void) {
  int i;
  gfsmLabelId lab;
  seekus = g_array_sized_new(FALSE,TRUE,sizeof(gfsmLabelId),count_test);
  for (i=0; i < count_test; i++) {
    lab = random_label();
    g_array_append_val(seekus,lab);
  }
}
