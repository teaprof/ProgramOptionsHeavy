#include <Backend/Option.h>
#include <Completer/Completer2.h>
#include <gtest/gtest.h>

class Completer2Fixture : public ::testing::Test
{
  protected:
    std::shared_ptr<LiteralString> exename_with_options_;
    void SetUp() override
    {
        auto run_options = std::make_shared<OptionsGroup2>();
        run_options->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--dim", "-d"));
        auto gather_options = std::make_shared<OptionsGroup2>();
        gather_options->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--gather", "-g"));

        auto common_options = std::make_shared<OptionsGroup2>();
        common_options->addUnlock(std::make_shared<NamedOption>("--common", "-c"));
        common_options->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--value", "-v"));

        auto options = std::make_shared<OneOf>();
        options
            ->addAlternative(std::make_shared<LiteralString>("run")->addUnlock(run_options)->addUnlock(common_options))
            ->addAlternative(
                std::make_shared<LiteralString>("gather")->addUnlock(gather_options)->addUnlock(common_options));

        exename_with_options_ = std::make_shared<LiteralString>("exename");
        exename_with_options_->addUnlock(options);
    }

    void TearDown() override
    {
    }
};

TEST_F(Completer2Fixture, ExeNameEmpty)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("");
    std::vector<std::string> expected = {"exename"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, ExeNamePartial)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exena");
    std::vector<std::string> expected = {"exename"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, ExeNameMismatch)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exenamee");
    std::vector<std::string> expected = {};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, FullMatch)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename");
    std::vector<std::string> expected = {"run", "gather"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, CommandPartialMatch)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename ru");
    std::vector<std::string> expected = {"run"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, CommandFullMatch)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename run");
    std::vector<std::string> expected = {"--dim", "--common", "--value"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, OptionsPartialMatch)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename run --d");
    std::vector<std::string> expected = {"--dim"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, OptionsFullMatch)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename run --dim 2 -v 1");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, OptionsMatchPartial1)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename run --dim 1 -v 2 --c");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, OptionsMatchPartial2)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename run --dim 1 -v 2 --c");
    std::vector<std::string> expected = {"--common"};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, OptionsNotMatch)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename run --dim -v --cd");
    std::vector<std::string> expected = {};
    ASSERT_EQ(received, expected);
}

TEST_F(Completer2Fixture, WaitForValue)
{
    Completer completer(exename_with_options_);
    auto received = completer.getCompletionVariants("exename run --dim");
    std::vector<std::string> expected = {};
    ASSERT_EQ(received, expected);
}
