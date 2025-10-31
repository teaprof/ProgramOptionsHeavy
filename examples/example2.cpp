#include<ProgramOptionsHeavy.h>

using program_options_heavy::ParserWithSubcommands;
using program_options_heavy::OptionsGroup;
using program_options_heavy::printers::ProgramSubcommandsPrinter;
using program_options_heavy::printers::PrettyPrinter;

int main(int argc, const char* argv[]) {
    namespace po = boost::program_options;
    ParserWithSubcommands subcommands_parser(argc, argv);
    auto runOptions = std::make_shared<OptionsGroup>("run group");
    size_t dim;
    runOptions->addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
    auto gatherOptions= std::make_shared<OptionsGroup>("gather group");
    size_t gather_opt;
    gatherOptions->addPartialVisible("gather,g", po::value<size_t>(&gather_opt)->default_value(2), "some option for gathering");
    auto commonOptions = std::make_shared<OptionsGroup>("common group");
    size_t common_value;
    commonOptions->addPartialVisible("common,c", po::value<size_t>(&common_value)->default_value(2), "common value");    
    subcommands_parser["run"]->addGroup(runOptions);
    subcommands_parser["run"]->addGroup(commonOptions);
    subcommands_parser["gather"]->addGroup(gatherOptions);
    subcommands_parser["gather"]->addGroup(commonOptions);

    ProgramSubcommandsPrinter printer;
    auto dom = printer.print(subcommands_parser);

    PrettyPrinter pp;
    dom->accept(pp);
    return 0;
}

