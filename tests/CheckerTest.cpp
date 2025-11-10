#include <Backend/ValueSemantics.h>
#include <Backend/Checker.h>

#include <gtest/gtest.h>

TEST(CheckerTest, Simple1) {
    auto option = std::make_shared<NamedOption>("--opt", "-o");
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Simple2) {
    auto option = std::make_shared<NamedOption>("--opt1", "-o");
    option->addUnlock(std::make_shared<NamedOption>("--opt2"));
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));

    option->addUnlock(std::make_shared<NamedOption>("--opt2"));
    EXPECT_THROW(option->accept(checker), DuplicateOption);
}

TEST(CheckerTest, SimplePositionalOptions) {
    auto option = std::make_shared<NamedOption>("--opt1", "-o");
    option->addUnlock(std::make_shared<NamedOption>("--opt2"));
    option->addUnlock(std::make_shared<PositionalOption<int>>());
    option->addUnlock(std::make_shared<PositionalOption<int>>());
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, OneOf) {
    auto option = std::make_shared<OneOf>(
        std::make_shared<NamedCommand>("run"),
        std::make_shared<NamedCommand>("gather"),
        std::make_shared<NamedCommand>("clean")
    );
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Alternatives2) {
    auto option = std::make_shared<OneOf>(
        std::make_shared<NamedCommand>("run")->addUnlock(std::make_shared<NamedOption>("--opt1"))->addUnlock(std::make_shared<NamedOption>("--opt2")),
        std::make_shared<NamedCommand>("gather")->addUnlock(std::make_shared<NamedOption>("--opt1")),
        std::make_shared<NamedCommand>("clean")->addUnlock(std::make_shared<NamedOption>("--opt2"))
    );
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Alternatives3) {    
    auto option = std::make_shared<AbstractOption>()->
        addUnlock(std::make_shared<NamedOption>("--opt1"))->
        addUnlock(    
            std::make_shared<OneOf>(
                std::make_shared<NamedCommand>("run")->addUnlock(std::make_shared<NamedOption>("--opt1")),
                std::make_shared<NamedCommand>("gather")->addUnlock(std::make_shared<NamedOption>("--opt2")),
                std::make_shared<NamedCommand>("clean")->addUnlock(std::make_shared<NamedOption>("--opt1"))                
            )
        );
    Checker checker;
    EXPECT_THROW(option->accept(checker), DuplicateOption);
}


TEST(CheckerTest, Complex) {
    auto helpOption = std::make_shared<NamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<OneOf>(
        helpOption,
        std::make_shared<NamedCommand>("run")->
            addUnlock(std::make_shared<NamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<NamedOption>("--mIntervalsPerDim", "-m"))->
            addUnlock(helpOption),
        std::make_shared<NamedCommand>("gather")->addUnlock(helpOption)
    );
    Checker checker;
    EXPECT_NO_THROW(hypercubeOptions->accept(checker));
}


TEST(CheckerTest, RepeatedOption) {
    auto helpOption = std::make_shared<NamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<OneOf>(
        helpOption,
        std::make_shared<NamedCommand>("run")->
            addUnlock(std::make_shared<NamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<NamedOption>("--mIntervalsPerDim", "-m"))->
            addUnlock(helpOption)->
            addUnlock(helpOption),
        std::make_shared<NamedCommand>("gather")->addUnlock(helpOption)
    );
    Checker checker;
    EXPECT_THROW(hypercubeOptions->accept(checker), DuplicateOption);
}

TEST(CheckerTest, Cycle) {
    auto option1 = std::make_shared<NamedOption>("--opt1");
    auto option2 = std::make_shared<NamedOption>("--opt2");
    option1->addUnlock(option2);
    option2->addUnlock(option1);
    Checker checker;
    EXPECT_THROW(option1->accept(checker), DuplicateOption);
}