#include <Backend/Matcher.h>
#include <ProgramOptionsHeavy.h>

using program_options_heavy::OptionsGroup;
using program_options_heavy::ParserWithSubcommands;
using program_options_heavy::printers::PrettyPrinter;
using program_options_heavy::printers::ProgramSubcommandsPrinter;

int main(int argc, const char *argv[])
{
    namespace po = boost::program_options;
    ParserWithSubcommands subcommands_parser(argc, argv);
    auto run_options = std::make_shared<OptionsGroup>("run group");
    run_options->setGroupDescription("run group description");
    size_t dim;
    // runOptions->addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
    run_options->addPartial("dim,d", std::ref(dim), "hypercube dimension");
    auto gather_options = std::make_shared<OptionsGroup>("gather group");
    gather_options->setGroupDescription("gather group description");
    size_t gather_opt;
    // gatherOptions->addPartialVisible("gather,g", po::value<size_t>(&gather_opt)->default_value(2), "some option for
    // gathering");
    gather_options->addPartial("gather,g", std::ref(gather_opt), "some option for gathering");
    auto common_options = std::make_shared<OptionsGroup>("common group");
    common_options->setGroupDescription("common group description");
    size_t common_value;
    // commonOptions->addPartialVisible("common,c", po::value<size_t>(&common_value)->default_value(2), "common value");
    common_options->addPartial("common,c", std::ref(common_value), "common value");
    subcommands_parser["run"]->addGroup(run_options);
    subcommands_parser["run"]->addGroup(common_options);
    subcommands_parser["gather"]->addGroup(gather_options);
    subcommands_parser["gather"]->addGroup(common_options);

    ProgramSubcommandsPrinter printer;
    auto dom = printer.print(subcommands_parser);

    PrettyPrinter pp;
    dom->accept(pp);
    return 0;
}
