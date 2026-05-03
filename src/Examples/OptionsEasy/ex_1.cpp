#include <OptionsEasy/OptionsEasy.h>
#include <Help/TextExtractors.h>

int main(int argc, char* argv[]) {    
    OptionsEasy options_easy;
    int dim{0};
    options_easy.makeOption<int>("dim", dim, "dimension");    
    HelpStringsStorage help_storage = options_easy.help();    
    SimpleExtractor extractor;
    ProgramUsageMindMap mindmap = extractor.extract(help_storage, options_easy.opt());

    return 0;
}