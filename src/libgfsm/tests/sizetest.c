#include <stdio.h>

typedef struct {
  int x1 : 1;
  int x2 : 1;
  int x3 : 30;
} tstruc;

int main (void) {
  int i;
  tstruc ts;

  printf("sizeof(int)=%u ; sizeof(tstruc)=%u\n", sizeof(int), sizeof(tstruc));
  return 0;
}
