#include "presentation.hpp"
#include <iostream>

int main (int argc, char *argv[]) {
  if (argc <= 1) {
    std::cout << "Invalid arguments, usage: ./frosch <path>" << std::endl;
  } else {
    std::string path = argv[1];
    Presentation p(path);
  }
  return 0;
}
