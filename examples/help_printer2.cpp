#include <Backend/Parser.h>
#include <ProgramOptionsHeavy.h>

using program_options_heavy::HeavyOptionsGroup;
using program_options_heavy::ParserWithSubcommands;
using program_options_heavy::printers::PrettyPrinter;
using program_options_heavy::printers::ProgramSubcommandsFormatter;

int main(int argc, const char* argv[]) {
    namespace po = boost::program_options;
        
    auto run_options = std::make_shared<HeavyOptionsGroup>("run group");
    run_options->setGroupDescription("run group description");
    size_t dim;
    // runOptions->addPartialVisible("dim,d",
    // po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
    run_options->addPartial("dim,d", std::ref(dim), "hypercube dimension");
    run_options->addPartial("mIntervalsPerDim,m", std::ref(dim), "number of intervals per dimension");
    
    auto gather_options = std::make_shared<HeavyOptionsGroup>("gather group");
    gather_options->setGroupDescription("gather group description");
    size_t gather_opt;
    // gatherOptions->addPartialVisible("gather,g",
    // po::value<size_t>(&gather_opt)->default_value(2), "some option for
    // gathering");
    gather_options->addPartial("gather,g", std::ref(gather_opt), "some option for gathering");
    
    auto common_options = std::make_shared<HeavyOptionsGroup>("common group");
    common_options->setGroupDescription("common group description");
    size_t common_value;
    // commonOptions->addPartialVisible("common,c",
    // po::value<size_t>(&common_value)->default_value(2), "common value");
    common_options->addPartial("common,c", std::ref(common_value), "common value");
    common_options->addPartial("verbose,v", std::ref(common_value), "print detailed info");
    common_options->addPartial("--vverbose", std::ref(common_value), "print more detailed info");
    
    ParserWithSubcommands subcommands_parser(argc, argv);
    subcommands_parser["run"]->addGroup(run_options);
    subcommands_parser["run"]->addGroup(common_options);
    subcommands_parser["gather"]->addGroup(gather_options);
    subcommands_parser["gather"]->addGroup(common_options);

    ProgramSubcommandsFormatter formatter;
    auto dom = formatter.print(subcommands_parser);

    PrettyPrinter printer;
    dom->accept(printer);
    return 0;
}
