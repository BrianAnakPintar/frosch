#include <cstdlib>
#include <iostream>
#include <string>
#include "presentation.hpp"

// pat++
int main (int argc, char *argv[]) {
  if (argc <= 1) {
    std::cout << "Invalid arguments, usage: ./slides-pp <path>" << std::endl;
  } else {
    std::string path = argv[1];
    Presentation p(path);
  }
  return 0;
}
