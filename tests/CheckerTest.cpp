#include <Backend/ValueSemantics.h>
#include <Checker/Checker.h>
#include <Backend/Printer.h>

#include <gtest/gtest.h>

TEST(CheckerTest, Simple1) {
    auto options = std::make_shared<NamedOption>("--opt", "-o");
    Checker checker;
    EXPECT_NO_THROW(checker(options));
}

TEST(CheckerTest, Simple2) {
    auto options = std::make_shared<NamedOption>("--opt1", "-o");
    options->addUnlock(std::make_shared<NamedOption>("--opt2"));
    Checker checker;
    EXPECT_NO_THROW(checker(options));

    options->addUnlock(std::make_shared<NamedOption>("--opt2"));
    EXPECT_THROW(checker(options), DuplicateOptionName);
}

TEST(CheckerTest, DuplicateOptionPtrDetected) {
    auto option = std::make_shared<NamedOption>("--opt1", "-o");
    auto options = std::make_shared<NamedOption>("--opt2", "-p");
    options->addUnlock(option)->addUnlock(option);
    Checker checker;
    EXPECT_THROW(checker(options), DuplicateOptionPtrDetected);
}

TEST(CheckerTest, SimplePositionalOptions) {
    auto options = std::make_shared<NamedOption>("--opt1", "-o");
    options->addUnlock(std::make_shared<NamedOption>("--opt2"));
    options->addUnlock(std::make_shared<PositionalOptionWithValue<int>>());
    options->addUnlock(std::make_shared<PositionalOptionWithValue<int>>());
    Checker checker;
    EXPECT_NO_THROW(checker(options));
}

TEST(CheckerTest, TooManyPositionalOptions) {
    /// TODO replace this single test with tests for positional options compatibility
    auto options = std::make_shared<AbstractOption>();
    auto posopt = std::make_shared<PositionalOptionWithValue<int>>();
    posopt->setMaxOccurreneCount(2);
    options->addUnlock(posopt);
    options->addUnlock(std::make_shared<PositionalOptionWithValue<int>>());
    Checker checker;
    EXPECT_THROW(checker(options), MultipleOccurenceOnlyForLastPosopt);
}

TEST(CheckerTest, OneOf) {
    auto option = std::make_shared<OneOf>(
        std::make_shared<LiteralString>("run"),
        std::make_shared<LiteralString>("gather"),
        std::make_shared<LiteralString>("clean")
    );
    Checker checker;
    EXPECT_NO_THROW(checker(option));
}

TEST(CheckerTest, Alternatives2) {
    auto option = std::make_shared<OneOf>(
        std::make_shared<LiteralString>("run")->addUnlock(std::make_shared<NamedOption>("--opt1"))->addUnlock(std::make_shared<NamedOption>("--opt2")),
        std::make_shared<LiteralString>("gather")->addUnlock(std::make_shared<NamedOption>("--opt1")),
        std::make_shared<LiteralString>("clean")->addUnlock(std::make_shared<NamedOption>("--opt2"))
    );
    Checker checker;
    EXPECT_NO_THROW(checker(option));
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
    EXPECT_THROW(checker(option), DuplicateOptionName);
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
    EXPECT_NO_THROW(checker(hypercubeOptions));
}


TEST(CheckerTest, ComplexDuplicateOptionName) {
    auto helpOption1 = std::make_shared<NamedOption>("--help", "-h");
    auto helpOption2 = std::make_shared<NamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<OneOf>(
        std::make_shared<LiteralString>("run")->
            addUnlock(std::make_shared<NamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<NamedOption>("--mIntervalsPerDim", "-m"))->
            addUnlock(helpOption1)->
            addUnlock(helpOption2),
        std::make_shared<LiteralString>("gather")->addUnlock(helpOption1)
    );
    Checker checker;
    EXPECT_THROW(checker(hypercubeOptions), DuplicateOptionName);
}

TEST(CheckerTest, PositionalWithAllowedOptions) {
    auto options = std::make_shared<PositionalOptionWithValue<std::string>>();
    options->valueSemantics().unlocks("run");
    options->valueSemantics().unlocks("gather");
    options->valueSemantics().unlocks("clean");
    Checker checker;
    EXPECT_NO_THROW(checker(options));
}

