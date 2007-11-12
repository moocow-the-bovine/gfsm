#include <stdio.h>
#include <string.h>

typedef struct {
  void *a;
  void *b;
} PtrPairT;

typedef struct {
  int i;
} IntT;

typedef union {
  PtrPairT pp;
  IntT      i;
} PtrPairIntU;


int main(void) {
  PtrPairIntU ppi;
  memset(&ppi,0,sizeof(ppi));

  printf("zero: pp.a=%p=%d, pp.b=%p=%d, i.i=%p=%x\n",
	 ppi.pp.a, (int)ppi.pp.a, ppi.pp.b, (int)ppi.pp.b, (void*)ppi.i.i, ppi.i.i);

  ppi.pp.a = &(ppi.pp.a);
  ppi.pp.b = &(ppi.pp.b);
  printf("a=&a,b=&b: pp.a=%p=%d, pp.b=%p=%d, i.i=%p=%d\n",
	 ppi.pp.a, (int)ppi.pp.a, ppi.pp.b, (int)ppi.pp.b, (void*)ppi.i.i, ppi.i.i);

  ppi.i.i = 420;
  printf("i=420: pp.a=%p=%d, pp.b=%p=%d, i.i=%p=%d\n",
	 ppi.pp.a, (int)ppi.pp.a, ppi.pp.b, (int)ppi.pp.b, (void*)ppi.i.i, ppi.i.i);

  return 0;
}
