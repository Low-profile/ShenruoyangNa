#include <stdio.h>

int foo(int a){
  int b = 1;
  int c = a + b;
  while(c < 100)
    c += c;
  return c;
}

int main() {
  return 0;
}