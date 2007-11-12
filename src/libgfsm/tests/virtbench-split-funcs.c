#include "virtbench-split.h"

int test1_literal_func(int *ip) { test1_code(ip); return (*ip); }
int test2_literal_func(int *ip) { test2_code(ip); return (*ip); }

int test1_switch_func(int *ip) { test1_code(ip); return (*ip); }
int test2_switch_func(int *ip) { test2_code(ip); return (*ip); }

int test1_vtable_global_func(int *ip) { test1_code(ip); return (*ip); }
int test2_vtable_global_func(int *ip) { test2_code(ip); return (*ip); }

int test1_vtable_local_func(int *ip) { test1_code(ip); return (*ip); }
int test2_vtable_local_func(int *ip) { test2_code(ip); return (*ip); }
