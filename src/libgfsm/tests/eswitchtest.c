#include <stdio.h>
#include <stdlib.h>

static inline int testme(int i, int j) {
  switch (i) {
  case 0:
    switch (j) {
    case 0: return 1;
    default: return j*j;
    }
    break;
  default:
    switch (j) {
    case 0: return i*i;
    default: return i*j;
    }
    break;
  }
  return i+j;
}

int main (int argc, char **argv) {
  const char *is="0", *js="1";
  int i,j, k;
  if (argc>1) { is=argv[1]; }
  if (argc>2) { js=argv[2]; }
  i = strtol(is,NULL,0);
  j = strtol(js,NULL,0);

  k = testme(i,j);
  printf("i=%d, j=%d: k=%d\n", i,j,k);

  return 0;
}
