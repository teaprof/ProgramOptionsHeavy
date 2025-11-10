#include <Backend/ValueSemantics.h>
#include <Backend/Matcher.h>
#include <gtest/gtest.h>

class MatcherFixtureSimple : public ::testing::Test {
    protected:
        std::shared_ptr<AbstractOption> options;
        std::shared_ptr<NamedOption> opt1;
        std::shared_ptr<NamedOptionWithValue<int>> opt2;
        std::shared_ptr<PositionalOption<std::string>> opt3;
        void SetUp() override {
            options = std::make_shared<OptionsGroup>();
            opt1 = std::make_shared<NamedOption>("--opt1", "-1");
            opt2 = std::make_shared<NamedOptionWithValue<int>>("--opt2", "-2");
            opt3 = std::make_shared<PositionalOption<std::string>>();
            options->addUnlock(opt1);
            options->addUnlock(opt2);
            options->addUnlock(opt3);

            opt1->setRequired(true);
            opt2->valueSemantics().setMinMax(-10, 10);
        }

        void TearDown() override {
        }        
};

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

TEST_F(MatcherFixtureSimple, Test1) {
    Parser parser(options);
    int v{0};
    parser.storage.setExternalStorage<int>(opt2, &v);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt3->setRequired(true);
    EXPECT_TRUE(parser.parse("-1 filename"));
    EXPECT_TRUE(parser.parse("filename -1"));
    opt3->setRequired(false);
    ASSERT_EQ(v, 0);
    EXPECT_TRUE(parser.parse("--opt1"));
    ASSERT_EQ(v, 0);
    EXPECT_THROW(parser.parse("--opt1 --opt2"), ExpectedValue);
    ASSERT_EQ(v, 0); /// TODO here should be default value
    EXPECT_TRUE(parser.parse("--opt1 --opt2=10"));
    ASSERT_EQ(v, 10);
    EXPECT_THROW(parser.parse("--opt1 --opt2 11"), ValueIsOutOfRange);
    ASSERT_EQ(v, 10);
    EXPECT_THROW(parser.parse("--opt1 --opt2 22"), ValueIsOutOfRange);
    ASSERT_EQ(v, 10);
    EXPECT_THROW(parser.parse("--opt1 --opt1"), MaxOptionOccurenceIsExceeded);
    ASSERT_EQ(v, 10);
}

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
    EXPECT_THROW(parser.parse({"run", "gather", "-g"}), OnlyOneChoiseIsAllowed);
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

TEST(Matcher, DefaultValue) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);

    Parser parser(opt);
    int d{0};
    parser.storage.setExternalStorage(opt, &d);
    parser.parse({});
    ASSERT_EQ(d, 10);
    EXPECT_TRUE(parser.parse({"--opt1 20"}));
    ASSERT_EQ(d, 20);
}

TEST(Matcher, MultipleOccurrenceOfNamedOption) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);
    opt->setMaxOccurreneCount(2);

    Parser parser(opt);
    int d{0};
    parser.storage.setExternalStorage(opt, &d);
    parser.parse({});
    ASSERT_EQ(d, 10);
    EXPECT_TRUE(parser.parse("--opt1 20"));
    EXPECT_EQ(parser.storage[opt]->size(), 1);
    EXPECT_EQ(parser.storage[opt]->rawValues(0), "20");
    ASSERT_EQ(d, 20);
    EXPECT_TRUE(parser.parse("--opt1 20 --opt1 30"));
    EXPECT_TRUE(parser.storage[opt]->size() == 2);
    EXPECT_EQ(parser.storage[opt]->rawValues(0), "20");
    EXPECT_EQ(parser.storage[opt]->rawValues(1), "30");
    ASSERT_EQ(d, 30);
    ASSERT_THROW(parser.parse("--opt1 20 --opt1 30 --opt1 40"), MaxOptionOccurenceIsExceeded);
}

TEST(Matcher, MultipleOccurrenceOfPositionalOption) {
    auto opt = std::make_shared<PositionalOption<int>>();
    opt->setMaxOccurreneCount(2);

    Parser parser(opt);
    int d{0};
    parser.storage.setExternalStorage(opt, &d);
    opt->setRequired(true);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt->setRequired(false);
    EXPECT_NO_THROW(parser.parse({}));
    opt->valueSemantics().setDefaultValue(10);
    ASSERT_FALSE(parser.storage.contains(opt));
    EXPECT_NO_THROW(parser.parse({}));
    opt->valueSemantics().setDefaultValue(10);
    ASSERT_EQ(d, 10);
    EXPECT_TRUE(parser.parse("20"));
    EXPECT_EQ(parser.storage[opt]->size(), 1);
    EXPECT_EQ(parser.storage[opt]->rawValues(0), "20");
    auto int_storage = std::dynamic_pointer_cast<TypedValueStorage<int>>(parser.storage[opt]);
    ASSERT_EQ(int_storage->values(0), 20);
    ASSERT_EQ(d, 20);
    EXPECT_TRUE(parser.parse("20 30"));
    EXPECT_TRUE(parser.storage[opt]->size() == 2);
    EXPECT_EQ(parser.storage[opt]->rawValues(0), "20");
    EXPECT_EQ(parser.storage[opt]->rawValues(1), "30");
    int_storage = std::dynamic_pointer_cast<TypedValueStorage<int>>(parser.storage[opt]);
    ASSERT_EQ(int_storage->values(0), 20);
    ASSERT_EQ(int_storage->values(1), 30);
    ASSERT_EQ(d, 30);
    ASSERT_THROW(parser.parse("20 30 40"), TooManyPositionalOptions);
}

