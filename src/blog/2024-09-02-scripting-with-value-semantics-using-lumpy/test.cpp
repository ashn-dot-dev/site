// test.cpp
#include <iostream>
#include <string>
#include <vector>

int main() {
    auto a = std::vector<std::string>{"foo", "bar", "baz"};
    auto b = a; // b becomes a *copy* of a
    a.push_back("lasagne"); // modification of a does not affect b

    std::cout << "a:";
    for (auto element: a) {
        std::cout << " " << element;
    }
    std::cout << "\n";

    std::cout << "b:";
    for (auto element: b) {
        std::cout << " " << element;
    }
    std::cout << "\n";
}
