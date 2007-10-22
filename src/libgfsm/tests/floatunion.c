#include <stdio.h>

typedef union {
  float        f;
  void*        p;
  int          i;
  unsigned int u;
} t_weight;

int main(void) {
  t_weight w1;
  t_weight w2 = {42.24};

  w1.f=42.24;
  printf("w1.f=%f, w1.p=%p, w1.i=%d, w1.u=%u\n", w1.f, w1.p, w1.i, w1.u);
  printf("w2.f=%f, w2.p=%p, w2.i=%d, w2.u=%u\n", w2.f, w2.p, w2.i, w2.u);

  return 0;
}
