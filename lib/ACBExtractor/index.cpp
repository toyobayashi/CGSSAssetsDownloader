#include <iostream>
#include "./include/ACBExtractor.h"

using namespace std;

int main(int argc, char** argv) {
  ACBExtractor* acb;
  try {
    for (int x = 1; x < argc; x++) {
      acb = new ACBExtractor(argv[x]);

      if (acb->extract([](string filename, unsigned int length) -> void {
        cout << "File: " << filename.c_str() << " Size: " << length << "\n";
      })) {
        cout << argv[x] << " done." << "\n\n";
      }
      delete acb;
    }
  } catch (const char* err) {
    cout << err << "\n";
    return 0;
  }

  return 0;
}
