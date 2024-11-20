#include "presentation.hpp"
#include <ftxui/component/component_base.hpp>
#include <ftxui/screen/color.hpp>
#include "ftxui/component/screen_interactive.hpp"
#include <md4c.h>
#include "ftxui/dom/elements.hpp"
#include "utils.hpp"
#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <string>
#include <map>

using namespace ftxui;

// Represents a block in MD.
struct MarkdownBlock {
  MD_BLOCKTYPE type;
  void* detail;

  MarkdownBlock(MD_BLOCKTYPE type, void* detail) : type(type), detail(detail) {}
};

// Represents a span in MD.
struct MarkdownSpan {
  MD_SPANTYPE type;
  void* detail;

  MarkdownSpan(MD_SPANTYPE type, void* detail) : type(type), detail(detail) {}
};

// Either a MarkdownBlock or MarkdownSpan.
struct MarkdownInformation {
  enum MD_Type { Block, Span, None } md_type = None;

  union {
    MarkdownBlock block;
    MarkdownSpan span;
  };

  MarkdownInformation() : md_type(None) {}

  MarkdownInformation(const MarkdownBlock& b) : md_type(Block), block(b) {}

  MarkdownInformation(const MarkdownSpan& s) : md_type(Span), span(s) {}

  ~MarkdownInformation() {
    if (md_type == Block) {
      block.~MarkdownBlock();
    } else if (md_type == Span) {
      span.~MarkdownSpan();
    }
  }
};

// Represents our current state when parsing the markdown.
struct SlidesData {
  std::stack<MarkdownInformation> info_stack;
  std::stack<std::vector<Element>> elem_stack;
  Presentation& presentation;

  enum ListType {
    unordered,
    ordered,
    none
  };

  // For lists.
  ListType last_list_type = none;
  int list_idx = 0;

  SlidesData(Presentation& p) : presentation(p) {}
};

void PrintComponent(const Component& component) {
  auto element = component->Render();
  auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(element));
  Render(screen, element);
  std::cout << screen.ToString() << std::endl;
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

// Callback for entering block.
int EnterBlock(MD_BLOCKTYPE block_type, void* block_detail, void* userdata) {
  auto* sd = static_cast<SlidesData*>(userdata);
  std::cout << "Entering: " << blockTypeToString(block_type) << '\n';
  sd->info_stack.emplace(MarkdownBlock(block_type, block_detail));
  sd->elem_stack.emplace(); // Push an empty vector for elements.
  return 0;
}

Element GetBlockType(MD_BLOCKTYPE type, Elements elems, SlidesData* data) {
  switch (type) {
    case MD_BLOCK_H:
      return hbox(elems);
    case MD_BLOCK_P:
      return hbox(elems);
    case MD_BLOCK_DOC:
      return vbox(elems);
    case MD_BLOCK_LI: {
      Elements list;
      list.reserve(elems.size()+1);
      if (data->last_list_type == SlidesData::ordered) {
        list.emplace_back(text("• "));
      } else if (data->last_list_type) {
        list.emplace_back(text(std::to_string(data->list_idx) + ". "));
        data->list_idx++;
      }
      list.insert(list.end(), elems.begin(), elems.end());
      return hbox(list);
    }
    case MD_BLOCK_OL:
      return vbox(elems);
    case MD_BLOCK_UL:
      return vbox(elems);
    default:
      return vbox(elems);
  }
}

Component CreatePageRenderer(Element container) {
  auto page = Renderer([container] { 
    constexpr int HORIZONTAL_PADDING = 5;
    constexpr int VERTICAL_PADDING = 2;
    Element h_empty = hbox({}) | size(ftxui::WIDTH, ftxui::EQUAL, HORIZONTAL_PADDING); 
    Element v_empty = hbox({}) | size(ftxui::HEIGHT, ftxui::EQUAL, VERTICAL_PADDING); 
    return vbox({
      v_empty,
      hbox({
        h_empty, container | flex_grow, h_empty
      }) | flex_grow,
      v_empty
  }); });
  return page;
}

