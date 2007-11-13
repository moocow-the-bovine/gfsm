#include <stdio.h>

typedef struct {
  int i;
  int j;
} IntPairT;

int main (void) {
  IntPairT pairs[8];

  printf("sizeof(int)=%d; sizeof(IntPairT)=%d\n", sizeof(int), sizeof(IntPairT));
  printf("&pairs[1]-&pairs[0]=%d\n", &(pairs[1])-&(pairs[0]));
  printf("(char*)&pairs[1]-(char*)&pairs[0]=%d\n", ((char*)&(pairs[1]))-((char*)&(pairs[0])));
  printf("(void*)&pairs[1]-(void*)&pairs[0]=%d\n", ((void*)&(pairs[1]))-((void*)&(pairs[0])));

  return 0;
}
