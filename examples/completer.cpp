#include "completer.h"

using program_options_heavy::SubcommandsParser;
using program_options_heavy::OptionsGroup;
using program_options_heavy::Completer;


int main() {

    std::optional<std::string> a, b;
    a = "a";
    b = "a";
    if(a == b) {
        std::cout<<"="<<std::endl;
    }

    namespace po = boost::program_options;
    auto subcommands_parser = std::make_shared<SubcommandsParser>();
    auto runOptions = std::make_shared<OptionsGroup>("run group");
    size_t dim;
    runOptions->addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
    auto gatherOptions= std::make_shared<OptionsGroup>("gather group");
    size_t gather_opt;
    gatherOptions->addPartialVisible("gather,g", po::value<size_t>(&gather_opt)->default_value(2), "some option for gathering");
    auto commonOptions = std::make_shared<OptionsGroup>("common group");
    size_t common_value;
    commonOptions->addPartialVisible("common,c", po::value<size_t>(&common_value)->default_value(2), "common value");

    //subcommands_parser->"run"

    Completer completer(subcommands_parser);
    std::vector<std::string> variants = completer.complete();
    for(const auto& it : variants) {
        std::cout<<it<<"\n";
    }
    return 0;
}