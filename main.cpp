#include "dine.hpp"
#include <iostream>
constexpr auto fallback{5};
int main(int argc, char** argv) {
    auto num{argc > 1 ? (std::atoi(argv[1])) : fallback};
    Table table(!int{num}?fallback:num);
    return 0;
}