TEST(CheckerTest, PositionalWithAllowedOptions2) {
    auto options = std::make_shared<PositionalOptionWithValue<std::string>>();
    options->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--opt1"));
    options->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--opt2"));
    options->valueSemantics().unlocks("gather").push_back(std::make_shared<NamedOption>("--opt1"));
    options->valueSemantics().unlocks("clean").push_back(std::make_shared<NamedOption>("--opt2"));
    Checker checker;
    EXPECT_NO_THROW(checker(options));
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
    EXPECT_THROW(checker(option), DuplicateOptionName);
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
    EXPECT_NO_THROW(checker(options));
}

TEST(CheckerTest, Cycle) {
    auto option1 = std::make_shared<NamedOption>("--opt1");
    auto option2 = std::make_shared<NamedOption>("--opt2");
    option1->addUnlock(option2);
    option2->addUnlock(option1);
    Checker checker;
    EXPECT_THROW(checker(option1), RecursionDetected);
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
    EXPECT_NO_THROW(checker(opts));
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
    EXPECT_THROW(checker(opts), DuplicateOptionName);
}

TEST(CheckerTest, CartesianProductConflict) {
    auto opts_nested_1 = std::make_shared<OneOf>();
    opts_nested_1->alternatives.push_back(
        std::make_shared<LiteralString>("alt1")->addUnlock(std::make_shared<NamedOption>("--dim"))
    );
    auto opts_nested_2 = std::make_shared<OneOf>();
    opts_nested_2->alternatives.push_back(
        std::make_shared<LiteralString>("alt1")->addUnlock(std::make_shared<NamedOption>("--dim"))
    );
    auto opts = std::make_shared<AbstractOption>();
    opts->addUnlock(opts_nested_1);
    opts->addUnlock(opts_nested_2);

    Checker checker;
    EXPECT_THROW(checker(opts), DuplicateOptionName);
}

TEST(CheckerCombinatorTest, Simple) {
    auto option = std::make_shared<NamedOption>("--opt", "-o");
    auto options = std::make_shared<AbstractOption>()->addUnlock(option);
    Combinator combinator(options);
    ASSERT_EQ(combinator.branches.size(), 1);
    ASSERT_EQ(combinator.branches[0].size(), 2);
    ASSERT_EQ(combinator.branches[0][0], options);
    ASSERT_EQ(combinator.branches[0][1], option);
}


TEST(CheckerCombinatorTest, Simple2) {
    auto option = std::make_shared<NamedOption>("--opt", "-o");
    auto options = std::make_shared<AbstractOption>()->addUnlock(option)->addUnlock(option);
    Combinator combinator(options);
    ASSERT_EQ(combinator.branches.size(), 1);
    ASSERT_EQ(combinator.branches[0].size(), 3);
    ASSERT_EQ(combinator.branches[0][0], options);
    ASSERT_EQ(combinator.branches[0][1], option);
    ASSERT_EQ(combinator.branches[0][2], option);
}


TEST(CheckerCombinatorTest, SingleOneOf) {
    auto option = std::make_shared<NamedOption>("--opt", "-o");
    auto one_of = std::make_shared<OneOf>()->addAlternative(option)->addAlternative(option);
    auto options = std::make_shared<AbstractOption>()->addUnlock(option)->addUnlock(one_of)->addUnlock(option);
    Combinator combinator(options);
    ASSERT_EQ(combinator.branches.size(), 2);
    ASSERT_EQ(combinator.branches[0].size(), 5);
    ASSERT_EQ(combinator.branches[0][0], options);
    ASSERT_EQ(combinator.branches[0][1], option);
    ASSERT_EQ(combinator.branches[0][2], one_of);
    ASSERT_EQ(combinator.branches[0][3], option);
    ASSERT_EQ(combinator.branches[0][4], option);
    ASSERT_EQ(combinator.branches[1].size(), 5);
    ASSERT_EQ(combinator.branches[1][0], options);
    ASSERT_EQ(combinator.branches[1][1], option);
    ASSERT_EQ(combinator.branches[1][2], one_of);
    ASSERT_EQ(combinator.branches[1][3], option);
    ASSERT_EQ(combinator.branches[1][4], option);
}

