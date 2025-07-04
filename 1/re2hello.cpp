#include <iostream>
#include <string>
#include <re2/re2.h>

#include <memory>
int main() {
    std::string s = "hello world";
    // 创建一个RE2正则表达式对象
    RE2 pattern("hello.*world");


    if (!pattern.ok()) {
        std::cerr << "Invalid regex pattern: " << pattern.error() << std::endl;
        return 1;
    }

    if (RE2::FullMatch(s, pattern)) {
        std::cout << "Match!" << std::endl;
    } else {
        std::cout << "No match!" << std::endl;
    }

    return 0;
}
