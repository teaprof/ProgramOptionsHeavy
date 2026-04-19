#include <Backend/Matcher.h>
#include <Backend/Printer.h>
#include <Backend/ValueSemantics.h>
#include <gtest/gtest.h>

class MatcherFixtureSimple : public ::testing::Test {
   protected:
    std::shared_ptr<AbstractOption> options_;
    std::shared_ptr<NamedOption> opt1_;
    std::shared_ptr<NamedOptionWithValue<int>> opt2_;
    std::shared_ptr<PositionalOptionWithValue<std::string>> opt3_;
    void SetUp() override {
        options_ = std::make_shared<OptionsGroup2>();
        opt1_ = std::make_shared<NamedOption>("--opt1", "-1");
        opt2_ = std::make_shared<NamedOptionWithValue<int>>("--opt2", "-2");
        opt3_ = std::make_shared<PositionalOptionWithValue<std::string>>();
        options_->addUnlock(opt1_);
        options_->addUnlock(opt2_);
        options_->addUnlock(opt3_);

        opt1_->setRequired(true);
        opt2_->valueSemantics().setMinMax(-10, 10);
    }

    void TearDown() override {}
};

class MatcherFixture : public ::testing::Test {
   protected:
    std::shared_ptr<AbstractOption> options_;
    std::shared_ptr<NamedOption> common_option_;
    std::shared_ptr<OneOf> command_;
    void SetUp() override {
        common_option_ = std::make_shared<NamedOption>("--common", "-c");
        options_ = std::make_shared<OptionsGroup2>();
        options_->addUnlock(common_option_);
        auto run_options = std::make_shared<LiteralString>("run");
        run_options->addUnlock(std::make_shared<NamedOption>("--dim", "-d"));
        auto gather_options = std::make_shared<LiteralString>("gather");
        gather_options->addUnlock(std::make_shared<NamedOption>("--gatheropt", "-g"));
        command_ = std::make_shared<OneOf>(run_options, gather_options);
        options_->addUnlock(command_);

        Printer prn;
        options_->accept(prn);
    }

    void TearDown() override {}
};

class MatcherFixtureWithUnlocksByValue : public ::testing::Test {
   protected:
    std::shared_ptr<AbstractOption> options_;
    std::shared_ptr<NamedOption> common_option_;
    std::shared_ptr<PositionalOptionWithValue<std::string>> command_;

    void SetUp() override {
        options_ = std::make_shared<OptionsGroup2>();

        common_option_ = std::make_shared<NamedOption>("--common", "-c");
        options_->addUnlock(common_option_);

        command_ = std::make_shared<PositionalOptionWithValue<std::string>>();
        command_->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--dim", "-d"));
        command_->valueSemantics().unlocks("gather").push_back(std::make_shared<NamedOption>("--gatheropt", "-g"));
        options_->addUnlock(command_);
    }

    void TearDown() override {}
};

TEST_F(MatcherFixtureSimple, Test1) {
    Matcher parser(options_);
    int v{0};
    opt2_->valueSemantics().setExternalStorage(v);
    parser.storage.setExternalStorage<int>(opt2_, &v);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt3_->setRequired(true);
    EXPECT_TRUE(parser.parse("-1 filename"));
    EXPECT_TRUE(parser.parse("filename -1"));
    opt3_->setRequired(false);
    EXPECT_EQ(v, 0);
    EXPECT_TRUE(parser.parse("--opt1"));
    EXPECT_EQ(v, 0);
    EXPECT_THROW(parser.parse("--opt1 --opt2"), ExpectedValue);
    EXPECT_EQ(v, 0);  /// TODO here should be default value
    EXPECT_TRUE(parser.parse("--opt1 --opt2=10"));
    EXPECT_EQ(v, 10);
    EXPECT_THROW(parser.parse("--opt1 --opt2 11"), ValueIsOutOfRange);
    EXPECT_EQ(v, 10);
    EXPECT_THROW(parser.parse("--opt1 --opt2 22"), ValueIsOutOfRange);
    EXPECT_EQ(v, 10);
    EXPECT_THROW(parser.parse("--opt1 --opt1"), MaxOptionOccurenceIsExceeded);
    EXPECT_EQ(v, 10);
}

