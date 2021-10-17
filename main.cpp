#include <iostream>
#include "clockwithshift.h"

int main() {
  std::cout << "setup()\n";
  setup();

  for (int i=0; i < 2; i++) {
    std::cout << "\nloop()\n";
    loop();
  }

  return 0;
}
