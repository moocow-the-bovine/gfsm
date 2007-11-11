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
const char *prog = "seektest";

gulong count_test = 
//1024
//1048576  //==2**20
//4194304  //==2**22
//16777216 //==2**24
67108864   //==2**26
;





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

  elapsed = bench_switch_code();
  fprintf(stderr, "switch_code: %.2f sec: %.2g iter/sec\n", elapsed, count_dbl/elapsed);

  elapsed = bench_switch_func();
  fprintf(stderr, "switch_func: %.2f sec: %.2g iter/sec\n", elapsed, count_dbl/elapsed);

  elapsed = bench_vtable_global();
  fprintf(stderr, "vtable_global: %.2f sec: %.2g iter/sec\n", elapsed, count_dbl/elapsed);

  elapsed = bench_vtable_local();
  fprintf(stderr, "vtable_local: %.2f sec: %.2g iter/sec\n", elapsed, count_dbl/elapsed);

  return 0;
}
