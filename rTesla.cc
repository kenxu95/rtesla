#include "rTesla.h"
#include <math.h>
#include <algorithm>
#include <numeric>
#include <functional>

RingTesla::RingTesla() {
  n = 512;
  w = 19;
  sigma = 52;
  B = (1 << 22) - 1;
  d = 23;
  U = 3173;
  L = 2766;
  q = 39960577;
  lambda = 128;

  generator.seed(timeSeed);
}

vector<int> RingTesla::sampleZqPolynomial(bool useB){
  int minDist = useB ? -1 * B : -1 * ceil(q / 2);
  int maxDist = useB ? B : floor(q / 2);

  uniform_int_distribution<int> dist(minDist, maxDist);

  vector<int> result(n);
  for (unsigned int i = 0; i < result.size(); i++){
    result[i] = dist(generator);
  }
  return result;
}

/* Samples and returns the public polynomials a1 and a2.
 * Each polynomial is a member of ring group R_q and can be 
 * represented as a single number. */
void RingTesla::genPublic(){
  a1 = sampleZqPolynomial(false);
  a2 = sampleZqPolynomial(false); 
}

/* Returns a polynomial of length n according to the discrete Gaussian distribution with
 * with standard deviation sigma */
vector<int> RingTesla::sampleGaussianPolynomial(){
  std::normal_distribution<double> dist(0, sigma);
  vector<int> samples(n);
  for (unsigned int i = 0; i < n; i++){
    samples[i] = trunc(dist(generator));
  }
  return samples;
}

/* Return false if polynomial e passes, true otherwise. */
bool RingTesla::checkE(vector<int>& e){
  nth_element(e.begin(), e.begin() + w, e.end());
  return accumulate(e.end() - w, e.end(), 0) > L;
}

/* Perform multiplication on two polynomials in the ring R/(x^n + 1) */
vector<int> RingTesla::multiplyPolynomials(vector<int>& vec1, vector<int>& vec2){
  vector<int> result(n, 0);
  for(unsigned int i = 0; i < vec1.size(); i++){
    for(unsigned int j = 0; j < vec2.size(); j++){
      int val = vec1[i] * vec2[j];
      int index = (i + j) % n; 
      result[index] += val;
    }
  }
  return result;
}

/* Addition of two polynomials */
vector<int> RingTesla::addPolynomials(vector<int>& vec1, vector<int>& vec2){
  vector<int> result(n);
  transform(vec1.begin(), vec1.end(), vec2.begin(), result.begin(), std::plus<int>());
  return result;
}

/* Calculates a * s + e */
vector<int> RingTesla::calculateT(vector<int>& a, vector<int>& s, vector<int>& e){
  vector<int> multResult = multiplyPolynomials(a, s); 
  return addPolynomials(multResult, e);
}

void RingTesla::keyGen(){
  vector<int> s;
  vector<int> e1; 
  vector<int> e2;
  do {
    s = sampleGaussianPolynomial();
    e1 = sampleGaussianPolynomial();
    e2 = sampleGaussianPolynomial();
  } while(checkE(e1) || checkE(e2)); /* Continue to sample if polynomials do not pass */

  /* Generate the public and private keys */
  vector<int> t1 = calculateT(a1, s, e1);
  vector<int> t2 = calculateT(a2, s, e2);

  /* Obtain the public and secret keys */
  sk = make_tuple(s, e1, e2);
  pk = make_tuple(t1, t2); 
}

/* Signs a message with the secret key */
void RingTesla::sign(string message){

  /* Sample y uniformly from R_{q, {B}} */
  vector<int> y = sampleZqPolynomial(true);


}


void RingTesla::test(){

  /* KeyGen: CheckE() test */
  // vector<int> test= {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 26};
  // shuffle(test.begin(), test.end(), generator);

  /* KeyGen: t generation test */
  // int a = 3;
  // vector<int> b = {3, 2, 1};
  // vector<int> c = {10, 20, 30};
  // vector<int> result = generateT(a, b, c);
  // for (unsigned int i = 0; i < b.size(); i++){
  //   cout << result[i] << " ";
  // }
  // cout << endl;

  // cout << a << endl;

  // for (unsigned int i = 0; i < b.size(); i++){
  //   cout << b[i] << " ";
  // }
  // cout << endl; 

  // for (unsigned int i = 0; i < b.size(); i++){
  //   cout << c[i] << " ";
  // }
  // cout << endl; 
}






