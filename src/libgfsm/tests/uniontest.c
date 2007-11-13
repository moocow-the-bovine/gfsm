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


//-- try this
typedef enum {
  fooList,
  fooArray
} fooType;

typedef struct {
  fooType type;
} fooBaseT;

typedef struct {
  int begin;
  int end;
} intPairT;

typedef struct intListNode_ {
  int val;
  struct intListNode_ *next;
} intListNode;

typedef struct {
  fooBaseT base;
  union {
    intListNode *l;
    intPairT     a;
  } list;
  union {
    intListNode *l;
    intPairT    *a;
  } cur;
} FooT;


//--- BAR

typedef struct {
  fooType type;
  union {
    struct {
      intListNode *first;
      intListNode *cur;
    } l;
    struct {
      int *begin;
      int *end;
      int *cur;
    } a;
  } u;
} BarT;


/*======================================================================
 * MAIN
 */
int main(void) {
  PtrPairIntU ppi;
  FooT        foo;
  memset(&ppi,0,sizeof(ppi));
  memset(&foo,0,sizeof(foo));

  printf("zero: pp.a=%p=%d, pp.b=%p=%d, i.i=%p=%x\n",
	 ppi.pp.a, (int)ppi.pp.a, ppi.pp.b, (int)ppi.pp.b, (void*)ppi.i.i, ppi.i.i);

  ppi.pp.a = &(ppi.pp.a);
  ppi.pp.b = &(ppi.pp.b);
  printf("a=&a,b=&b: pp.a=%p=%d, pp.b=%p=%d, i.i=%p=%d\n",
	 ppi.pp.a, (int)ppi.pp.a, ppi.pp.b, (int)ppi.pp.b, (void*)ppi.i.i, ppi.i.i);

  ppi.i.i = 420;
  printf("i=420: pp.a=%p=%d, pp.b=%p=%d, i.i=%p=%d\n",
	 ppi.pp.a, (int)ppi.pp.a, ppi.pp.b, (int)ppi.pp.b, (void*)ppi.i.i, ppi.i.i);

  printf("\n---\n");
  {
    intListNode nod  = { 42, NULL };
    intPairT    pair = { 24, 7 };

    foo.base.type = fooList;
    foo.list.l    = &nod;
    foo.cur.l     = &nod;
    printf("typ=list=%d, list.l=cur.l=&nod=%p:\n", fooList, &nod);
    printf("--> typ=%d, list.l=%p, cur.l=%p, cur.l->val=%d, list.a.begin=%d, list.a.end=%d, cur.a=%p\n",
	   foo.base.type, foo.list.l, foo.cur.l, foo.cur.l->val,
	   foo.list.a.begin, foo.list.a.end, foo.cur.a);


    foo.base.type = fooArray;
    foo.list.a    = pair;
    foo.cur.a     = &pair;
    printf("typ=list=%d, list.a={24,7}, cur.a=%p:\n", fooList, &pair);
    printf("--> typ=%d, list.l=%p, cur.l=%p, cur.l->val=%d, list.a.begin=%d, list.a.end=%d, cur.a=%p ~ {%d,%d}\n",
	   foo.base.type, foo.list.l, foo.cur.l, foo.cur.l->val,
	   foo.list.a.begin, foo.list.a.end, foo.cur.a, foo.cur.a->begin, foo.cur.a->end);

  }

  printf("\n---\n");
  {
    BarT bar;
    memset(&bar,0,sizeof(bar));

    printf("typ=%d, l={first=%p,cur=%p}, a={beg=%p,end=%p,cur=%p}\n",
           bar.type, bar.u.l.first, bar.u.l.cur, bar.u.a.begin, bar.u.a.end, bar.u.a.cur);
  }

  return 0;
}
