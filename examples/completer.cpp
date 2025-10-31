#include "completer.h"

using program_options_heavy::ParserWithSubcommands;
using program_options_heavy::OptionsGroup;
using program_options_heavy::Completer;


int main() {
    namespace po = boost::program_options;
    auto commands_parser = std::make_shared<ParserWithSubcommands>("compl");
    auto runOptions = std::make_shared<OptionsGroup>("run group");
    size_t dim;
    runOptions->addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2)->required(), "hypercube dimension");
    auto gatherOptions= std::make_shared<OptionsGroup>("gather group");
    size_t gather_opt;
    gatherOptions->addPartialVisible("gather,g", po::value<size_t>(&gather_opt)->default_value(2), "some option for gathering");
    auto commonOptions = std::make_shared<OptionsGroup>("common group");
    size_t common_value;
    commonOptions->addPartialVisible("--common,c", po::value<size_t>(&common_value)->default_value(2), "common value");
    commonOptions->addPartialVisible("d", po::value<size_t>(&common_value)->default_value(2), "common value");

    (*commands_parser)["run"]->addGroup(runOptions);
    (*commands_parser)["run"]->addGroup(commonOptions);
    (*commands_parser)["gather"]->addGroup(gatherOptions);
    (*commands_parser)["gather"]->addGroup(commonOptions);

    const char* argv[] = {"compl", "run", "-d10"};
    commands_parser->parse(3, argv);

    //commands_parser->"run"

    Completer completer(commands_parser);
    std::vector<std::string> variants = completer.getCompletionVariants();
    for(const auto& it : variants) {
        std::cout<<it<<"\n";
    }
    return 0;
}