TEST(CheckerCombinatorTest, TwiceOneOf) {
    auto option = std::make_shared<NamedOption>("--opt", "-o");
    auto option2 = std::make_shared<NamedOption>("--opt2", "-p");
    auto one_of = std::make_shared<OneOf>()->addAlternative(option)->addAlternative(option2);
    auto options = std::make_shared<AbstractOption>()->addUnlock(option)->addUnlock(one_of)->addUnlock(option)->addUnlock(one_of);
    Combinator combinator(options);
    ASSERT_EQ(combinator.branches.size(), 4);
    ASSERT_EQ(combinator.branches[0].size(), 7);
    ASSERT_EQ(combinator.branches[0][0], options);
    ASSERT_EQ(combinator.branches[0][1], option);
    ASSERT_EQ(combinator.branches[0][2], one_of);
    ASSERT_EQ(combinator.branches[0][3], option);
    ASSERT_EQ(combinator.branches[0][4], option);
    ASSERT_EQ(combinator.branches[0][5], one_of);
    ASSERT_EQ(combinator.branches[0][6], option);
    ASSERT_EQ(combinator.branches[1].size(), 7);
    ASSERT_EQ(combinator.branches[1][0], options);
    ASSERT_EQ(combinator.branches[1][1], option);
    ASSERT_EQ(combinator.branches[1][2], one_of);
    ASSERT_EQ(combinator.branches[1][3], option);
    ASSERT_EQ(combinator.branches[1][4], option);
    ASSERT_EQ(combinator.branches[1][5], one_of);
    ASSERT_EQ(combinator.branches[1][6], option2);
    ASSERT_EQ(combinator.branches[2].size(), 7);
    ASSERT_EQ(combinator.branches[2][0], options);
    ASSERT_EQ(combinator.branches[2][1], option);
    ASSERT_EQ(combinator.branches[2][2], one_of);
    ASSERT_EQ(combinator.branches[2][3], option2);
    ASSERT_EQ(combinator.branches[2][4], option);
    ASSERT_EQ(combinator.branches[2][5], one_of);
    ASSERT_EQ(combinator.branches[2][6], option);
    ASSERT_EQ(combinator.branches[3].size(), 7);
    ASSERT_EQ(combinator.branches[3][0], options);
    ASSERT_EQ(combinator.branches[3][1], option);
    ASSERT_EQ(combinator.branches[3][2], one_of);
    ASSERT_EQ(combinator.branches[3][3], option2);
    ASSERT_EQ(combinator.branches[3][4], option);
    ASSERT_EQ(combinator.branches[3][5], one_of);
    ASSERT_EQ(combinator.branches[3][6], option2);
}

TEST(CheckerCombinatorTest, NestedOneOf) {
    auto option = std::make_shared<NamedOption>("--opt", "-o");
    auto option2 = std::make_shared<NamedOption>("--opt2", "-p");
    auto one_of_nested = std::make_shared<OneOf>()->addAlternative(option)->addAlternative(option2);
    auto one_of = std::make_shared<OneOf>()->addAlternative(one_of_nested)->addAlternative(option);
    auto options = std::make_shared<AbstractOption>()->addUnlock(option)->addUnlock(one_of)->addUnlock(option);
    Combinator combinator(options);
    ASSERT_EQ(combinator.branches.size(), 3);
    ASSERT_EQ(combinator.branches[0].size(), 6);
    ASSERT_EQ(combinator.branches[0][0], options);
    ASSERT_EQ(combinator.branches[0][1], option);
    ASSERT_EQ(combinator.branches[0][2], one_of);
    ASSERT_EQ(combinator.branches[0][3], one_of_nested);
    ASSERT_EQ(combinator.branches[0][4], option);
    ASSERT_EQ(combinator.branches[0][5], option);
    ASSERT_EQ(combinator.branches[1].size(), 6);
    ASSERT_EQ(combinator.branches[1][0], options);
    ASSERT_EQ(combinator.branches[1][1], option);
    ASSERT_EQ(combinator.branches[1][2], one_of);
    ASSERT_EQ(combinator.branches[1][3], one_of_nested);
    ASSERT_EQ(combinator.branches[1][4], option2);
    ASSERT_EQ(combinator.branches[1][5], option);
    ASSERT_EQ(combinator.branches[2].size(), 5);
    ASSERT_EQ(combinator.branches[2][0], options);
    ASSERT_EQ(combinator.branches[2][1], option);
    ASSERT_EQ(combinator.branches[2][2], one_of);
    ASSERT_EQ(combinator.branches[2][3], option);
    ASSERT_EQ(combinator.branches[2][4], option);

}