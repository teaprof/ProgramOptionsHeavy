#include <ProgramOptionsHeavy.h>
#include <gtest/gtest.h>

using program_options_heavy::DynamicOptionsGroup;
using program_options_heavy::ParserWithSubcommands;
using program_options_heavy::printers::PrettyPrinter;
using program_options_heavy::printers::ProgramSubcommandsPrinter;

TEST(PROGRAMMODEOPTIONS, PARSE) {
    namespace po = boost::program_options;
    ParserWithSubcommands subcommands_parser("programname");
    auto run_options = std::make_shared<DynamicOptionsGroup>("run group");
    size_t dim;
    // runOptions->addPartialVisible("dim,d",
    // po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
    run_options->addPartial("dim,d", std::ref(dim), "hypercube dimension");
    auto gather_options = std::make_shared<DynamicOptionsGroup>("gather group");
    size_t gather_opt;
    // gatherOptions->addPartialVisible("gather,g",
    // po::value<size_t>(&gather_opt)->default_value(2), "some option for
    // gathering");
    gather_options->addPartial("gather,g", std::ref(gather_opt), "some option for gathering");
    auto common_options = std::make_shared<DynamicOptionsGroup>("common group");
    size_t common_value;
    // commonOptions->addPartialVisible("common,c",
    // po::value<size_t>(&common_value)->default_value(2), "common value");
    common_options->addPartial("common,c", std::ref(common_value), "common value");
    subcommands_parser["run"]->addGroup(run_options);
    subcommands_parser["run"]->addGroup(common_options);
    subcommands_parser["gather"]->addGroup(gather_options);
    subcommands_parser["gather"]->addGroup(common_options);

    const char* argv1[] = {"prgmname", "run", "-d", "10", "-c", "20"};
    subcommands_parser.parse(6, argv1);
    ASSERT_EQ(dim, 10);
    ASSERT_EQ(common_value, 20);

    const char* argv2[] = {"prgmname", "gather", "-g", "15", "-c", "30"};
    subcommands_parser.parse(6, argv2);
    ASSERT_EQ(gather_opt, 15);
    ASSERT_EQ(common_value, 30);

    const char* argv3[] = {"prgmname", "run", "-g", "15", "-c", "30"};
    EXPECT_THROW(subcommands_parser.parse(6, argv3), UnknownNamedOption);
}