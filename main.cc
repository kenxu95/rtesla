#include "rTesla.h"
#include <random>
#include <ctime>
#include "ecc/uECC.h"
#include "stdint.h"


#define NUM_MESSAGES 10000
#define LENGTH_MESSAGE 500
#define NUM_TRIALS 10000

#define SCHEME_TYPE 0 // 0 for rTesla, 1 for Ecc
static string charset = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

static char getRandomChar(default_random_engine& generator){
  uniform_int_distribution<int> dist(0, charset.length());
  return charset[dist(generator)];
}

static string generateMessage(default_random_engine& generator) {
  string result;
  result.resize(LENGTH_MESSAGE); 

  for (int i = 0; i < LENGTH_MESSAGE; i++){
    result[i] = getRandomChar(generator);
  } 
  return result;
}

static vector<string> generateMessages(default_random_engine& generator){
  vector<string> results(NUM_MESSAGES);
  for (unsigned int i = 0; i < results.size(); i++){
    results[i] = generateMessage(generator);
  }
  return results;
}


static void keyGenBenchmarkTests(){
  cout << "RING-TESLA TESTS: " << endl;
  cout << "Running Benchmarks for KeyGen()" << endl;
  cout << "---------------------------------------------------------" << endl;
  RingTesla rT = RingTesla();

  clock_t begin = clock();
  for (int i = 0; i < NUM_TRIALS; i++){
    rT.genPublic();
    rT.keyGen();
  }
  clock_t end = clock();
  double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  cout << NUM_TRIALS << " calls to keyGen() takes: " << elapsed_secs << " seconds" << endl;
}

static void signAndVerifyBenchmarkTests(vector<string>& messages){
  clock_t begin;
  clock_t end;
  double elapsed_secs;

  RingTesla rT = RingTesla();
  rT.genPublic();
  rT.keyGen(); 

  /* Benchmarks for signatures */
  cout << "Running Benchmarks for Signing: " << endl;
  cout << "-----------------------------------------------------------" << endl;
  vector<tuple<vector<int>, string> > signatures(messages.size());
  begin = clock();
  for (unsigned int i = 0; i < messages.size(); i++){
    signatures[i] = rT.sign(messages[i]);
  }
  end = clock();
  elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  cout << messages.size() << " calls to sign(): " << elapsed_secs << " seconds" << endl;

  /* Benmarks for verification */
  cout << "Running Benchmarks for Verifying: " << endl;
  cout << "------------------------------------------------------------" << endl;
  vector<bool> verifyResults(messages.size());
  begin = clock();
  for (unsigned int i = 0; i < messages.size(); i++){
    verifyResults[i] = rT.verify(messages[i], get<0>(signatures[i]), get<1>(signatures[i]));
  }
  end = clock();
  elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  cout << messages.size() << " calls to verify(): " << elapsed_secs << " seconds" << endl;

  /* Soundness verification */
  bool isSound = true;
  for (unsigned int i = 0; i < verifyResults.size(); i++){
    if (!verifyResults[i]){
      isSound = false;
    }
  }
  cout << "-----------------------------------------------------------" << endl;
  cout << "Soundness verification " << (isSound ? "passed." : "failed.") << endl;
}

static void testEcsda(default_random_engine& generator){
  int c;
  uint8_t priv[32] = {0};
  uint8_t pub[64] = {0};
  // uint8_t hash[64] = {0};

  const struct uECC_Curve_t * curves[5];
  int num_curves = 0;
#if uECC_SUPPORTS_secp160r1
  curves[num_curves++] = uECC_secp160r1();
#endif
#if uECC_SUPPORTS_secp192r1
  curves[num_curves++] = uECC_secp192r1();
#endif
#if uECC_SUPPORTS_secp224r1
  curves[num_curves++] = uECC_secp224r1();
#endif
#if uECC_SUPPORTS_secp256r1
  curves[num_curves++] = uECC_secp256r1();
#endif
#if uECC_SUPPORTS_secp256k1
  curves[num_curves++] = uECC_secp256k1();
#endif

  uint8_t messages[NUM_TRIALS][LENGTH_MESSAGE];
  for (int r = 0; r < NUM_TRIALS; r++){
    for (int i = 0; i < LENGTH_MESSAGE; i++){
      messages[r][i] = (uint8_t) getRandomChar(generator);
    }
  }

  /* For Timing */
  clock_t begin;
  clock_t end;
  double elapsed_secs;

  cout << "ECDSA TESTS: " << endl;
  for (c = 0; c < num_curves; ++c) {
    cout << "FOR CURVE NUMBER " << c << endl;
    cout << "-------------------------------------------------" << endl;

    cout << "Running Benchmarks for KeyGen()" << endl;
    cout << "---------------------------------------------------------" << endl;
    begin = clock();
    for(int t = 0; t < NUM_TRIALS; t++){
      if(!uECC_make_key(pub, priv, curves[c])){
        cout << "Make key failed: curve " << c << endl;
      }
    }
    end = clock();
    elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << NUM_TRIALS << " calls to keyGen() takes: " << elapsed_secs << " seconds" << endl;


    /* Sign benchmark tests */

    uint8_t signatures[NUM_TRIALS][64];
    cout << "Running Benchmarks for Signing: " << endl;
    cout << "-----------------------------------------------------------" << endl;
    uECC_make_key(pub, priv, curves[c]);
    begin = clock();
    for(int t = 0; t < NUM_TRIALS; t++){
      // if(!uECC_sign(priv, hash, sizeof(hash), signatures[t], curves[c])){
      if(!uECC_sign(priv, messages[t], LENGTH_MESSAGE, signatures[t], curves[c])){     
        cout << "Sign failed: curve " << c << endl;
      }
    }
    end = clock();
    elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << NUM_TRIALS << " calls to sign(): " << elapsed_secs << " seconds" << endl;

    // /* Verify benchmark tests */
    begin = clock();
    for(int t = 0; t < NUM_TRIALS; t++){
      // if(!uECC_verify(pub, hash, sizeof(hash), signatures[t], curves[c])){
      if(!uECC_verify(pub, messages[t], LENGTH_MESSAGE, signatures[t], curves[c])){
        cout << "Verify failed: curve " << c << endl;
      }
    }
    end = clock();
    elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;   
    cout << NUM_TRIALS << " calls to verify(): " << elapsed_secs << " seconds" << endl;
  }


}

int main(int argc, char *argv[]) {
  unsigned timeSeed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine generator(timeSeed);
  vector<string> messages = generateMessages(generator);

  if(SCHEME_TYPE == 0){
    keyGenBenchmarkTests();
    signAndVerifyBenchmarkTests(messages);
  }else if(SCHEME_TYPE == 1){
    testEcsda(generator);
  }

}






