#include <stdio.h>

#define ptr2int(p)   ((int)(p))
#define int2ptr(i)   ((void*)(i))
#define ptr2float(p) (*((float*)(&(p))))
#define int2float(i) (*((float*)(&(i))))

#define float2int(f) (*((int*)(&(f))))
#define float2ptr(f) (*((void**)(&(f))))

int main(void) {
 float f = 42.24;
 int  fi;
 void *fp;
 float fif, fpf;

 printf("f=%f\n", f);
 fi = float2int(f);
 printf("->fi=%d\n", fi);

 fif = int2float(fi);
 printf("-->fif=%f\n", fif);

 fp = float2ptr(f);
 printf("->fp=%p\n", fp);

 fpf=ptr2float(fp);
 printf("-->fpf=%f\n", fpf);

 return 0;
}
