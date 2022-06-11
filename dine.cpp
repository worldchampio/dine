#include "def.hpp"
#include <string>
#include <sstream>

int main(int argc, char** argv) {

    int num;
    std::stringstream ss;

    std::string numstr {argc > 1 ? (argv[1]) : "5"};
    ss << numstr;
    ss >> num;
    Table table(num);
    
    return 0;
}
