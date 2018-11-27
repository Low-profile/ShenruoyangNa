#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

int64_t extern_input() {
  int64_t n;
  scanf("%"SCNi64, &n);
  return n;
}

void extern_print_int(int64_t x) {
  printf("%"PRIi64, x);
}

void extern_print_string(char* str) {
  printf("%s", str);
}

void extern_abort() {
  abort();
}

uint64_t extern_pow(uint64_t x, uint64_t n) {
  uint64_t result = 1;
  while (n--) {
    result *= x;
  }
  return result;
}

