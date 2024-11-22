cmake_minimum_required(VERSION 3.22)

project(Frosch
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(md4c REQUIRED)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v5.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
# ------------------------------------------------------------------------------

# Fetch MD4C source code
FetchContent_Declare(
    md4c
    GIT_REPOSITORY https://github.com/mity/md4c.git
    GIT_TAG        release-0.5.2
)
FetchContent_MakeAvailable(md4c)

# --- Add ftxui-image-view -----------------------------------------------------
add_subdirectory(external/ftxui-image-view)

# --- Main executable ----------------------------------------------------------
add_executable(Frosch src/main.cpp
                      src/presentation.cpp
                      src/utils.cpp)

target_include_directories(Frosch PRIVATE src)
target_include_directories(Frosch PRIVATE include)

target_link_libraries(Frosch PRIVATE md4c::md4c)

target_link_libraries(Frosch
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE ftxui-image-view
)
