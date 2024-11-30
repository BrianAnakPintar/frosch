#include "dashboard.hpp"
#include "image_view.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

Dashboard::Dashboard() { 
  Run();
}

ftxui::ButtonOption CustomButton() {
    using namespace ftxui;

    ButtonOption option;
    option.transform = [](const EntryState& s) {
        const std::string t = s.focused ? "> " + s.label + " "  //
                                        : "  " + s.label + " ";
        if (s.focused)
            return text(t) | color(Color::Cyan);
        return text(t);
    };
    return option;
}

void Dashboard::Run() {
  using namespace ftxui;

  auto screen = ScreenInteractive::Fullscreen();

  int tab_num = 0;
  auto buttons = Container::Vertical({
    Button("Open a presentation", [&] {}, CustomButton()) | center,
    Button("Settings", [&] {tab_num=1;}, CustomButton()) | center,
    Button("Quit", [&] {screen.Exit();}, CustomButton()) | center,
  });

  auto menu = Renderer(buttons, [&] {
    return vbox({
      image_view("../examples/img_frieren.jpg") | size(ftxui::WIDTH, ftxui::LESS_THAN, 30) 
                                                | size(ftxui::HEIGHT, ftxui::LESS_THAN, 15)
                                                | center,
      text("") | center,
      buttons->Render() | center
    }) | center;
  });

  

  // === START
  // Color Options
  std::vector<std::string> colors = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta"};
  int selected_color = 0;

  // Toggle Option
  bool dark_mode = false;

  // Custom Input for Color (e.g., Hex)
  std::string custom_color = "#FFFFFF";

  // Container for all interactive elements
  auto container = Container::Vertical({});

  // Dropdown for colors
  auto color_selector = Dropdown(&colors, &selected_color);
  container->Add(color_selector);

  // Toggle for dark mode
  auto dark_mode_toggle = Checkbox("Enable Dark Mode", &dark_mode);
  container->Add(dark_mode_toggle);

  // Input for custom color
  auto custom_color_input = Input(&custom_color, "Custom Color");
  container->Add(custom_color_input);

  // Buttons for Save and Cancel
  auto save_button = Button("Save", []() {
    // Save settings
  });
  auto cancel_button = Button("Cancel", []() {
    // Discard changes
  });
  container->Add(save_button);
  container->Add(cancel_button);

  // Main renderer
  auto set = Renderer(container, [&]() {
    return vbox({
      text("Settings") | bold | center,
      hbox(text("Select Color: "), color_selector->Render()),
      hbox(text("Dark Mode: "), dark_mode_toggle->Render()),
      hbox(text("Custom Color: "), custom_color_input->Render()),
      separator(),
      hbox(save_button->Render() | center, cancel_button->Render() | center)
    });
  });

  // === END

  auto final = Container::Tab({
    menu,
   set 
  }, &tab_num);

  screen.Loop(final);
}
