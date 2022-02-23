#include <stdio.h>

int sub(int x, int y) {
  int z;
  z = x - y;
  return z;
}

int main() {
  int diff1;
  diff1 = sub(3, 7);
  printf("diff = %d\n", diff1);
  return 0;
}
