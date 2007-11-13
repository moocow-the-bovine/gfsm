#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

/*======================================================================
 * Types
 */
typedef struct {
  int typ;
  int val;
} TestT;


#define test1_code(i) (i)+1
#define test2_code(i) (i)+1
int test1(int i) { return test1_code(i); }
int test2(int i) { return test2_code(i); }

typedef int (*IntUnaryFunc)(int i);
IntUnaryFunc vtable[3] = { NULL, test1, test2 };

typedef struct {
  int val;
  IntUnaryFunc func;
} VTestT;

TestT   t = {1,42};
VTestT vt = {42,test1};

/*======================================================================
 * Globals
 */
const char *prog = "virtbench";

gulong count_test = 
//1024
//1048576  //==2**20
//4194304  //==2**22
//16777216 //==2**24
//67108864LU   //==2**26
268435456LU  //==2**28
//2147483647U   //==2**31
;



/*======================================================================
 * Tests: literal
 */
double bench_literal(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test1(t.val);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}


/*======================================================================
 * Tests: switch + code
 */
int test_switch_code(TestT *t) {
  switch (t->typ) {
  case 1: return test1_code(t->val);
  case 2: return test2_code(t->val);
  }
  return t->val;
}

double bench_switch_code(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test_switch_code(&t);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}

/*======================================================================
 * Tests: switch + func
 */
int test_switch_func(TestT *t) {
  switch (t->typ) {
  case 1: return test1(t->val);
  case 2: return test2(t->val);
  }
  return t->val;
}

double bench_switch_func(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test_switch_func(&t);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}

/*======================================================================
 * Tests: vtable (global)
 */
int test_vtable_global(TestT *t) {
  return (*(vtable[t->typ]))(t->val);
}

double bench_vtable_global(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test_vtable_global(&t);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}

/*======================================================================
 * Tests: vtable (local)
 */
int test_vtable_local(VTestT *t) {
  return (*(t->func))(t->val);
}

double bench_vtable_local(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test_vtable_local(&vt);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}



/*======================================================================
 * Main
 */
int main(void)
{
  double elapsed;
  double count_dbl = count_test;

  printf("count_test=%ld\n", count_test);

  elapsed = bench_literal();
  fprintf(stderr, "  %-16s: %4.2f sec: %6.2f M iter/sec\n", "literal", elapsed, count_dbl/elapsed/1e6);

  elapsed = bench_switch_code();
  fprintf(stderr, "  %-16s: %4.2f sec: %6.2f M iter/sec\n", "switch_code", elapsed, count_dbl/elapsed/1e6);

  elapsed = bench_switch_func();
  fprintf(stderr, "  %-16s: %4.2f sec: %6.2f M iter/sec\n", "switch_func", elapsed, count_dbl/elapsed/1e6);

  elapsed = bench_vtable_global();
  fprintf(stderr, "  %-16s: %4.2f sec: %6.2f M iter/sec\n", "vtable_global", elapsed, count_dbl/elapsed/1e6);

  elapsed = bench_vtable_local();
  fprintf(stderr, "  %-16s: %4.2f sec: %6.2f M iter/sec\n", "vtable_local", elapsed, count_dbl/elapsed/1e6);

  /* carrot, -g
count_test=268435456
         literal: 1.65 sec: 163.03 M iter/sec : 100.00%
     switch_code: 2.32 sec: 115.47 M iter/sec :  70.83%
    vtable_local: 3.15 sec:  85.28 M iter/sec :  52.31%
   vtable_global: 3.50 sec:  76.78 M iter/sec :  47.10%
     switch_func: 4.69 sec:  57.25 M iter/sec :  35.11%

     carrot, -O2:
count_test=268435456
         literal: 0.00 sec: 134217728.00 M iter/sec
     switch_code: 0.00 sec: 268435456.00 M iter/sec
     switch_func: 0.00 sec: 134217728.00 M iter/sec
   vtable_global: 2.60 sec: 103.38 M iter/sec
    vtable_local: 1.51 sec: 177.88 M iter/sec
  */

  return 0;
}
