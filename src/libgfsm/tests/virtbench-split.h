#ifndef VIRTBENCH_H
#define VIRTBENCH_H

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

int test1_literal_func(int *ip);
int test2_literal_func(int *ip);

int test1_switch_func(int *ip);
int test2_switch_func(int *ip);

int test1_vtable_global_func(int *ip);
int test2_vtable_global_func(int *ip);

int test1_vtable_local_func(int *ip);
int test2_vtable_local_func(int *ip);

typedef int (*IntUnaryFunc)(int *ip);

typedef struct {
  int val;
  IntUnaryFunc func;
} VTestT;


#endif /* VIRTBENCH_H */
