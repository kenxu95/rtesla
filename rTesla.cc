#include "rTesla.h"
#include <math.h>
#include <algorithm>
#include <numeric>
#include <functional>
#include "sha256.h"
#include <string>
#include <sstream>

// static void printVector(vector<int>& vec){
//   for (unsigned int i = 0; i < vec.size(); i++){
//     cout << vec[i] << ", ";
//   }
//   cout << endl; 
// }

RingTesla::RingTesla() {
  n = 512;
  // w = 19;
  w = 16;
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

int RingTesla::performModOnVal(int val, unsigned int magnitudeMod){
  int valToMod = val < 0 ? (-1 * val) : val;
  int moddedVal = valToMod % (magnitudeMod);
  return val < 0 ? (-1 * moddedVal) : moddedVal;
}

/* Performs mod of q */
void RingTesla::performMod(vector<int>& vec){
  for(unsigned int i = 0; i < vec.size(); i++){
    vec[i] = performModOnVal(vec[i], q / 2);
  }
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

vector<int> RingTesla::subtractPolynomials(vector<int>& vec1, vector<int>& vec2){
  vector<int> result(n);
  transform(vec1.begin(), vec1.end(), vec2.begin(), result.begin(), std::minus<int>());
  return result;
}

/* Calculates a * s + e */
vector<int> RingTesla::calculateT(vector<int>& a, vector<int>& s, vector<int>& e){
  vector<int> multResult = multiplyPolynomials(a, s); 
  vector<int> result = addPolynomials(multResult, e);
  performMod(result);
  return result;
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

bool RingTesla::checkW(vector<int>& w){
  unsigned int magnitudeMod = 1 << (d - 1);
  unsigned int magnitudeModCheck = (1 << (d - 1)) - L;
  for (unsigned int i = 0; i < w.size(); i++){
    int moddedW = performModOnVal(w[i], magnitudeMod); 
    if (abs(moddedW) > magnitudeModCheck){
      return false;
    }
  } 
  return true;
}

bool RingTesla::checkZ(vector<int>& z_vec){
  unsigned int magnitude = B - U;
  for (unsigned int i = 0; i < z_vec.size(); i++){
    if (abs(z_vec[i]) > magnitude){
      return false;
    }
  }
  return true;
}

vector<int> RingTesla::encoding(string hashResult){
  unsigned int numIndexBits = (unsigned int)log2(n); /* is 9 */
  unsigned int blockSize = 256 / w; /* is 16 */

  /* For each block, encode a single element in result */
  vector<int> result(n);
  for(int i = 0; i < w; i++){
    string blockStr = hashResult.substr(i * (blockSize / 4), blockSize / 4); /* divide by 4 because hexadecimal */
    unsigned int x;
    std::stringstream ss;
    ss << hex << blockStr;
    ss >> x;

    unsigned int signBitTest = (1 << (blockSize - 1));
    bool sign = x & signBitTest; // Gets most significant bit
    unsigned int index = (x & (~signBitTest)) >> (blockSize - numIndexBits);
    result[index] = sign ? 1 : -1;
  }
  return result;
}

int RingTesla::roundVal(int val, unsigned int magnitudeMod){
  int diff = val - performModOnVal(val, magnitudeMod);
  return performModOnVal(diff, magnitudeMod);
}

string RingTesla::hash(string message, vector<int>& v1, vector<int>& v2){
  string toHash = message;
  for (unsigned int i = 0; i < v1.size(); i++){
    toHash += to_string(roundVal(v1[i], 1 << (d - 1)));
  }
  for (unsigned int i = 0; i < v2.size(); i++){
    toHash += to_string(roundVal(v2[i], 1 << (d - 1))); 
  }

  string hashResult = sha256(toHash);
  return hashResult;
}

/* Signs a message with the secret key. The result is stored in c_prime and z */
tuple<vector<int>, string> RingTesla::sign(string message){
  vector<int> w1;
  vector<int> w2;
  vector<int> z;
  string c_prime;

  do{
    /* Sample y uniformly from R_{q, {B}} */
    vector<int> y = sampleZqPolynomial(true);

    /* Calculate v1 and v2 */
    vector<int> v1 = multiplyPolynomials(a1, y);
    performMod(v1);
    vector<int> v2 = multiplyPolynomials(a2, y);
    performMod(v2);

    /* TODO: Generate the ciphertext with the hash and encoding functions */
    c_prime = hash(message, v1, v2);
    vector<int> c = encoding(c_prime);

    /* Calculate z */
    vector<int> s_c = multiplyPolynomials(get<0>(sk), c);
    z = addPolynomials(y, s_c);

    /* Rejection Sampling */
    vector<int> e1_c = multiplyPolynomials(get<1>(sk), get<0>(sk));
    w1 = subtractPolynomials(v1, e1_c);
    performMod(w1);
    vector<int> e2_c = multiplyPolynomials(get<2>(sk), get<0>(sk));
    w2 = subtractPolynomials(v2, e2_c);
    performMod(w2);

  }while (!checkW(w1) || !checkW(w2) || !checkZ(z));

  cout << "sign:\t" << c_prime << endl;
  return make_tuple(z, c_prime);
}

/* Verify */
bool RingTesla::verify(string message, vector<int>& z, string c_prime){
  vector<int> c = encoding(c_prime);

  /* Calculate w1 and w2 */
  vector<int> a1_z = multiplyPolynomials(a1, z);
  vector<int> t1_c = multiplyPolynomials(get<0>(pk), c);
  vector<int> w1 = subtractPolynomials(a1_z, t1_c);
  performMod(w1);

  vector<int> a2_z = multiplyPolynomials(a2, z);
  vector<int> t2_c = multiplyPolynomials(get<1>(pk), c);
  vector<int> w2 = subtractPolynomials(a2_z, t2_c);
  performMod(w2); 


  /* Calculate c_verify */
  string c_verify = hash(message, w1, w2);
  cout << "verify:\t" << c_verify << endl;
  return (c_prime.compare(c_verify) == 0) && checkZ(z);
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






