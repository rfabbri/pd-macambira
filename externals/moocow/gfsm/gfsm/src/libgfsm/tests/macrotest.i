#define _gfsm_bitvector_bits2bytes(nbits) ((nbits)>0 ? ((((nbits)-1)/8)+1) : 1)

#define gfsm_bitvector_set(bv,i,v) \
  ( (i >= gfsm_bitvector_size(bv) ? gfsm_bitvector_resize(bv,i) : 0), \
    (v ? ( (bv)->data[ _gfsm_bitvector_bits2bytes(i)-1 ] |= (1<<((i)%8)) ) \
       : ( (bv)->data[ _gfsm_bitvector_bits2bytes(i)-1 ] &= ~(1<<((i)%8)) ) ) )

bits2bytes: _gfsm_bitvector_bits2bytes(MyBit)

set: gfsm_bitvector_set(MyVector,MyBit,MyValue)

