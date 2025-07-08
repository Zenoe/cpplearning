#include "g_utils.h"
namespace gutils {
std::string glob_to_regex(std::string_view glob) {
    std::string regex_str;
    for (char c : glob) {
        switch (c) {
            case '*': regex_str += ".*"; break;
            case '?': regex_str += '.';  break;
            case '.': regex_str += "\\."; break;
            default:  regex_str += c;
        }
    }
    return regex_str;
}

}
