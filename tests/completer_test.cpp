#include<completer.h>
#include<Parsers/ParserWithSubcommands.h>
#include<gtest/gtest.h>

namespace po = boost::program_options;
using program_options_heavy::ParserWithSubcommands;
using program_options_heavy::OptionsGroup;
using program_options_heavy::Completer;


class CompleterFixture : public ::testing::Test {
    protected:
        std::shared_ptr<ParserWithSubcommands> commands_parser;
        void SetUp() override {
            commands_parser = std::make_shared<ParserWithSubcommands>("exename");
            auto runOptions = std::make_shared<OptionsGroup>("run group");
            size_t dim;
            runOptions->addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2)->required(), "hypercube dimension");
            auto gatherOptions= std::make_shared<OptionsGroup>("gather group");
            size_t gather_opt;
            gatherOptions->addPartialVisible("gather,g", po::value<size_t>(&gather_opt)->default_value(2), "some option for gathering");
            auto commonOptions = std::make_shared<OptionsGroup>("common group");
            size_t common_value;
            commonOptions->addPartialVisible("common,c", po::value<size_t>(&common_value)->default_value(2), "common value");
            commonOptions->addPartialVisible(",v", po::value<size_t>(&common_value)->default_value(2), "common value");

            (*commands_parser)["run"]->addGroup(runOptions);
            (*commands_parser)["run"]->addGroup(commonOptions);
            (*commands_parser)["gather"]->addGroup(gatherOptions);
            (*commands_parser)["gather"]->addGroup(commonOptions);
        }

        void TearDown() override {
        }        
};

TEST_F(CompleterFixture, ExeNameEmpty) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("");
    std::vector<std::string> expected = {"exename"};
    ASSERT_EQ(received, expected);
}

TEST_F(CompleterFixture, ExeNamePartial) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exena");
    std::vector<std::string> expected = {"exename"};
    ASSERT_EQ(received, expected);
}

TEST_F(CompleterFixture, ExeNameMismatch) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exenamee");
    std::vector<std::string> expected = {};
    ASSERT_EQ(received, expected);
}

TEST_F(CompleterFixture, FullMatch) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename");
    std::vector<std::string> expected = {"gather", "run"};
    ASSERT_EQ(received, expected);
}

TEST_F(CompleterFixture, CommandPartialMatch) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename ru");
    std::vector<std::string> expected = {"run"};
    ASSERT_EQ(received, expected);
}

TEST_F(CompleterFixture, CommandFullMatch) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename run");
    std::vector<std::string> expected = {"--dim", "--common", "-v"};
    ASSERT_EQ(received, expected);
}

TEST_F(CompleterFixture, OptionsPartialMatch) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename run --d");
    std::vector<std::string> expected = {"--dim"};
    ASSERT_EQ(received, expected);
}


TEST_F(CompleterFixture, OptionsFullMatch) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename run --dim -v");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}


TEST_F(CompleterFixture, OptionsMatch1) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename run --dim -v --c");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}


TEST_F(CompleterFixture, OptionsMatch2) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename run --dim -v --c");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}

TEST_F(CompleterFixture, OptionsMatch3) {
    Completer completer(commands_parser);
    auto received = completer.getCompletionVariants("exename run --dim -v --cd");
    std::vector<std::string> expected = {};
    ASSERT_EQ(received, expected);
}
