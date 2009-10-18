#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

typedef long long uint64;

uint64 gf_timeofday () {
	timeval t;
	gettimeofday(&t,0);
	return t.tv_sec*1000000+t.tv_usec;
}
static void test1 (size_t n) {
	uint64 t = gf_timeofday();
	for (int i=0; i<10000; i++) free(malloc(n));
	t = gf_timeofday() - t;
	printf("10000 mallocs of %7ld bytes takes %7ld us (%f us/malloc)\n",n,(long)t,t/(float)10000);
}
static void test2 (size_t n) {
	uint64 t = gf_timeofday();
	// the real calloc is lazy, let's try a manual (strict) calloc
	//for (int i=0; i<10000; i++) free(calloc(1,n));
	for (int i=0; i<10000; i++) {
		long *p = (long *)malloc(n);
		size_t nn=n/sizeof(long);
		for (size_t j=0; j<nn; j++) p[j] = 0;
		free(p);
	}
	t = gf_timeofday() - t;
	printf("10000 callocs of %7ld bytes takes %7ld us (%f us/calloc)\n",n,(long)t,t/(float)10000);
}
int main () {
	for (int i=0; i<20; i++) {test1(4<<i); test2(4<<i);}
	//for (int i=0; i<20; i++) test1(4096*(32+i));
	return 0;
}
