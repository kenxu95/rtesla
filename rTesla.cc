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
  // RingTesla-I parameters
  n = 512;
  w = 16; /* original paper set value to 11 */
  sigma = 30;
  B = (1 << 21) - 1;
  d = 21;
  U = 993;
  L = 814;
  q = 8399873;
  lambda = 80;
  kappa = 256;

  // RingTesla-II parameters
  // n = 512;
  // w = 16; /* original paper set value to 19 */
  // sigma = 52;
  // B = (1 << 22) - 1;
  // d = 23;
  // U = 3173;
  // L = 2766;
  // q = 39960577;
  // lambda = 128;
  // kappa = 256;

  generator.seed(timeSeed);
  q_inv = 1 / (double) q;
}


vector<int> RingTesla::sampleZqPolynomial(bool useB){
  int minDist = useB ? (-1 * (int) B) : -1 * ceil(q / 2);
  int maxDist = useB ? ((int) B) : floor(q / 2);

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
  return accumulate(e.end() - w, e.end(), 0) > (int) L;
}

int RingTesla::performModOnVal(int val, unsigned int magnitudeMod){
  int valToMod = val < 0 ? (-1 * val) : val;
  int moddedVal = valToMod % (magnitudeMod);
  return val < 0 ? (-1 * moddedVal) : moddedVal;
}

/* Performs mod of q */
void RingTesla::performModQ(vector<int>& vec){
	double divider;
	int divider_int;
	for(unsigned int i = 0; i < vec.size(); i++){
	divider = (double) vec[i] * q_inv;
	divider_int = (int) round(divider);
    vec[i] = vec[i] - (divider_int * q);
  }
}

int RingTesla::performModQOnLongVal(int64_t val){
	double divider = (double) val * q_inv;
	//int64_t divider_int = divider > 0 ? (int64_t) floor(divider) : (int64_t) ceil(divider);
	int64_t  divider_int = (int64_t) round(divider);
 return val - (divider_int * (int64_t)q);
}

/* Perform multiplication on two polynomials in the ring R/(x^n + 1) */
vector<int> RingTesla::multiplyPolynomials(vector<int>& vec1, vector<int>& vec2){
if(vec1.size() != vec2.size())
	cout << "Multiplication dimension error" << endl;
  vector<int64_t> result(vec1.size(), 0);
  for(unsigned int i = 0; i < vec1.size(); i++){
    for(unsigned int j = 0; j < vec2.size(); j++){
      int64_t val = (int64_t) vec1[i] * (int64_t) vec2[j];
      int index = (i + j) % vec1.size();
      result[index] += val;
    }
  }
  vector<int> res(vec1.size(), 0);
  for(unsigned int i = 0; i < vec1.size(); i++){
	  res[i] = performModQOnLongVal(result[i]);
  }
  return res;
}

/* Addition of two polynomials */
vector<int> RingTesla::addPolynomials(vector<int>& vec1, vector<int>& vec2){
  vector<int> result(n);
  transform(vec1.begin(), vec1.end(), vec2.begin(), result.begin(), std::plus<int>());
  return result;
}

vector<int> RingTesla::subtractPolynomials(vector<int>& vec1, vector<int>& vec2){
	if(vec1.size() != vec2.size())
		cout << "Subtraction dimension error" << endl;
	vector<int> result(vec1.size());
    for(unsigned int i = 0; i < result.size(); i++){
	  result[i] = vec1[i] - vec2[i];
  }
  return result;
}

/* Calculates a * s + e */
vector<int> RingTesla::calculateT(vector<int>& a, vector<int>& s, vector<int>& e){
  vector<int> multResult = multiplyPolynomials(a, s); 
  vector<int> result = addPolynomials(multResult, e);
  performModQ(result);
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
  int magnitudeMod = 1 << (d - 1);
  int magnitudeModCheck = (1 << (d - 1)) - L;
  for (unsigned int i = 0; i < w.size(); i++){
    int moddedW = performModOnVal(w[i], magnitudeMod); 
    if (abs(moddedW) > magnitudeModCheck){
      return false;
    }
  } 
  return true;
}

bool RingTesla::checkZ(vector<int>& z_vec){
  int magnitude0 = B - U;
  for (unsigned int i = 0; i < z_vec.size(); i++){
    if (abs(z_vec[i]) > magnitude0){
      return false;
    }
  }
  return true;
}

vector<int> RingTesla::encoding(string hashResult){
  int numIndexBits = (unsigned int)log2(n);
  int blockSize = kappa / w;

  /* For each block, encode a single element in result */
  vector<int> result(n);
  for(unsigned int i = 0; i < w; i++){
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

string RingTesla::hash(string message, vector<int>& v1, vector<int>& v2){
  string toHash = message;
  for (unsigned int i = 0; i < v1.size(); i++){
	  toHash += to_string(v1[i]>> d);
  }
  for (unsigned int i = 0; i < v2.size(); i++){
	  toHash += to_string(v2[i]>> d);
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
    performModQ(v1);
    vector<int> v2 = multiplyPolynomials(a2, y);
    performModQ(v2);

    c_prime = hash(message, v1, v2);
    vector<int> c = encoding(c_prime);

    /* Calculate z */
    vector<int> s_c = multiplyPolynomials(get<0>(sk), c);
    z = addPolynomials(y, s_c);
    /* Rejection Sampling */
    vector<int> e1_c = multiplyPolynomials(get<1>(sk), c);
    w1 = subtractPolynomials(v1, e1_c);
    performModQ(w1);
    vector<int> e2_c = multiplyPolynomials(get<2>(sk), c);
    w2 = subtractPolynomials(v2, e2_c);
    performModQ(w2);
//    cout << checkW(w1) << checkW(w2) << checkZ(z) << endl;
  }while (!checkW(w1) || !checkW(w2) || !checkZ(z));
//  cout << "sign:\t" << c_prime << endl;
  return make_tuple(z, c_prime);
}

/* Verify */
bool RingTesla::verify(string message, vector<int>& z, string c_prime){
  vector<int> c = encoding(c_prime);

  /* Calculate w1 and w2 */
  vector<int> a1_z = multiplyPolynomials(a1, z);
  vector<int> t1_c = multiplyPolynomials(get<0>(pk), c);
  vector<int> w1 = subtractPolynomials(a1_z, t1_c);
  performModQ(w1);

  vector<int> a2_z = multiplyPolynomials(a2, z);
  vector<int> t2_c = multiplyPolynomials(get<1>(pk), c);
  vector<int> w2 = subtractPolynomials(a2_z, t2_c);
  performModQ(w2);

  /* Calculate c_verify */
  string c_verify = hash(message, w1, w2);
//  cout << "verify:\t" << c_verify << endl;
  return (c_prime.compare(c_verify) == 0) && checkZ(z);
}



