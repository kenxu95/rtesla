#ifndef RTESLA_H_
#define RTESLA_H_

#include <iostream>
#include <tuple>
#include <random>
#include <chrono>
#include <vector>

using namespace std;

class RingTesla {
private:
  unsigned int n; // Encoding function: output vector length (must be positive)
  unsigned int w; // Encoding function: weight
  unsigned int sigma;
  unsigned int B;
  unsigned int d;
  unsigned int U;
  unsigned int L; // Polynomial threshold (for verifying e_1 and e_2)
  unsigned int q;
  unsigned int lambda; // Security parameter, which is < kappa < length

  //int kappa; // Output length of hash function

  // Hash H: hashes to length kappa
  // Encoding function F: takes hash output and maps to vector of length n and weight w

  unsigned timeSeed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine generator;

  /* Public polynomials */
  vector<int> a1;
  vector<int> a2; 

  /* Keys */
  tuple<vector<int>, vector<int>, vector<int> > sk;
  tuple<vector<int>, vector<int> > pk;

  /* Methods */ 
  vector<int> sampleZqPolynomial(bool useB);
  vector<int> sampleGaussianPolynomial();
  bool checkE(vector<int>& e);
  vector<int> multiplyPolynomials(vector<int>& vec1, vector<int>& vec2);
  vector<int> addPolynomials(vector<int>& vec1, vector<int>& vec2); 
  vector<int> calculateT(vector<int>& a, vector<int>& s, vector<int>& e);

public:
  RingTesla();
  void genPublic();
  vector<int> getA1(){return a1;}
  vector<int> getA2(){return a2;}
 
  void keyGen();
  vector<int> getT1(){return get<0>(pk);}
  vector<int> getT2(){return get<1>(pk);}

  void sign(string message);

  void test();
};

#endif