#ifndef PRESENTATION_HPP
#define PRESENTATION_HPP

#include <string>
#include <md4c.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/screen/color.hpp>
#include "ftxui/component/screen_interactive.hpp"
#include <ftxui/screen/terminal.hpp>
#include "ftxui/dom/elements.hpp"
#include <stack>
#include <string>
#include <vector>
#include <cassert>
#include <string>


// This class contains the UI elements that will be generated.
class Presentation {

public:
  /* 
  *
  * Creates a presentation from a markdown or json file. Initializing Various components.
  * @param: file_path points to either a valid JSON or Markdown file.
  *
  */
  Presentation(const std::string& file_path);
  ~Presentation();

  /* 
  *
  * Saves the current presentation as a JSON file.
  * @param: output_path is the filename which to write the file to.
  * @return: true if save successful, false otherwise.
  */
  bool SavePresentation(const std::string& output_path);

  /*
  *
  * Adds a slide.
  *
  */
  void AddSlide(const ftxui::Component& slide);
  
  // Stores the current directory
  std::string presentation_path;

private:
  // Slides contains all the slides for the presentation.
  std::vector<ftxui::Component> m_slides;
  
  // The current theme for the presentation
  std::string m_theme;

  // What slide we are currently on.
  int m_current_slide;

  // Starts the presentation.
  void StartPresentation();
};

#endif // !PRESENTATION_HPP
