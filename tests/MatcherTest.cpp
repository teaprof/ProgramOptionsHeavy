#include <Backend/Matcher.h>
#include <gtest/gtest.h>

class MatcherFixture : public ::testing::Test {
    protected:
        std::shared_ptr<AbstractOption> options;
        std::shared_ptr<AbstractNamedOption> common_option;
        std::shared_ptr<Alternatives> command;
        void SetUp() override {
            common_option = std::make_shared<AbstractNamedOption>("--common", "-c");
            options = std::make_shared<Compatibles>();
            options->addUnlock(common_option);            
            auto runOptions = std::make_shared<AbstractNamedOption>("run");
            runOptions->addUnlock(
                std::make_shared<AbstractNamedOption>("--dim", "-d")
            );
            auto gatherOptions= std::make_shared<AbstractNamedOption>("gather");
            gatherOptions->addUnlock(std::make_shared<AbstractNamedOption>("--gatheropt", "-g"));
            command = std::make_shared<Alternatives>(runOptions, gatherOptions);
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
    EXPECT_THROW(parser.parse({}), std::runtime_error);
    EXPECT_TRUE(parser.parse({"run"}));
    EXPECT_TRUE(parser.parse({"gather", "-g"}));
    EXPECT_FALSE(parser.parse({"run", "gather", "-g"}));
    EXPECT_TRUE(parser.parse({"run", "--common"}));
    EXPECT_TRUE(parser.parse({"--common", "run", "-d"}));
}
