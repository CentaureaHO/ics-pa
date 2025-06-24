#include <klib.h>

static unsigned long int next = 1;

int rand(void) {
  // LCG: next = (a * next + c) % m
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0) ? -x : x;
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + (*nptr - '0');
    nptr++;
  }
  return x;
}

unsigned long time() {
  // This should be implemented by AM
  return 0;
}