TEST_F(MatcherFixture, Test1) {
    Matcher parser(options_);
    EXPECT_TRUE(parser.parse({}));
}

TEST_F(MatcherFixture, Test2) {
    Matcher parser(options_);
    command_->setRequired(true);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    EXPECT_TRUE(parser.parse({"run"}));
    EXPECT_TRUE(parser.parse({"gather", "-g"}));
    EXPECT_THROW(parser.parse({"run", "gather", "-g"}), OnlyOneChoiseIsAllowed);
    EXPECT_TRUE(parser.parse({"run", "--common"}));
    EXPECT_TRUE(parser.parse({"--common", "run", "-d"}));
}

TEST_F(MatcherFixtureWithUnlocksByValue, Test2) {
    Matcher parser(options_);
    command_->setRequired(true);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    EXPECT_TRUE(parser.parse({"run"}));
    EXPECT_TRUE(parser.parse({"gather", "-g"}));
    EXPECT_THROW(parser.parse({"run", "gather", "-g"}), TooManyPositionalOptions);
    //TODO: in the previous test gather could be treated as filename and it will be Ok
    EXPECT_TRUE(parser.parse({"run", "--common"}));
    EXPECT_TRUE(parser.parse({"--common", "run", "-d"}));
}

TEST(Matcher, OptionRequired) {
    auto opt = std::make_shared<NamedOption>("--opt1");
    opt->setRequired(true);

    Matcher parser(opt);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    EXPECT_TRUE(parser.parse({"--opt1"}));
}

TEST(Matcher, DefaultValue) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);

    Matcher parser(opt);
    int d{0};
    opt->valueSemantics().setExternalStorage(d);
    parser.storage.setExternalStorage(opt, &d);
    parser.parse({});
    EXPECT_EQ(d, 10);
    EXPECT_TRUE(parser.parse({"--opt1 20"}));
    EXPECT_EQ(d, 20);
}

TEST(Matcher, MultipleOccurrenceOfNamedOption) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);
    opt->setMaxOccurreneCount(2);

    Matcher parser(opt);
    int d{0};
    opt->valueSemantics().setExternalStorage(d);
    parser.storage.setExternalStorage(opt, &d);
    parser.parse({});
    EXPECT_EQ(d, 10);
    EXPECT_TRUE(parser.parse("--opt1 20"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    EXPECT_EQ(d, 20);
    EXPECT_TRUE(parser.parse("--opt1 20 --opt1 30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 2);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), "30");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "30");
    EXPECT_EQ(d, 30);
    ASSERT_THROW(parser.parse("--opt1 20 --opt1 30 --opt1 40"), MaxOptionOccurenceIsExceeded);
}

TEST(Matcher, MultipleOccurrenceOfPositionalOption) {
    auto opt = std::make_shared<PositionalOptionWithValue<int>>();
    opt->setMaxOccurreneCount(2);

    Matcher parser(opt);
    int d{0};
    opt->valueSemantics().setExternalStorage(d);
    parser.storage.setExternalStorage(opt, &d);
    opt->setRequired(true);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt->setRequired(false);
    EXPECT_NO_THROW(parser.parse({}));
    opt->valueSemantics().setDefaultValue(10);
    ASSERT_FALSE(parser.storage.contains(opt));
    EXPECT_NO_THROW(parser.parse({}));
    EXPECT_EQ(d, 10);
    EXPECT_TRUE(parser.parse("20"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    ASSERT_EQ(parser.storage[opt].valueAs<int>(0), 20);
    EXPECT_EQ(d, 20);
    EXPECT_TRUE(parser.parse("20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), "30");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "30");
    EXPECT_EQ(d, 30);
    ASSERT_THROW(parser.parse("20,30,40"), TooManyPositionalOptions);
}

TEST(Matcher, MultipleValuesOfNamedOptionExact) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);
    opt->setMaxOccurreneCount(1);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::EXACT, 2);

    Matcher parser(opt);
    int d{0};
    opt->valueSemantics().setExternalStorage(d);
    parser.storage.setExternalStorage(opt, &d);
    parser.parse({});
    EXPECT_EQ(d, 10);
    EXPECT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(d, 30);
    ASSERT_THROW(parser.parse("--opt1 20,30 --opt1 40,40 --opt1 20,20"), MaxOptionOccurenceIsExceeded);
    ASSERT_THROW(parser.parse("--opt1 20"), ExpectedExactNumberOfValues);
    ASSERT_THROW(parser.parse("--opt1"),
                 ExpectedValue);  /// todo: ExpectedValue vs TooFewValuesForOption
}

TEST(Matcher, MultipleValuesOfNamedOptionUpTo) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);
    opt->setMaxOccurreneCount(1);
    opt->setValueRequired(true);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 2);

    Matcher parser(opt);
    int d{0};
    opt->valueSemantics().setExternalStorage(d);
    parser.storage.setExternalStorage(opt, &d);
    parser.parse({});
    EXPECT_EQ(d, 10);
    ASSERT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(d, 30);
    ASSERT_TRUE(parser.parse("--opt1 31"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "31");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "31");
    EXPECT_EQ(d, 31);
    ASSERT_THROW(parser.parse("--opt1 20 --opt1 30"), MaxOptionOccurenceIsExceeded);
    // parse command finished with exception but parser storage should be
    // initialized with successfully parsed values
    /*ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");*/

    ASSERT_THROW(parser.parse("--opt1 20,30 filename"), TooManyPositionalOptions);

    ASSERT_THROW(parser.parse("--opt1"), ExpectedValue);
    opt->setValueRequired(false);
    ASSERT_TRUE(parser.parse("--opt1"));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 0);
    ASSERT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(d, 30);
}

