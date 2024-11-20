#include "utils.hpp"
#include <fstream>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <sstream>

std::string load_file(const std::string &filepath) {
  std::ifstream file(filepath);
  if (!file) {
    std::cerr << "Error loading file." << std::endl;
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
} 
