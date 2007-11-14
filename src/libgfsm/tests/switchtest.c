#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#define DO_INLINE_DECL 1
#define DO_INLINE_DEFN 1

#ifdef DO_INLINE_DECL
# define INLINE_DECL __inline__
#else
# define INLINE_DECL
#endif

#ifdef DO_INLINE_DEFN
# define INLINE_DEFN __inline__
#else
# define INLINE_DEFN
#endif

/*======================================================================
 * Types
 */
typedef struct {
  int typ;
  int val;
} TestT;


//#define test1_code(ip) (*(ip))=rand()+*(ip)

#define test1_code(tp) tp->val=tp->val*7867
#define test2_code(tp) tp->val++

static INLINE_DECL void test_switch_base1(TestT *tp);
static INLINE_DEFN void test_switch_base1(TestT *tp) {
  switch (tp->typ) {
  case 1: test1_code(tp); break;
  case 2: test2_code(tp); break;
  default: break;
  }
}
static INLINE_DECL void test_switch_base2(TestT *tp);
static INLINE_DEFN void test_switch_base2(TestT *tp) {
  switch (tp->typ) {
  case 1: test2_code(tp); break;
  case 2: test1_code(tp); break;
  default: break;
  }
}

static INLINE_DECL void test_switch_embedded1(TestT *tp);
static INLINE_DEFN void test_switch_embedded1(TestT *tp) {
  switch (tp->typ) {
  case 1: test_switch_base1(tp); break;
  case 2: test_switch_base2(tp); break;
  default: break;
  }
}
static INLINE_DECL void test_switch_embedded2(TestT *tp);
static INLINE_DEFN void test_switch_embedded2(TestT *tp) {
  switch (tp->typ) {
  case 1: test_switch_base2(tp); break;
  case 2: test_switch_base1(tp); break;
  default: break;
  }
}

void test_switch_compile(TestT *tp) {
  switch (tp->typ) {
  case 1: {
    int i;
    for (i=0; i < tp->val; i++) { tp->val += i; }
    break;
  }
  case 2: {
    int i;
    for (i=1; i < tp->val; i++) { tp->val *= i; }
    break;
  }
  default:
    break;
  }
}

TestT t = {1,42};

/*======================================================================
 * Globals
 */
const char *prog = "switchtest";

gulong count_test = 
//1024
//1048576  //==2**20
//4194304  //==2**22
16777216 //==2**24
//67108864LU   //==2**26
//268435456LU  //==2**28
//2147483647U   //==2**31
;

/*======================================================================
 * Tests: literal code
 */
double bench_literal_code(void) {
  double elapsed;
  GTimer *timer = g_timer_new();
  gulong i;
  TestT  *tp = &t;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test1_code(tp);
    test2_code(tp);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}


/*======================================================================
 * Tests: switch: base
 */
double bench_switch_base(void) {
  double elapsed;
  GTimer *timer = g_timer_new();
  gulong i;
  TestT *tp = &t;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test_switch_base1(tp);
    test_switch_base2(tp);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}

/*======================================================================
 * Tests: switch: embedded
 */
double bench_switch_embedded(void) {
  double elapsed;
  GTimer *timer = g_timer_new();
  gulong i;
  TestT *tp = &t;

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    test_switch_embedded1(tp);
    test_switch_embedded2(tp);
  }
  elapsed = g_timer_elapsed(timer,NULL);

  g_timer_destroy(timer);
  return elapsed;
}

/*======================================================================
 * dobench
 */
#define dobench(fnc) \
  t.val   = i0; \
  elapsed = bench_ ## fnc (); \
  elapsed = bench_ ## fnc (); \
  fprintf(stderr, "  %-16s: %4.2f sec: %6.2f M iter/sec: %6.2f %%\n", \
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
  t.val =i0;
  
  printf("%s: count_test=%ld; i0=%d\n", *argv, count_test, i0);

  elapsed0 = bench_literal_code();
  dobench(literal_code);

  dobench(switch_base);
  dobench(switch_embedded);


  return 0;
}