TEST(Matcher, MultipleValuesOfNamedOptionInfinite) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);
    opt->setMaxOccurreneCount(1);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::INFINITE);

    Matcher parser(opt);
    int d{0};
    opt->valueSemantics().setExternalStorage(d);
    parser.storage.setExternalStorage(opt, &d);
    parser.parse({});
    EXPECT_EQ(d, 10);
    ASSERT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(d, 30);
}

TEST(Matcher, MultipleValuesOfPositionalOption) {
    auto opt = std::make_shared<PositionalOptionWithValue<int>>();
    opt->setMaxOccurreneCount(2);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 2);

    Matcher parser(opt);
    int d{0};
    opt->valueSemantics().setExternalStorage(d);
    parser.storage.setExternalStorage(opt, &d);
    opt->setRequired(true);
    EXPECT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt->setRequired(false);
    EXPECT_NO_THROW(parser.parse({}));
    opt->valueSemantics().setDefaultValue(10);
    ASSERT_FALSE(parser.storage.contains(opt));
    EXPECT_NO_THROW(parser.parse({}));
    EXPECT_EQ(d, 10);
    EXPECT_TRUE(parser.parse("20"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].occurrenceSize(0), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    EXPECT_EQ(parser.storage[opt].valueAs<int>(0), 20);
    EXPECT_EQ(d, 20);
    EXPECT_TRUE(parser.parse("20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(1), "30");
    EXPECT_EQ(d, 30);
    EXPECT_TRUE(parser.parse("20,30,40"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 2);
    ASSERT_EQ(parser.storage[opt].occurrenceSize(0), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    ASSERT_EQ(parser.storage[opt].occurrenceSize(1), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), "40");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "40");
    EXPECT_EQ(d, 40);
    ASSERT_THROW(parser.parse("20,30,40 50 60"), TooManyPositionalOptions);
}

TEST(Matcher, TwoPositionalOptions) {
    auto opt1 = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto opt2 = std::make_shared<PositionalOptionWithValue<int>>();
    opt2->setMaxOccurreneCount(2);
    auto opt = std::make_shared<OptionsGroup2>()->addUnlock(opt1)->addUnlock(opt2);

    Matcher parser(opt);
    EXPECT_NO_THROW(parser.parse({}));
    EXPECT_NO_THROW(parser.parse("file1"));
    EXPECT_NO_THROW(parser.parse("file1 10"));
    EXPECT_NO_THROW(parser.parse("file1 10 20"));
    EXPECT_THROW(parser.parse("file1 10 20,30"), TooManyPositionalOptions);
}

TEST(Matcher, TwoPositionalOptionsWithDoubleDash) {
    auto opt1 = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto opt2 = std::make_shared<PositionalOptionWithValue<int>>();
    opt1->setMaxOccurreneCount(10);
    opt1->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 10);
    opt2->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 2);
    auto opt = std::make_shared<OptionsGroup2>()->addUnlock(opt1)->addUnlock(opt2);

    Matcher parser(opt);  // todo: rename parser to matcher here and all other places
    EXPECT_NO_THROW(parser.parse("file1 file2 file3 -- 10"));
    ASSERT_EQ(parser.storage[opt1].occurrenceCount(), 3);
    ASSERT_EQ(parser.storage[opt1].occurrenceSize(0), 1);
    ASSERT_EQ(parser.storage[opt1].rawValues(0, 0), "file1");
    ASSERT_EQ(parser.storage[opt1].occurrenceSize(1), 1);
    ASSERT_EQ(parser.storage[opt1].rawValues(1, 0), "file2");
    ASSERT_EQ(parser.storage[opt1].occurrenceSize(1), 1);
    ASSERT_EQ(parser.storage[opt1].rawValues(2, 0), "file3");

    ASSERT_EQ(parser.storage[opt2].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt2].occurrenceSize(0), 1);
    ASSERT_EQ(parser.storage[opt2].rawValues(0, 0), "10");
}

