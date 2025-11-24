#include <ProgramOptionsHeavy.h>
#include <gtest/gtest.h>

using program_options_heavy::ParserWithSubcommands;
using program_options_heavy::OptionsGroup;
using program_options_heavy::printers::ProgramSubcommandsPrinter;
using program_options_heavy::printers::PrettyPrinter;


TEST(PROGRAMMODEOPTIONS, PARSE) {
    namespace po = boost::program_options;
    ParserWithSubcommands subcommands_parser("programname");
    auto runOptions = std::make_shared<OptionsGroup>("run group");
    size_t dim;    
    //runOptions->addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");    
    runOptions->addPartial("dim,d", std::ref(dim), "hypercube dimension");
    auto gatherOptions= std::make_shared<OptionsGroup>("gather group");
    size_t gather_opt;
    //gatherOptions->addPartialVisible("gather,g", po::value<size_t>(&gather_opt)->default_value(2), "some option for gathering");
    gatherOptions->addPartial("gather,g", std::ref(gather_opt), "some option for gathering");
    auto commonOptions = std::make_shared<OptionsGroup>("common group");
    size_t common_value;
    //commonOptions->addPartialVisible("common,c", po::value<size_t>(&common_value)->default_value(2), "common value");    
    commonOptions->addPartial("common,c", std::ref(common_value), "common value");    
    subcommands_parser["run"]->addGroup(runOptions);
    subcommands_parser["run"]->addGroup(commonOptions);
    subcommands_parser["gather"]->addGroup(gatherOptions);
    subcommands_parser["gather"]->addGroup(commonOptions);

    const char* argv1[] = {"prgmname", "run", "-d", "10",  "-c", "20"};
    subcommands_parser.parse(6, argv1);
    ASSERT_EQ(dim, 10);
    ASSERT_EQ(common_value, 20);

    const char* argv2[] = {"prgmname", "gather", "-g", "15",  "-c", "30"};
    subcommands_parser.parse(6, argv2);
    ASSERT_EQ(gather_opt, 15);
    ASSERT_EQ(common_value, 30);

    const char* argv3[] = {"prgmname", "run", "-g", "15",  "-c", "30"};
    EXPECT_THROW(subcommands_parser.parse(6, argv3), UnknownNamedOption);
}