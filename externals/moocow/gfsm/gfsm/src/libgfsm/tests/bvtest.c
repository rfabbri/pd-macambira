#include <gfsm.h>

int main (int argc, char **argv) {
  int i;
  guint bit;
  gfsmBitVector *bv = gfsm_bitvector_new();

  for (i = 0; i <= 16 ; i++) {
    printf("\t%d bits ~= %d bytes @ %d\n",
	   i, _gfsm_bitvector_bits2bytes(i), i%8);
  }

  for (i = 1; i < argc; i++) {
    sscanf(argv[i], "%u", &bit);
    printf("%s: setting bit=%u : bit2byte=%u\n", *argv, bit, _gfsm_bitvector_bits2bytes(bit));
    gfsm_bitvector_set(bv,bit,1);
  }

  printf("%s: vector [bytes=%u ; bits=%u] =\n",
	 *argv, bv->len, gfsm_bitvector_size(bv));
  for (bit = 0; bit < gfsm_bitvector_size(bv); bit++) {
    if ((bit%8)==0) fputc(' ', stdout);
    fputc((gfsm_bitvector_get(bv,bit) ? '1' : '0'), stdout);
  }
  printf("\n");

  gfsm_bitvector_free(bv);

  return 0;
}
