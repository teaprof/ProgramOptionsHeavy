#include <Backend/Checker.h>

#include <gtest/gtest.h>

TEST(CheckerTest, Simple1) {
    auto option = std::make_shared<AbstractNamedOption>("--opt", "-o");
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Simple2) {
    auto option = std::make_shared<AbstractNamedOption>("--opt1", "-o");
    option->addUnlock(std::make_shared<AbstractNamedOption>("--opt2"));
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));

    option->addUnlock(std::make_shared<AbstractNamedOption>("--opt2"));
    EXPECT_THROW(option->accept(checker), DuplicateOption);
}

TEST(CheckerTest, SimplePositionalOptions) {
    auto option = std::make_shared<AbstractNamedOption>("--opt1", "-o");
    option->addUnlock(std::make_shared<AbstractNamedOption>("--opt2"));
    option->addUnlock(std::make_shared<AbstractPositionalOption>());
    option->addUnlock(std::make_shared<AbstractPositionalOption>());
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Alternatives) {
    auto option = std::make_shared<Alternatives>(
        std::make_shared<AbstractNamedCommand>("run"),
        std::make_shared<AbstractNamedCommand>("gather"),
        std::make_shared<AbstractNamedCommand>("clean")
    );
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Alternatives2) {
    auto option = std::make_shared<Alternatives>(
        std::make_shared<AbstractNamedCommand>("run")->addUnlock(std::make_shared<AbstractNamedOption>("--opt1"))->addUnlock(std::make_shared<AbstractNamedOption>("--opt2")),
        std::make_shared<AbstractNamedCommand>("gather")->addUnlock(std::make_shared<AbstractNamedOption>("--opt1")),
        std::make_shared<AbstractNamedCommand>("clean")->addUnlock(std::make_shared<AbstractNamedOption>("--opt2"))
    );
    Checker checker;
    EXPECT_NO_THROW(option->accept(checker));
}

TEST(CheckerTest, Alternatives3) {    
    auto option = std::make_shared<AbstractOption>()->
        addUnlock(std::make_shared<AbstractNamedOption>("--opt1"))->
        addUnlock(    
            std::make_shared<Alternatives>(
                std::make_shared<AbstractNamedCommand>("run")->addUnlock(std::make_shared<AbstractNamedOption>("--opt1")),
                std::make_shared<AbstractNamedCommand>("gather")->addUnlock(std::make_shared<AbstractNamedOption>("--opt2")),
                std::make_shared<AbstractNamedCommand>("clean")->addUnlock(std::make_shared<AbstractNamedOption>("--opt1"))                
            )
        );
    Checker checker;
    EXPECT_THROW(option->accept(checker), DuplicateOption);
}


TEST(CheckerTest, Complex) {
    auto helpOption = std::make_shared<AbstractNamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<Alternatives>(
        helpOption,
        std::make_shared<AbstractNamedCommand>("run")->
            addUnlock(std::make_shared<AbstractNamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<AbstractNamedOption>("--mIntervalsPerDim", "-m"))->
            addUnlock(helpOption),
        std::make_shared<AbstractNamedCommand>("gather")->addUnlock(helpOption)
    );
    Checker checker;
    EXPECT_NO_THROW(hypercubeOptions->accept(checker));
}


TEST(CheckerTest, RepeatedOption) {
    auto helpOption = std::make_shared<AbstractNamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<Alternatives>(
        helpOption,
        std::make_shared<AbstractNamedCommand>("run")->
            addUnlock(std::make_shared<AbstractNamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<AbstractNamedOption>("--mIntervalsPerDim", "-m"))->
            addUnlock(helpOption)->
            addUnlock(helpOption),
        std::make_shared<AbstractNamedCommand>("gather")->addUnlock(helpOption)
    );
    Checker checker;
    EXPECT_THROW(hypercubeOptions->accept(checker), DuplicateOption);
}

TEST(CheckerTest, Cycle) {
    auto option1 = std::make_shared<AbstractNamedOption>("--opt1");
    auto option2 = std::make_shared<AbstractNamedOption>("--opt2");
    option1->addUnlock(option2);
    option2->addUnlock(option1);
    Checker checker;
    EXPECT_THROW(option1->accept(checker), DuplicateOption);
}