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
  unsigned int kappa; // Output length of hash function

  /* Random Number Generator */
  /* TODO: Is not necessarily cryptographically secure */
  unsigned timeSeed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine generator;

  /* Public polynomials */
  vector<int> a1;
  vector<int> a2; 

  /* Keys */
  tuple<vector<int>, vector<int>, vector<int> > sk; /* (s, e1, e2) */
  tuple<vector<int>, vector<int> > pk; /* (t1, t2) */

  /* Methods */ 
  vector<int> sampleZqPolynomial(bool useB);
  vector<int> sampleGaussianPolynomial();
  bool checkE(vector<int>& e); /* for keygen */
  bool checkW(vector<int>& w); /* for signing */
  bool checkZ(vector<int>& z_vec);
  int performModOnVal(int val, unsigned int magnitudeMod);
  void performMod(vector<int>& vec);
  int roundVal(int val, unsigned int magnitudeMod);
  vector<int> multiplyPolynomials(vector<int>& vec1, vector<int>& vec2);
  vector<int> subtractPolynomials(vector<int>& vec1, vector<int>& vec2); 
  vector<int> addPolynomials(vector<int>& vec1, vector<int>& vec2); 
  vector<int> calculateT(vector<int>& a, vector<int>& s, vector<int>& e);
  string hash(string message, vector<int>& v1, vector<int>& v2);
  vector<int> encoding(string hashResult);

public:
  RingTesla();
  void genPublic();
  vector<int> getA1(){return a1;}
  vector<int> getA2(){return a2;}
 
  void keyGen();
  tuple<vector<int>, vector<int> > getPK(){return pk;} /* Public key is accessible */

  tuple<vector<int>, string> sign(string message);
  bool verify(string message, vector<int>& z, string c_prime);
};

#endif