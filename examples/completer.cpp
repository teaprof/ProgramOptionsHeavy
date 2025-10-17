#include "completer.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::vector<std::string> names{"COMP_LINE", "COMP_WORDS", "COMP_CWORD", "COMP_POINT", "COMP_KEY", "COMP_TYPE"};
    for(const auto& it : names) {
        if(auto str = getenv(it.c_str())) {
            std::cout<<it<<"="<<str<<"\n";
        } else {
            std::cout<<it<<"=NULL\n";
        }
    }
    std::cout<<"runsn\njoke\ncollect\n";
    return 0;
}