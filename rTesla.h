#ifndef RTESLA_H_
#define RTESLA_H_

#include <iostream>

using namespace std;

class RingTesla {
private:
  unsigned int n; // Encoding function: output vector length (must be positive)
  int w; // Encoding function: weight
  int d; 
  int B;
  int q; 
  int U;
  int L; // Polynomail threshold (for verifying e_1 and e_2)
  int kappa; // Output length of hash function

  int lambda; // Security parameter, which is < kappa < length

  // TODO: 
  // Hash H: hashes to length kappa
  // Encoding function F: takes hash output and maps to vector of length n and weight w


public:
  // a_1, a_2


  RingTesla(int t);
};

#endif