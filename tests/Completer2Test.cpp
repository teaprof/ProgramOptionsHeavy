#include<Backend/Option.h>
#include<Backend/Completer2.h>
#include<gtest/gtest.h>

class Completer2Fixture : public ::testing::Test {
    protected:
        std::shared_ptr<LiteralString> exename_with_options;
        void SetUp() override {
            auto runOptions = std::make_shared<OptionsGroup>();
            runOptions->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--dim", "-d"));
            auto gatherOptions= std::make_shared<OptionsGroup>();
            gatherOptions->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--gather", "-g"));

            auto commonOptions = std::make_shared<OptionsGroup>();
            commonOptions->addUnlock(std::make_shared<NamedOption>("--common", "-c"));
            commonOptions->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--value", "-v"));

            auto options = std::make_shared<OneOf>();
            options->addAlternative(
                std::make_shared<LiteralString>("run")->addUnlock(runOptions)->addUnlock(commonOptions)
            )->addAlternative(
                std::make_shared<LiteralString>("gather")->addUnlock(gatherOptions)->addUnlock(commonOptions));
            
            exename_with_options = std::make_shared<LiteralString>("exename");
            exename_with_options->addUnlock(options);
            
        }

        void TearDown() override {
        }        
};

TEST_F(Completer2Fixture, ExeNameEmpty) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("");
    std::vector<std::string> expected = {"exename"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, ExeNamePartial) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exena");
    std::vector<std::string> expected = {"exename"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, ExeNameMismatch) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exenamee");
    std::vector<std::string> expected = {};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, FullMatch) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename");
    std::vector<std::string> expected = {"run", "gather"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, CommandPartialMatch) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename ru");
    std::vector<std::string> expected = {"run"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, CommandFullMatch) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename run");
    std::vector<std::string> expected = {"--dim", "--common", "--value"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, OptionsPartialMatch) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename run --d");
    std::vector<std::string> expected = {"--dim"};
    ASSERT_EQ(received, expected);
}


TEST_F(Completer2Fixture, OptionsFullMatch) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename run --dim 2 -v 1");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}


TEST_F(Completer2Fixture, OptionsMatchPartial1) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename run --dim 1 -v 2 --c");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}


TEST_F(Completer2Fixture, OptionsMatchPartial2) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename run --dim 1 -v 2 --c");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, OptionsNotMatch) {
    Completer completer(exename_with_options);
    auto received = completer.getCompletionVariants("exename run --dim -v --cd");
    std::vector<std::string> expected = {};
    ASSERT_EQ(received, expected);
}
