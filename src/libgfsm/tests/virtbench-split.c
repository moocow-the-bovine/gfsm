#include "virtbench-split.h"

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

IntUnaryFunc vtable[3] = { NULL, test1_vtable_global_func, test2_vtable_global_func };
TestT   t = {1,42};
VTestT vt = {42,test1_vtable_local_func};


/*======================================================================
 * dobench
 */
#define dobench(fnc) \
  elapsed = bench_ ## fnc (); \
  elapsed = bench_ ## fnc (); \
  fprintf(stderr, "  %-16s: %4.2f sec: %6.2f M iter/sec: %6.2f %%\n", \
	  # fnc, elapsed, (count_dbl/elapsed/1e6), (100.0*elapsed0/elapsed));

/*======================================================================
 * Tests: literal code
 */
double bench_literal_code(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test1_code(&(t.val));
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}

/*======================================================================
 * Tests: literal inline
 */
double bench_literal_inline(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test1_literal_inline(&t.val);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}

/*======================================================================
 * Tests: literal func
 */
double bench_literal_func(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test1_literal_func(&t.val);
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
  case 1: return test1_code(&(t->val));
  case 2: return test2_code(&(t->val));
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
 * Tests: switch + func (inline)
 */
int test_switch_inline(TestT *t) {
  switch (t->typ) {
  case 1: return test1_switch_inline(&(t->val));
  case 2: return test2_switch_inline(&(t->val));
  }
  return t->val;
}

double bench_switch_inline(void) {
  double elapsed;
  GTimer *timer  = g_timer_new();
  gulong i;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test_switch_inline(&t);
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
  case 1: return test1_switch_func(&(t->val));
  case 2: return test2_switch_func(&(t->val));
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
  return (*(vtable[t->typ]))(&(t->val));
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
int test_vtable_local(VTestT *vt) {
  return (*(vt->func))(&(vt->val));
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
int main(int argc, char **argv)
{
  double elapsed0, elapsed;
  double count_dbl = count_test;
  int i0 = 42;
  if (argc > 1) i0 = strtol(argv[1],NULL,0);

  t.val=i0;
  vt.val=i0;
  
  printf("count_test=%ld; i0=%d\n", count_test, i0);

  elapsed0 = bench_literal_code();
  dobench(literal_code);

  dobench(literal_inline);
  dobench(literal_func);
  dobench(switch_code);
  dobench(switch_inline);
  dobench(switch_func);
  dobench(vtable_global);
  dobench(vtable_local);

  return 0;
}
