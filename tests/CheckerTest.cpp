#include <Backend/ValueSemantics.h>
#include <Backend/Checker.h>
#include <Backend/Printer.h>

#include <gtest/gtest.h>

TEST(CheckerTest, Simple1) {
    auto options = std::make_shared<NamedOption>("--opt", "-o");
    Checker checker;
    EXPECT_NO_THROW(options->accept(checker));
}

TEST(CheckerTest, Simple2) {
    auto options = std::make_shared<NamedOption>("--opt1", "-o");
    options->addUnlock(std::make_shared<NamedOption>("--opt2"));
    Checker checker;
    EXPECT_NO_THROW(options->accept(checker));

    options->addUnlock(std::make_shared<NamedOption>("--opt2"));
    EXPECT_THROW(options->accept(checker), DuplicateOption);
}

TEST(CheckerTest, SimplePositionalOptions) {
    auto options = std::make_shared<NamedOption>("--opt1", "-o");
    options->addUnlock(std::make_shared<NamedOption>("--opt2"));
    options->addUnlock(std::make_shared<PositionalOptionWithValue<int>>());
    options->addUnlock(std::make_shared<PositionalOptionWithValue<int>>());
    Checker checker;
    EXPECT_NO_THROW(options->accept(checker));
}

TEST(CheckerTest, TooManyPositionalOptions) {
    auto options = std::make_shared<OptionsGroup2>();
    auto posopt = std::make_shared<PositionalOptionWithValue<int>>();
    posopt->setMaxOccurreneCount(2);
    options->addUnlock(posopt);
    options->addUnlock(std::make_shared<PositionalOptionWithValue<int>>());
    Checker checker;
    EXPECT_THROW(options->accept(checker), MultipleOccurenceOnlyForLastPosopt);
}

TEST(CheckerTest, OneOf) {
    auto option = std::make_shared<OneOf>(
        std::make_shared<LiteralString>("run"),
        std::make_shared<LiteralString>("gather"),
        std::make_shared<LiteralString>("clean")
    );
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Alternatives2) {
    auto option = std::make_shared<OneOf>(
        std::make_shared<LiteralString>("run")->addUnlock(std::make_shared<NamedOption>("--opt1"))->addUnlock(std::make_shared<NamedOption>("--opt2")),
        std::make_shared<LiteralString>("gather")->addUnlock(std::make_shared<NamedOption>("--opt1")),
        std::make_shared<LiteralString>("clean")->addUnlock(std::make_shared<NamedOption>("--opt2"))
    );
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Alternatives3) {    
    auto option = std::make_shared<AbstractOption>()->
        addUnlock(std::make_shared<NamedOption>("--opt1"))->
        addUnlock(    
            std::make_shared<OneOf>(
                std::make_shared<LiteralString>("run")->addUnlock(std::make_shared<NamedOption>("--opt1")),
                std::make_shared<LiteralString>("gather")->addUnlock(std::make_shared<NamedOption>("--opt2")),
                std::make_shared<LiteralString>("clean")->addUnlock(std::make_shared<NamedOption>("--opt1"))                
            )
        );
    Checker checker;
    EXPECT_THROW(option->accept(checker), DuplicateOption);
}


TEST(CheckerTest, OneOfComplex) {
    auto helpOption = std::make_shared<NamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<OneOf>(
        helpOption,
        std::make_shared<LiteralString>("run")->
            addUnlock(std::make_shared<NamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<NamedOption>("--mIntervalsPerDim", "-m"))->
            addUnlock(helpOption),
        std::make_shared<LiteralString>("gather")->addUnlock(helpOption)
    );
    Checker checker;
    EXPECT_NO_THROW(hypercubeOptions->accept(checker));
}


TEST(CheckerTest, RepeatedOption) {
    auto helpOption = std::make_shared<NamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<OneOf>(
        helpOption,
        std::make_shared<LiteralString>("run")->
            addUnlock(std::make_shared<NamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<NamedOption>("--mIntervalsPerDim", "-m"))->
            addUnlock(helpOption)->
            addUnlock(helpOption),
        std::make_shared<LiteralString>("gather")->addUnlock(helpOption)
    );
    Checker checker;
    EXPECT_THROW(hypercubeOptions->accept(checker), DuplicateOption);
}