/*TEST(Matcher, DoubleDashTerminatesNamedOptions) {
    auto opt1 = std::make_shared<NamedOptionWithValue<int>>("--arg1");
    auto opt2 = std::make_shared<NamedOptionWithValue<int>>("--arg2");
    auto opt3 = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto opt = std::make_shared<OptionsGroup2>()->addUnlock(opt1)->addUnlock(opt2)->addUnlock(opt3);

    Matcher parser(opt);  // todo: rename parser to matcher here and all other places
    EXPECT_NO_THROW(parser.parse("--arg1 10 -- --arg2"));
    ASSERT_EQ(parser.storage[opt1].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt1].occurrenceSize(0), 1);
    ASSERT_EQ(parser.storage[opt1].rawValues(0, 0), "10");
    ASSERT_FALSE(parser.storage.contains(opt2));
    ASSERT_EQ(parser.storage[opt3].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt3].occurrenceSize(0), 1);
    ASSERT_EQ(parser.storage[opt3].rawValues(0, 0), "--arg2");
}*/

TEST(Matcher, PositionalAndNamed) {
    auto posopt = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto namedopt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    posopt->setMaxOccurreneCount(2);
    namedopt->setMaxOccurreneCount(2);

    auto opts = std::make_shared<OptionsGroup2>()->addUnlock(posopt)->addUnlock(namedopt);

    Matcher parser(opts);
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
    auto alt_nested_1 =
        std::make_shared<OneOf>()
            ->addAlternative(std::make_shared<LiteralString>("alt11")->addUnlock(std::make_shared<NamedOption>("--opt11")))
            ->addAlternative(std::make_shared<LiteralString>("alt12")->addUnlock(std::make_shared<NamedOption>("--opt12")));
    auto alt_nested_2 =
        std::make_shared<OneOf>()
            ->addAlternative(std::make_shared<LiteralString>("alt21")->addUnlock(std::make_shared<NamedOption>("--opt21")))
            ->addAlternative(std::make_shared<LiteralString>("alt22")->addUnlock(std::make_shared<NamedOption>("--opt22")));
    auto opts = std::make_shared<OneOf>()
                    ->addAlternative(std::make_shared<LiteralString>("alt1")->addUnlock(alt_nested_1))
                    ->addAlternative(std::make_shared<LiteralString>("alt2")->addUnlock(alt_nested_2));

    Matcher parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt11 --opt11"));
    ASSERT_NO_THROW(parser.parse("alt1 alt12 --opt12"));
    ASSERT_NO_THROW(parser.parse("alt2 alt21 --opt21"));
    ASSERT_NO_THROW(parser.parse("alt2 alt22 --opt22"));

    ASSERT_THROW(parser.parse("--opt11"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt12 --opt12 --unknown"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt2 --opt21"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt2"), OnlyOneChoiseIsAllowed);
    ASSERT_THROW(parser.parse("alt1 alt11 alt12"), OnlyOneChoiseIsAllowed);
}

TEST(Matcher, NestedAlternativesWithEqualNames) {
    auto alt_nested_1 =
        std::make_shared<OneOf>()
            ->addAlternative(std::make_shared<LiteralString>("alt1")->addUnlock(std::make_shared<NamedOption>("--opt1")))
            ->addAlternative(std::make_shared<LiteralString>("alt2")->addUnlock(std::make_shared<NamedOption>("--opt2")));
    auto alt_nested_2 =
        std::make_shared<OneOf>()
            ->addAlternative(std::make_shared<LiteralString>("alt1")->addUnlock(std::make_shared<NamedOption>("--opt1")))
            ->addAlternative(std::make_shared<LiteralString>("alt2")->addUnlock(std::make_shared<NamedOption>("--opt2")));
    auto opts = std::make_shared<OneOf>()
                    ->addAlternative(std::make_shared<LiteralString>("alt1")->addUnlock(alt_nested_1))
                    ->addAlternative(std::make_shared<LiteralString>("alt2")->addUnlock(alt_nested_2));

    Matcher parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt1 alt2 --opt2"));
    ASSERT_NO_THROW(parser.parse("alt2 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt2 alt2 --opt2"));
}

TEST(MatcherWithUnlocksByValue, NestedAlternatives) {
    auto alt_nested_1 = std::make_shared<PositionalOptionWithValue<std::string>>();
    alt_nested_1->valueSemantics().unlocks("alt11").push_back(std::make_shared<NamedOption>("--opt11"));
    alt_nested_1->valueSemantics().unlocks("alt12").push_back(std::make_shared<NamedOption>("--opt12"));
    alt_nested_1->valueSemantics().setOnlyAllowedValues(true);
    auto alt_nested_2 = std::make_shared<PositionalOptionWithValue<std::string>>();
    alt_nested_2->valueSemantics().unlocks("alt21").push_back(std::make_shared<NamedOption>("--opt21"));
    alt_nested_2->valueSemantics().unlocks("alt22").push_back(std::make_shared<NamedOption>("--opt22"));
    auto opts = std::make_shared<PositionalOptionWithValue<std::string>>();
    opts->valueSemantics().unlocks("alt1").push_back(alt_nested_1);
    opts->valueSemantics().unlocks("alt2").push_back(alt_nested_2);

    Matcher parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt11 --opt11"));
    ASSERT_NO_THROW(parser.parse("alt1 alt12 --opt12"));
    ASSERT_NO_THROW(parser.parse("alt2 alt21 --opt21"));
    ASSERT_NO_THROW(parser.parse("alt2 alt22 --opt22"));

    ASSERT_THROW(parser.parse("--opt11"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt12 --opt12 --unknown"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt2 --opt21"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt2"), InvalidOptionValue);
    ASSERT_THROW(parser.parse("alt1 alt11 alt12"), TooManyPositionalOptions);
}

TEST(MatcherWithUnlocksByValue, NestedAlternativesWithEqualNames) {
    auto alt_nested_1 = std::make_shared<PositionalOptionWithValue<std::string>>();
    alt_nested_1->valueSemantics().unlocks("alt1").push_back(std::make_shared<NamedOption>("--opt1"));
    alt_nested_1->valueSemantics().unlocks("alt2").push_back(std::make_shared<NamedOption>("--opt2"));
    auto alt_nested_2 = std::make_shared<PositionalOptionWithValue<std::string>>();
    alt_nested_2->valueSemantics().unlocks("alt1").push_back(std::make_shared<NamedOption>("--opt1"));
    alt_nested_2->valueSemantics().unlocks("alt2").push_back(std::make_shared<NamedOption>("--opt2"));
    auto opts = std::make_shared<PositionalOptionWithValue<std::string>>();
    opts->valueSemantics().unlocks("alt1").push_back(alt_nested_1);
    opts->valueSemantics().unlocks("alt2").push_back(alt_nested_2);

    Matcher parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt1 alt2 --opt2"));
    ASSERT_NO_THROW(parser.parse("alt2 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt2 alt2 --opt2"));
}