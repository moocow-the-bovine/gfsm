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


//#define test1_code(ip) (*(ip))=rand()+*(ip)

#define test1_code(ip) *(ip)=*(ip)*7867
#define test2_code(ip) test1_code(ip)

int test1_literal_func(int *ip) { test1_code(ip); return (*ip); }
int test2_literal_func(int *ip) { test2_code(ip); return (*ip); }

int test1_switch_func(int *ip) { test1_code(ip); return (*ip); }
int test2_switch_func(int *ip) { test2_code(ip); return (*ip); }

int test1_vtable_global_func(int *ip) { test1_code(ip); return (*ip); }
int test2_vtable_global_func(int *ip) { test2_code(ip); return (*ip); }

int test1_vtable_local_func(int *ip) { test1_code(ip); return (*ip); }
int test2_vtable_local_func(int *ip) { test2_code(ip); return (*ip); }

typedef int (*IntUnaryFunc)(int *ip);
IntUnaryFunc vtable[3] = { NULL, test1_vtable_global_func, test2_vtable_global_func };

typedef struct {
  int val;
  IntUnaryFunc func;
} VTestT;

TestT   t = {1,42};
VTestT vt = {42,test1_vtable_local_func};

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
 * dobench
 */
#define dobench(fnc) \
  elapsed = bench_ ## fnc (); \
  elapsed = bench_ ## fnc (); \
  fprintf(stderr, "%16s: %4.2f sec: %6.2f M iter/sec: %6.2f %%\n", \
	  # fnc, elapsed, (count_dbl/elapsed/1e6), (100.0*elapsed0/elapsed));

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

  dobench(literal_func);
  dobench(switch_code);
  dobench(switch_func);
  dobench(vtable_global);
  dobench(vtable_local);

  /*-- DATA: uhura, CFLAGS="-O3 -pipe":
count_test=268435456; i0=42
    literal_code: 0.44 sec: 611.73 M iter/sec:  99.63 %
    literal_func: 0.44 sec: 613.33 M iter/sec:  99.89 %
     switch_code: 0.44 sec: 612.72 M iter/sec:  99.79 %
     switch_func: 0.44 sec: 612.94 M iter/sec:  99.83 %
   vtable_global: 1.74 sec: 154.67 M iter/sec:  25.19 %
    vtable_local: 1.30 sec: 206.38 M iter/sec:  33.61 %
  */

  /*-- DATA: uhura, CFLAGS="-O3 -pipe -fno-inline"
count_test=268435456; i0=42
    literal_code: 0.44 sec: 613.77 M iter/sec:  99.94 %
    literal_func: 1.27 sec: 211.41 M iter/sec:  34.42 %
     switch_code: 1.15 sec: 232.51 M iter/sec:  37.86 %
     switch_func: 1.83 sec: 146.33 M iter/sec:  23.83 %
   vtable_global: 1.74 sec: 154.67 M iter/sec:  25.18 %
    vtable_local: 1.32 sec: 203.74 M iter/sec:  33.17 %
   */


  return 0;
}
