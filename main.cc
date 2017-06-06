#include "rTesla.h"

int main(int argc, char *argv[]) {
  RingTesla rT = RingTesla();
  rT.genPublic();
  rT.keyGen();

  vector<string> messages = {"Hello, World", "Kenny is awesome", "This town ain't big enough", 
                              "for the two of us", "why doesn't this work?"};
  cout << "Results: " << endl;                              
  for(unsigned int i = 0; i < messages.size(); i++){
    for(unsigned int j = 0; j < messages.size(); j++){
      cout << "------------------------------------------------" << endl;
      cout << messages[i] << " : " << messages[j] << endl;
      tuple<vector<int>, string> signature = rT.sign(messages[i]);
      cout << rT.verify(messages[j], get<0>(signature), get<1>(signature)) << endl;
    }
  }                              
}

