#include "virtbench-split.h"

int test1_literal_func(int *ip) { return test1_literal_inline(ip); }
     //{ test1_code(ip); return (*ip); }
int test2_literal_func(int *ip) { return test2_literal_inline(ip); }
     //{ test2_code(ip); return (*ip); }

int test1_switch_func(int *ip) { return test1_literal_inline(ip); }
int test2_switch_func(int *ip) { return test2_literal_inline(ip); }

int test1_vtable_global_func(int *ip) { return test1_literal_inline(ip); }
int test2_vtable_global_func(int *ip) { return test2_literal_inline(ip); }

int test1_vtable_local_func(int *ip) { return test1_literal_inline(ip); }
int test2_vtable_local_func(int *ip) { return test2_literal_inline(ip); }