TEST(Matcher, TwoPositionalOptions) {
    auto opt1 = std::make_shared<PositionalOption<std::string>>();
    auto opt2 = std::make_shared<PositionalOption<int>>();
    opt2->setMaxOccurreneCount(2);
    auto opt = std::make_shared<OptionsGroup>()->addUnlock(opt1)->addUnlock(opt2);

    Parser parser(opt);
    EXPECT_NO_THROW(parser.parse({}));
    EXPECT_NO_THROW(parser.parse("file1"));
    EXPECT_NO_THROW(parser.parse("file1 10"));
    EXPECT_NO_THROW(parser.parse("file1 10 20"));
    EXPECT_THROW(parser.parse("file1 10 20 30"), TooManyPositionalOptions);
}

TEST(Matcher, PositionalAndNamed) {
    auto posopt = std::make_shared<PositionalOption<std::string>>();
    auto namedopt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    posopt->setMaxOccurreneCount(2);
    namedopt->setMaxOccurreneCount(2);

    auto opts = std::make_shared<OptionsGroup>()->addUnlock(posopt)->addUnlock(namedopt);

    Parser parser(opts);
    EXPECT_NO_THROW(parser.parse(""));
    EXPECT_NO_THROW(parser.parse("--opt1 10"));
    EXPECT_NO_THROW(parser.parse("--opt1 10 filename"));
    EXPECT_NO_THROW(parser.parse("--opt1 10 --opt1 20 filename"));
    EXPECT_NO_THROW(parser.parse("--opt1 10 filename1 filename2"));
    EXPECT_NO_THROW(parser.parse("--opt1 10 --opt1 20 filename1 filename2"));
    EXPECT_THROW(parser.parse("--opt1 10 --opt1 20 filename1 filename2 filename3"), TooManyPositionalOptions);
    EXPECT_NO_THROW(parser.parse("filename --opt1 10 filename --opt1 20"));
}

TEST(Matcher, NestedAlternatives) {
    auto alt_nested_1 = std::make_shared<OneOf>()->addAlternative(
        std::make_shared<NamedCommand>("alt11")->addUnlock(std::make_shared<NamedOption>("--opt11"))
    )->addAlternative(
        std::make_shared<NamedCommand>("alt12")->addUnlock(std::make_shared<NamedOption>("--opt12"))
    );
    auto alt_nested_2 = std::make_shared<OneOf>()->addAlternative(
        std::make_shared<NamedCommand>("alt21")->addUnlock(std::make_shared<NamedOption>("--opt21"))
    )->addAlternative(
        std::make_shared<NamedCommand>("alt22")->addUnlock(std::make_shared<NamedOption>("--opt22"))
    );
    auto opts = std::make_shared<OneOf>()->addAlternative(
        std::make_shared<NamedCommand>("alt1")->addUnlock(alt_nested_1)
    )->addAlternative(
        std::make_shared<NamedCommand>("alt2")->addUnlock(alt_nested_2)
    );

    Parser parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt11 --opt11"));
    ASSERT_NO_THROW(parser.parse("alt1 alt12 --opt12"));
    ASSERT_NO_THROW(parser.parse("alt2 alt21 --opt21"));
    ASSERT_NO_THROW(parser.parse("alt2 alt22 --opt22"));

    ASSERT_THROW(parser.parse("--opt11"), UnknownOption);
    ASSERT_THROW(parser.parse("alt1 alt12 --opt12 --unknown"), UnknownOption);
    ASSERT_THROW(parser.parse("alt2 --opt21"), UnknownOption);
    ASSERT_THROW(parser.parse("alt1 alt2"), OnlyOneChoiseIsAllowed); 
    ASSERT_THROW(parser.parse("alt1 alt11 alt12"), OnlyOneChoiseIsAllowed); 
}

TEST(Matcher, NestedAlternativesWithEqualNames) {
    auto alt_nested_1 = std::make_shared<OneOf>()->addAlternative(
        std::make_shared<NamedCommand>("alt1")->addUnlock(std::make_shared<NamedOption>("--opt1"))
    )->addAlternative(
        std::make_shared<NamedCommand>("alt2")->addUnlock(std::make_shared<NamedOption>("--opt2"))
    );
    auto alt_nested_2 = std::make_shared<OneOf>()->addAlternative(
        std::make_shared<NamedCommand>("alt1")->addUnlock(std::make_shared<NamedOption>("--opt1"))
    )->addAlternative(
        std::make_shared<NamedCommand>("alt2")->addUnlock(std::make_shared<NamedOption>("--opt2"))
    );
    auto opts = std::make_shared<OneOf>()->addAlternative(
        std::make_shared<NamedCommand>("alt1")->addUnlock(alt_nested_1)
    )->addAlternative(
        std::make_shared<NamedCommand>("alt2")->addUnlock(alt_nested_2)
    );

    Parser parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt1 alt2 --opt2"));
    ASSERT_NO_THROW(parser.parse("alt2 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt2 alt2 --opt2"));
}