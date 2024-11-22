#include "utils.hpp"
#include <fstream>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <md4c.h>
#include <sstream>
#include <map>

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

std::string blockTypeToString(MD_BLOCKTYPE blockType) {
  static const std::map<MD_BLOCKTYPE, std::string> blockTypeToStringMap = {
    {MD_BLOCK_DOC, "MD_BLOCK_DOC"},
    {MD_BLOCK_QUOTE, "MD_BLOCK_QUOTE"},
    {MD_BLOCK_UL, "MD_BLOCK_UL"},
    {MD_BLOCK_OL, "MD_BLOCK_OL"},
    {MD_BLOCK_LI, "MD_BLOCK_LI"},
    {MD_BLOCK_HR, "MD_BLOCK_HR"},
    {MD_BLOCK_H, "MD_BLOCK_H"},
    {MD_BLOCK_CODE, "MD_BLOCK_CODE"},
    {MD_BLOCK_HTML, "MD_BLOCK_HTML"},
    {MD_BLOCK_P, "MD_BLOCK_P"},
    {MD_BLOCK_TABLE, "MD_BLOCK_TABLE"},
    {MD_BLOCK_THEAD, "MD_BLOCK_THEAD"},
    {MD_BLOCK_TBODY, "MD_BLOCK_TBODY"},
    {MD_BLOCK_TR, "MD_BLOCK_TR"},
    {MD_BLOCK_TH, "MD_BLOCK_TH"},
    {MD_BLOCK_TD, "MD_BLOCK_TD"}
  };
  auto it = blockTypeToStringMap.find(blockType);
  return it != blockTypeToStringMap.end() ? it->second : "Unknown Block Type";
}

std::string spanTypeToString(MD_SPANTYPE spanType) {
  static const std::map<MD_SPANTYPE, std::string> spanTypeToStringMap = {
    {MD_SPAN_CODE, "MD_SPAN_CODE"},
    {MD_SPAN_STRONG, "MD_SPAN_STRONG"},
    {MD_SPAN_A, "MD_SPAN_A"},
    {MD_SPAN_U, "MD_SPAN_U"},
    {MD_SPAN_EM, "MD_SPAN_EM"},
    {MD_SPAN_IMG, "MD_SPAN_IMG"},
    {MD_SPAN_DEL, "MD_SPAN_DEL"},
    {MD_SPAN_LATEXMATH, "MD_SPAN_LATEXMATH"},
    {MD_SPAN_LATEXMATH_DISPLAY, "MD_SPAN_LATEXMATH_DISPLAY"},
    {MD_SPAN_WIKILINK, "MD_SPAN_WIKILINK"}
  };
  auto it = spanTypeToStringMap.find(spanType);
  return it != spanTypeToStringMap.end() ? it->second : "Unknown Span Type";
}

void PrintComponent(const ftxui::Component& component) {
  auto element = component->Render();
  auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(), ftxui::Dimension::Fit(element));
  Render(screen, element);
  std::cout << screen.ToString() << std::endl;
}
