#include "presentation.hpp"
#include <md4c.h>
#include "utils.hpp"
#include <iostream>

// Callback for entering block.
int EnterBlock(MD_BLOCKTYPE block_type, void* block_detail, void* userdata) {
  // Header files indicate a title screen.
  switch (block_type) {
    case MD_BLOCK_DOC:
    case MD_BLOCK_QUOTE:
      break;
    case MD_BLOCK_UL: {
      MD_BLOCK_UL_DETAIL* detail = (MD_BLOCK_UL_DETAIL*) block_detail;

      break;
    }
    case MD_BLOCK_OL: {
      MD_BLOCK_OL_DETAIL* detail = (MD_BLOCK_OL_DETAIL*) block_detail;

      break;
    }
    case MD_BLOCK_LI: {

      break;
    }
    case MD_BLOCK_HR: {
      break;
    }
    case MD_BLOCK_H: {
      MD_BLOCK_H_DETAIL* detail = (MD_BLOCK_H_DETAIL*) block_detail;
      std::cout << detail->level << std::endl;
      break;
    }
    case MD_BLOCK_CODE: {

      break;
    }
    case MD_BLOCK_HTML: {
      break;
    }
    case MD_BLOCK_P:
      break;
    case MD_BLOCK_TABLE:
      break;
    case MD_BLOCK_THEAD:
      break;
    case MD_BLOCK_TBODY:
      break;
    case MD_BLOCK_TR:
      break;
    case MD_BLOCK_TH:
      break;
    case MD_BLOCK_TD:
      break;
  }
  if (block_type == MD_BLOCK_H) {
    MD_BLOCK_H_DETAIL* detail = (MD_BLOCK_H_DETAIL*) block_detail;
  }
  return 0;
}

int LeaveBlock(MD_BLOCKTYPE block_type, void* block_detail, void* userdata) {
  return 0;
}

int EnterSpan(MD_SPANTYPE block_type, void* block_detail, void* userdata) {
  return 0;
}

int LeaveSpan(MD_SPANTYPE block_type, void* block_detail, void* userdata) {
  return 0;
}

int EnterText(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata) {
  std::string text_str(text);
  std::cout << text_str << std::endl; 
  return 0;
}

Presentation::Presentation(const std::string& file_path) {
  // 1. Read markdown.
  std::string md_str = load_file(file_path);
  // 2. Parse markdown.
  MD_PARSER parser = {
    0,              // abi_version set to 0.
    0,              // flags
    EnterBlock,     // Callback for entering a block.
    LeaveBlock,     // Callback for leaving a block.
    EnterSpan,      // Callback for entering a span.
    LeaveSpan,      // Callback for leaving a span.
    EnterText       // Callback for reading text.
  };
  int ok = md_parse(md_str.c_str(), md_str.size(), &parser, nullptr);
  if (ok != 0) {
    std::cerr << "Something went wrong parsing file." << std::endl;
    return;
  }
  // 3. Start Presentation.
  StartPresentation();
}

bool Presentation::SavePresentation(const std::string& output_path) {
  return false;
}

void Presentation::StartPresentation() {
  
}


Presentation::~Presentation() {}
