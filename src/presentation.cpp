#include "presentation.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/screen/color.hpp>
#include "ftxui/component/screen_interactive.hpp"
#include <ftxui/screen/terminal.hpp>
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
  int list_idx = 1;

  SlidesData(Presentation& p) : presentation(p) {}
};

// Callback for entering block.
int EnterBlock(MD_BLOCKTYPE block_type, void* block_detail, void* userdata) {
  auto* sd = static_cast<SlidesData*>(userdata);
  DEBUG_PRINT_BLOCK("Entering", block_type);
  sd->info_stack.emplace(MarkdownBlock(block_type, block_detail));
  sd->elem_stack.emplace(); // Push an empty vector for elements.
  return 0;
}

Element GetBlockType(MD_BLOCKTYPE type, Elements elems, SlidesData* data) {
  switch (type) {
    case MD_BLOCK_H:
      return vbox(flexbox(elems), text(""));
    case MD_BLOCK_P:
      return vbox(flexbox(elems), text(""));
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
      return flexbox(list);
    }
    case MD_BLOCK_OL:
      return vbox(elems);
    case MD_BLOCK_UL:
      return vbox(elems);
    default:
      return vbox(elems);
  }
}

Element GetSpanType(MD_SPANTYPE type, Elements elem, SlidesData* data) {
  switch (type) {
    case MD_SPAN_STRONG:
      return hbox(elem) | bold;
    case MD_SPAN_CODE: {
      Elements new_elem = {text(" ")};
      new_elem.reserve(elem.size()+2);
      new_elem.insert(new_elem.end(), elem.begin(), elem.end());
      new_elem.push_back({text(" ")});
      return color(Color::RedLight, hbox(new_elem) | bgcolor(Color::Black));
    }
    default:
      return flexbox(elem);
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
      container = flexbox(elems);
      sd->elem_stack.top().push_back(container);
      sd->elem_stack.pop();
      sd->info_stack.pop();
    }

    container = vbox(elems);
    auto page = CreatePageRenderer(container);
    DEBUG_PRINT_SLIDE(page);
    sd->presentation.AddSlide(page);

    sd->elem_stack.push({});
    sd->info_stack.emplace(MarkdownBlock(MD_BLOCK_DOC, nullptr));
    return;
  }

  container = GetBlockType(block.type, elems, sd);

  if (sd->info_stack.size() >= 1) {
    sd->elem_stack.top().push_back(container);
  } else {
    auto page = CreatePageRenderer(container);
    DEBUG_PRINT_SLIDE(page);
    sd->presentation.AddSlide(page);
  }

}

void HandleLeaveSpan(MarkdownSpan& span, SlidesData* sd) {
  assert(sd->elem_stack.size() == sd->info_stack.size());

  if (sd->info_stack.empty() && sd->elem_stack.empty())
    return;

  MD_SPANTYPE top_type = sd->info_stack.top().span.type;
  // We have already handled this case with <hr>
  if (top_type != span.type)
    return;

  Elements elems = std::move(sd->elem_stack.top());
  sd->elem_stack.pop();
  sd->info_stack.pop();

  Element container; 

  container = GetSpanType(span.type, elems, sd);

  if (sd->info_stack.size() >= 1) {
    sd->elem_stack.top().push_back(container);
  } else {
    auto page = CreatePageRenderer(container);
    DEBUG_PRINT_SLIDE(page);
    sd->presentation.AddSlide(page);
  }

}

int LeaveBlock(MD_BLOCKTYPE block_type, void* block_detail, void* userdata) {
  auto* sd = static_cast<SlidesData*>(userdata);

  DEBUG_PRINT_BLOCK("Leaving", block_type);
  if (block_type == MD_BLOCK_OL)
    sd->list_idx = 1;
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
          MD_BLOCK_H_DETAIL* header_detail = (MD_BLOCK_H_DETAIL*) info.block.detail;
          Element bolded_elem;
          switch (header_detail->level) {
            case 1: 
            bolded_elem = color(Color::Cyan, ftxui::text("██ " + text_str) | bold);
            break;
            case 2:
            bolded_elem = color(Color::Blue, ftxui::text("████ " + text_str) | bold);
            break;
            default:
            bolded_elem = color(Color::Cyan, ftxui::text("██ " + text_str) | bold);
          }
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
    } else if (info.md_type == MarkdownInformation::Span) {
      switch (info.span.type) {
        case MD_SPAN_STRONG: {
          sd->elem_stack.top().push_back(ftxui::text(text_str) | bold);
          break;
        }
        case MD_SPAN_CODE: {
          sd->elem_stack.top().push_back(ftxui::text(text_str));
          break;
        }
        default:
          break;
      }
    }
  }
  return 0;
}

int EnterSpan(MD_SPANTYPE span_type, void* span_detail, void* userdata) {
  auto* sd = static_cast<SlidesData*>(userdata);
  DEBUG_PRINT_SPAN("Entering", span_type);
  sd->info_stack.emplace(MarkdownSpan(span_type, span_detail));
  sd->elem_stack.emplace(); // Push an empty vector for elements.
  return 0;
}

int LeaveSpan(MD_SPANTYPE span_type, void* span_detail, void* userdata) {
  auto* sd = static_cast<SlidesData*>(userdata);
  DEBUG_PRINT_SPAN("Leaving", span_type);
  assert(!sd->info_stack.empty());
  HandleLeaveSpan(sd->info_stack.top().span, sd);
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
      EnterSpan,      // EnterSpan placeholder.
      LeaveSpan,      // LeaveSpan placeholder.
      EnterText,      // Callback for reading text.
  };

  auto sd = std::make_unique<SlidesData>(*this);

  if (md_parse(md_str.c_str(), md_str.size(), &parser, sd.get()) != 0) {
    std::cerr << "Error parsing markdown." << std::endl;
    return;
  }

  StartPresentation();
}

void Presentation::AddSlide(const Component& slide) {
  m_slides.push_back(slide);
}

void Presentation::StartPresentation() {
  m_current_slide = 0;  // Start from the first slide
  auto screen = ScreenInteractive::Fullscreen();

  // Define the container that holds the slides
  auto slide = Container::Tab(m_slides, &m_current_slide); 
  
  // Progress bar at botoom.
  std::string msg = "Ribbit.";
  auto status_bar = Renderer([&] {
        return hbox(text("Slide: " + std::to_string(m_current_slide+1) + "/" + std::to_string(m_slides.size())) | bold,
                    text(" " + msg + " "));
  });

  Element h_empty = hbox({}) | size(ftxui::WIDTH, ftxui::EQUAL, 5); 

  auto container = Container::Vertical({
    slide,
    status_bar
  });

  auto app = Renderer(container, [&] {
    return vbox({
      slide->Render() | flex_grow,
      hbox({
        h_empty, status_bar->Render() | flex_grow | bgcolor(Color::Black), h_empty
      }) | size(HEIGHT, EQUAL, 1),
      hbox() | size(HEIGHT, EQUAL, 2)
    });
  });

  auto controls = CatchEvent(app, [&](Event event) {
    if ((event == Event::ArrowRight || event == Event::Character('l')) && m_current_slide < m_slides.size() - 1) {
      m_current_slide++;
      return true;
    } else if ((event == Event::ArrowLeft || event == Event::Character('h')) && m_current_slide > 0) {
      m_current_slide--;
      return true;
    } else if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }
    return false;
  });

  screen.Loop(controls);
}

Presentation::~Presentation() = default;

bool Presentation::SavePresentation(const std::string& output_path) {
  // Implement saving logic if required.
  return false;
}
