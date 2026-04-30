#include <OptionsEasy/OptionsEasy.h>

int main(int argc, char* argv[]) {    
    OptionsEasy options_easy;
    int dim{0};
    options_easy.makeOption<int>("dim", dim, "dimension");    
    options_easy.help();
    return 0;
}