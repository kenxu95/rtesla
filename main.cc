#include "rTesla.h"

int main(int argc, char *argv[]) {
  RingTesla rT = RingTesla();
  rT.genPublic();
  rT.keyGen();
  rT.sign("Hello, World");
}

