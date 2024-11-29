#include "dashboard.hpp"
#include "presentation.hpp"

int main (int argc, char *argv[]) {
  if (argc <= 1) {
    // USAGE: `frosch`
    // Opens the Dashboard.

    Dashboard();
  } else {
    std::string path = argv[1];
    Presentation p(path);
  }
  return 0;
}