TEST(CheckerTest, PositionalWithAllowedOptions) {
    auto options = std::make_shared<PositionalOptionWithValue<std::string>>();
    options->valueSemantics().unlocks("run");
    options->valueSemantics().unlocks("gather");
    options->valueSemantics().unlocks("clean");
    Checker checker;
    EXPECT_NO_THROW(options->accept(checker));
}

TEST(CheckerTest, PositionalWithAllowedOptions2) {
    auto options = std::make_shared<PositionalOptionWithValue<std::string>>();
    options->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--opt1"));
    options->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--opt2"));
    options->valueSemantics().unlocks("gather").push_back(std::make_shared<NamedOption>("--opt1"));
    options->valueSemantics().unlocks("clean").push_back(std::make_shared<NamedOption>("--opt2"));
    Checker checker;
    EXPECT_NO_THROW(options->accept(checker));
}

TEST(CheckerTest, OneOfAlternatives3) {    
    auto option = std::make_shared<AbstractOption>()->
        addUnlock(std::make_shared<NamedOption>("--opt1"))->
        addUnlock(    
            std::make_shared<OneOf>(
                std::make_shared<LiteralString>("run")->addUnlock(std::make_shared<NamedOption>("--opt1")),
                std::make_shared<LiteralString>("gather")->addUnlock(std::make_shared<NamedOption>("--opt2")),
                std::make_shared<LiteralString>("clean")->addUnlock(std::make_shared<NamedOption>("--opt1"))                
            )
        );
    Checker checker;
    EXPECT_THROW(option->accept(checker), DuplicateOption);
}


TEST(CheckerTest, Complex) {
    auto helpOption = std::make_shared<NamedOption>("--help", "-h");
    auto options = std::make_shared<PositionalOptionWithValue<std::string>>();
    //option->addUnlock(helpOption);
    options->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--dim", "-d"));
    options->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--mIntervalsPerDim", "-m"));
    options->valueSemantics().unlocks("run").push_back(helpOption);
    options->valueSemantics().unlocks("gather").push_back(helpOption);
    options->valueSemantics().unlocks("clean").push_back(helpOption);

    Checker checker;
    EXPECT_NO_THROW(options->accept(checker));
}

TEST(CheckerTest, Cycle) {
    auto option1 = std::make_shared<NamedOption>("--opt1");
    auto option2 = std::make_shared<NamedOption>("--opt2");
    option1->addUnlock(option2);
    option2->addUnlock(option1);
    Checker checker;
    EXPECT_THROW(option1->accept(checker), DuplicateOption);
}

TEST(CheckerTest, NestedAlternatives) {
    auto alt_nested_1 = std::make_shared<PositionalOptionWithValue<std::string>>();
    alt_nested_1->valueSemantics().unlocks("alt1").push_back(std::make_shared<NamedOption>("--opt1"));
    alt_nested_1->valueSemantics().unlocks("alt2").push_back(std::make_shared<NamedOption>("--opt2"));
    auto alt_nested_2 = std::make_shared<PositionalOptionWithValue<std::string>>();
    alt_nested_2->valueSemantics().unlocks("alt1").push_back(std::make_shared<NamedOption>("--opt1"));
    alt_nested_2->valueSemantics().unlocks("alt2").push_back(std::make_shared<NamedOption>("--opt2"));
    auto opts = std::make_shared<PositionalOptionWithValue<std::string>>();
    opts->valueSemantics().unlocks("alt1").push_back(alt_nested_1);
    opts->valueSemantics().unlocks("alt2").push_back(alt_nested_2);

    Checker checker;
    opts->accept(checker);
}

TEST(CheckerTest, NestedAlternativesWithConflict) {
    auto opts_nested = std::make_shared<OneOf>();
    opts_nested->alternatives.push_back(
        std::make_shared<LiteralString>("alt1")->addUnlock(std::make_shared<NamedOption>("--dim"))
    );
    auto opts = std::make_shared<OneOf>();
    opts->alternatives.push_back(
        std::make_shared<LiteralString>("alt")->addUnlock(opts_nested)->addUnlock(std::make_shared<NamedOption>("--dim"))
    );

    Checker checker;
    EXPECT_THROW(opts->accept(checker), DuplicateOption);
}