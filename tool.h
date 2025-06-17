#ifndef TOOL_H_
#define TOOL_H_
#include <iostream>

void print(){
    std:: cout << '\n';
}

template<typename T, typename... Args>
void print(T first, Args... args) {
    std::cout << first << " ";
    print(args...); // Recursively process the remaining arguments
}

#endif // TOOL_H_
