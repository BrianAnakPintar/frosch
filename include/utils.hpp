#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <md4c.h>
#include <ftxui/component/component_base.hpp>

// Toggle Define and Undefine Debug by uncommenting.
/*#define DEBUG*/

#ifdef DEBUG
    #define DEBUG_PRINT_BLOCK(msg, block_type) \
        std::cout << msg << ": " << blockTypeToString(block_type) << '\n';
    #define DEBUG_PRINT_SPAN(msg, span_type) \
        std::cout << msg << ": " << spanTypeToString(span_type) << '\n';
    #define DEBUG_PRINT_SLIDE(slide)\
        PrintComponent(slide);
#else
    #define DEBUG_PRINT_BLOCK(msg, block_type) \
        do {} while (0)
    #define DEBUG_PRINT_SPAN(msg, span_type) \
        do {} while (0)
    #define DEBUG_PRINT_SLIDE(slide)\
        do {} while (0)
#endif

std::string blockTypeToString(MD_BLOCKTYPE blockType);
std::string spanTypeToString(MD_SPANTYPE spanType);
void PrintComponent(const ftxui::Component& component);
std::string load_file(const std::string& filepath);

#endif // !UTILS_HPP
