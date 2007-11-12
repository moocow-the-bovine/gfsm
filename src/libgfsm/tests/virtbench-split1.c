#include "virtbench-split.h"

/*======================================================================
 * Tests: literal code
 */

/*======================================================================
 * Tests: literal func
 */

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

/*======================================================================
 * Tests: vtable (global)
 */
int test_vtable_global(TestT *t) {
  return (*(vtable[t->typ]))(&(t->val));
}

/*======================================================================
 * Tests: vtable (local)
 */
int test_vtable_local(VTestT *vt) {
  return (*(vt->func))(&(vt->val));
}
