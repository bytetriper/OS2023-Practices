#include "utils.h"
#include <cstring>
void print_with_escape(std::string str) {
    std::cout << "String with escape sequences: ";
    for (char c : str) {
        switch (c) {
            case '\n':
                std::cout << "\\n";
                break;
            case '\t':
                std::cout << "\\t";
                break;
            case '\"':
                std::cout << "\\\"";
                break;
            case '\\':
                std::cout << "\\\\";
                break;
            // Add other cases here for additional escape sequences
            default:
                std::cout << c;
        }
    }
    std::cout << std::endl;
}