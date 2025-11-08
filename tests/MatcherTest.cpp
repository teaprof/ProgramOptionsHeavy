#include <Backend/Matcher.h>
#include <gtest/gtest.h>

class MatcherFixture : public ::testing::Test {
    protected:
        std::shared_ptr<AbstractOption> options;
        std::shared_ptr<NamedOption> common_option;
        std::shared_ptr<OneOf> command;
        void SetUp() override {
            common_option = std::make_shared<NamedOption>("--common", "-c");
            options = std::make_shared<OptionsGroup>();
            options->addUnlock(common_option);            
            auto runOptions = std::make_shared<NamedCommand>("run");
            runOptions->addUnlock(
                std::make_shared<NamedOption>("--dim", "-d")
            );
            auto gatherOptions= std::make_shared<NamedCommand>("gather");
            gatherOptions->addUnlock(std::make_shared<NamedOption>("--gatheropt", "-g"));
            command = std::make_shared<OneOf>(runOptions, gatherOptions);
            options->addUnlock(command);
        }

        void TearDown() override {
        }        
};

TEST_F(MatcherFixture, Test1) {
    Parser parser(options);
    EXPECT_TRUE(parser.parse({}));
}

TEST_F(MatcherFixture, Test2) {
    Parser parser(options);
    command->setRequired(true);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    EXPECT_TRUE(parser.parse({"run"}));
    EXPECT_TRUE(parser.parse({"gather", "-g"}));
    EXPECT_FALSE(parser.parse({"run", "gather", "-g"}));
    EXPECT_TRUE(parser.parse({"run", "--common"}));
    EXPECT_TRUE(parser.parse({"--common", "run", "-d"}));
}

TEST(Matcher, OptionRequired) {
    auto opt = std::make_shared<NamedOption>("--opt1");
    opt->setRequired(true);

    Parser parser(opt);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    EXPECT_TRUE(parser.parse({"--opt1"}));
}