void HandleLeaveBlock(MarkdownBlock& block, SlidesData* sd) {
  assert(sd->elem_stack.size() == sd->info_stack.size());

  if (sd->info_stack.empty() && sd->elem_stack.empty())
    return;

  MD_BLOCKTYPE top_type = sd->info_stack.top().block.type;
  // We have already handled this case with <hr>
  if (top_type != block.type)
    return;

  Elements elems = std::move(sd->elem_stack.top());
  sd->elem_stack.pop();
  sd->info_stack.pop();

  Element container;
  // Deal with new slide.
  if (block.type == MD_BLOCK_HR) {
    while (sd->info_stack.size() >= 1) {
      elems = std::move(sd->elem_stack.top());
      container = hbox(elems);
      sd->elem_stack.top().push_back(container);
      sd->elem_stack.pop();
      sd->info_stack.pop();
    }

    container = vbox(elems);
    auto page = CreatePageRenderer(container);
    PrintComponent(page);
    sd->presentation.AddSlide(page);

    sd->elem_stack.push({});
    sd->info_stack.emplace(MarkdownBlock(MD_BLOCK_DOC, nullptr));
    return;
  }

  container = GetBlockType(block.type, elems, sd);

  std::cout << "size stack: " << sd->elem_stack.size() << std::endl;
  if (sd->info_stack.size() >= 1) {
    sd->elem_stack.top().push_back(container);
  } else {
    auto page = CreatePageRenderer(container);
    PrintComponent(page);
    sd->presentation.AddSlide(page);
  }

}

int LeaveBlock(MD_BLOCKTYPE block_type, void* block_detail, void* userdata) {
  auto* sd = static_cast<SlidesData*>(userdata);

  std::cout << "Leaving: " << blockTypeToString(block_type) << '\n';
  assert(!sd->info_stack.empty());
  HandleLeaveBlock(sd->info_stack.top().block, sd);
  return 0;
}

// Callback for entering text.
int EnterText(MD_TEXTTYPE type, const MD_CHAR* str, MD_SIZE size, void* userdata) {
  auto* sd = static_cast<SlidesData*>(userdata);
  std::string text_str(str, size);
  if (!sd->info_stack.empty()) {
    auto& info = sd->info_stack.top();
    if (info.md_type == MarkdownInformation::Block) {
      switch (info.block.type) {
        case MD_BLOCK_P: {
          sd->elem_stack.top().push_back(ftxui::text(text_str));
          break;
        }
        case MD_BLOCK_H: {
          Element bolded_elem = color(Color::Blue, ftxui::text("⣿⣿ " + text_str) | bold);
          sd->elem_stack.top().push_back(bolded_elem);
          break;
        }
        case MD_BLOCK_LI: {
          Element list = ftxui::text(text_str);
          sd->elem_stack.top().push_back(list);
        }
        default:
          break;
      }
    }
  }
  return 0;
}

// Presentation class.
Presentation::Presentation(const std::string& file_path) {
  std::string md_str = load_file(file_path);

  MD_PARSER parser = {
      0,              // abi_version
      0,              // flags
      EnterBlock,     // Callback for entering a block.
      LeaveBlock,     // Callback for leaving a block.
      nullptr,        // EnterSpan placeholder.
      nullptr,        // LeaveSpan placeholder.
      EnterText,      // Callback for reading text.
  };

  auto sd = std::make_unique<SlidesData>(*this);

  if (md_parse(md_str.c_str(), md_str.size(), &parser, sd.get()) != 0) {
    std::cerr << "Error parsing markdown." << std::endl;
    return;
  }

  std::cout << sd->elem_stack.size() << '\n';
  std::cout << "Slides count: " << m_slides.size() << std::endl;

  StartPresentation();
}

void Presentation::AddSlide(const Component& slide) {
  m_slides.push_back(slide);
}

void Presentation::StartPresentation() {
  m_current_slide = 0;  // Start from the first slide
  auto screen = ScreenInteractive::Fullscreen();

  // Define the container that holds the slides
  auto container = Container::Tab(m_slides, &m_current_slide); 

  // Left padding.
  constexpr int HORIZONTAL_PADDING = 10;
  Element v_empty = hbox({}) | size(ftxui::HEIGHT, ftxui::EQUAL, HORIZONTAL_PADDING); 

  auto controls = CatchEvent(container, [&](Event event) {
    if (event == Event::ArrowRight && m_current_slide < m_slides.size() - 1) {
      m_current_slide++;
      return true;
    }
    if (event == Event::ArrowLeft && m_current_slide > 0) {
      m_current_slide--;
      return true;
    }
    return false;
  });

  std::cout << "Slides count: " << m_slides.size() << std::endl;
  screen.Loop(controls);
}

Presentation::~Presentation() = default;

bool Presentation::SavePresentation(const std::string& output_path) {
  // Implement saving logic if required.
  return false;
}
