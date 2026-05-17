#include <Backend/Parser.h>
#include <Backend/Printer.h>
#include <Backend/ValueSemantics.h>
#include <gtest/gtest.h>

/*
TEST_F(MatcherFixtureSimple, Test1)
    RequiredOptionIsNotSet
    ExpectedValue
    ValueIsOutOfRange
    MaxOptionOccurrenceIsExceeded
TEST_F(MatcherFixture, Test1)
    RequiredOptionIsNotSet
    TooManyPositionalOptions
TEST_F(MatcherFixture, Test2)
    RequiredOptionIsNotSet
    TooManyPositionalOptions
TEST_F(MatcherFixtureWithUnlocksByValue, Test2)
    RequiredOptionIsNotSet
    TooManyPositionalOptions
TEST(Parser2, OptionRequired)
    RequiredOptionIsNotSet
TEST(Parser2, DefaultValue)
    none
TEST(Parser2, MultipleOccurrenceOfNamedOption)
    MaxOptionOccurrenceIsExceeded
TEST(Parser2, MultipleOccurrenceOfPositionalOption)
    RequiredOptionIsNotSet
    TooManyPositionalOptions
TEST(Parser2, MultipleValuesOfNamedOptionExact)
    MaxOptionOccurrenceIsExceeded
    ExpectedExactNumberOfValues
TEST(Parser2, MultipleValuesOfNamedOptionUpTo)
    MaxOptionOccurrenceIsExceeded
    TooManyPositionalOptions
TEST(Parser2, MultipleValuesOfNamedOptionInfinite)
    none
TEST(Parser2, MultipleValuesOfPositionalOption)
    RequiredOptionIsNotSet
    TooManyValuesForOption
    TooManyPositionalOptions
TEST(Parser2, TwoPositionalOptions)
    TooManyPositionalOptions
TEST(Parser2, TwoPositionalOptionsWithDoubleDash)
    none
//TEST(Parser2, DoubleDashTerminatesNamedOptions)
    none
TEST(Parser2, PositionalAndNamed)
    TooManyPositionalOptions
TEST(Parser2, NestedAlternatives)
    UnknownNamedOption
    UnexpectedValueForPositionalOption
    TooManyPositionalOptions
TEST(Parser2, NestedAlternativesWithEqualNames)
    none
TEST(MatcherWithUnlocksByValue, NestedAlternatives)
    UnknownNamedOption
    ValueIsNotAllowed
    TooManyPositionalOptions
TEST(MatcherWithUnlocksByValue, NestedAlternativesWithEqualNames)
    none
*/

class MatcherFixtureSimple : public ::testing::Test {
    // creates the parser with the following acceptable options:
    // --opt1   [--opt2=val]   [positional]
    // --opt1 is required, short name is -1
    // --opt2 is optional, val should be in range [-10, 10]
    // positional argument is optional
   protected:
    std::shared_ptr<AbstractOption> root_opt_;
    std::shared_ptr<NamedOption> opt1_;
    std::shared_ptr<NamedOptionWithValue<int>> opt2_;
    std::shared_ptr<PositionalOptionWithValue<std::string>> opt3_;
    void SetUp() override {
        root_opt_ = std::make_shared<OptionsGroup>();
        opt1_ = std::make_shared<NamedOption>("--opt1", "-1");
        opt2_ = std::make_shared<NamedOptionWithValue<int>>("--opt2", "-2");
        opt3_ = std::make_shared<PositionalOptionWithValue<std::string>>();
        root_opt_->addUnlock(opt1_);
        root_opt_->addUnlock(opt2_);
        root_opt_->addUnlock(opt3_);

        opt1_->setRequired(true);
        opt2_->valueSemantics().setMinMax(-10, 10);
    }

    void TearDown() override {}
};

class MatcherFixture : public ::testing::Test {
    // creates the parser with the following acceptable options:
    // run [--dim] [--common]
    // gather [--gatheropt] [--common]
   protected:
    std::shared_ptr<AbstractOption> root_opt_;
    std::shared_ptr<NamedOption> common_option_;
    std::shared_ptr<OneOfPositional> command_;
    void SetUp() override {
        common_option_ = std::make_shared<NamedOption>("--common", "-c");
        root_opt_ = std::make_shared<OptionsGroup>();
        root_opt_->addUnlock(common_option_);
        auto run_options = std::make_shared<LiteralString>("run");
        run_options->addUnlock(std::make_shared<NamedOption>("--dim", "-d"));
        auto gather_options = std::make_shared<LiteralString>("gather");
        gather_options->addUnlock(std::make_shared<NamedOption>("--gatheropt", "-g"));
        command_ = std::make_shared<OneOfPositional>(run_options, gather_options);
        root_opt_->addUnlock(command_);

        //Printer prn;
        //options_->accept(prn);
    }

    void TearDown() override {}
};

class MatcherFixtureWithUnlocksByValue : public ::testing::Test {
    // creates the parser with the following acceptable options:
    // --common run --dim
    // --common gather --gatheropt
   protected:
    std::shared_ptr<AbstractOption> root_opt_;
    std::shared_ptr<NamedOption> common_option_;
    std::shared_ptr<PositionalOptionWithValue<std::string>> command_;

    void SetUp() override {
        root_opt_ = std::make_shared<OptionsGroup>();

        common_option_ = std::make_shared<NamedOption>("--common", "-c");
        root_opt_->addUnlock(common_option_);

        command_ = std::make_shared<PositionalOptionWithValue<std::string>>();
        command_->valueSemantics().unlocks("run").push_back(std::make_shared<NamedOption>("--dim", "-d"));
        command_->valueSemantics().unlocks("gather").push_back(std::make_shared<NamedOption>("--gatheropt", "-g"));
        root_opt_->addUnlock(command_);
    }

    void TearDown() override {}
};

TEST_F(MatcherFixtureSimple, Test1) {
    Parser2 parser(root_opt_);
    int int_value{0};
    opt2_->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage<int>(opt2_, &int_value);
    ASSERT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt3_->setRequired(true);
    ASSERT_TRUE(parser.parse("-1 filename"));
    ASSERT_TRUE(parser.parse("filename -1"));
    opt3_->setRequired(false);
    EXPECT_EQ(int_value, 0);
    ASSERT_TRUE(parser.parse("--opt1"));
    EXPECT_EQ(int_value, 0);
    ASSERT_THROW(parser.parse("--opt1 --opt2"), ExpectedValue);
    EXPECT_EQ(int_value, 0);  /// unchanged
    ASSERT_TRUE(parser.parse("--opt1 --opt2=10"));
    EXPECT_EQ(int_value, 10);
    ASSERT_THROW(parser.parse("--opt1 --opt2 11"), ValueIsOutOfRange);
    EXPECT_EQ(int_value, 10);
    ASSERT_THROW(parser.parse("--opt1 --opt2 22"), ValueIsOutOfRange);
    EXPECT_EQ(int_value, 10);
    ASSERT_THROW(parser.parse("--opt1 --opt1"), MaxOptionOccurrenceIsExceeded);
    EXPECT_EQ(int_value, 10);
}

TEST_F(MatcherFixture, Test1) {
    Parser2 parser(root_opt_);
    ASSERT_TRUE(parser.parse({}));
}

TEST_F(MatcherFixture, Test2) {
    Parser2 parser(root_opt_);
    command_->setRequired(true);
    ASSERT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    ASSERT_TRUE(parser.parse({"run"}));
    ASSERT_TRUE(parser.parse({"gather", "-g"}));
    ASSERT_THROW(parser.parse({"run", "gather", "-g"}), TooManyPositionalOptions);
    ASSERT_TRUE(parser.parse({"run", "--common"}));
    ASSERT_TRUE(parser.parse({"--common", "run", "-d"}));
}

TEST_F(MatcherFixtureWithUnlocksByValue, Test2) {
    Parser2 parser(root_opt_);
    command_->setRequired(true);
    ASSERT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    ASSERT_TRUE(parser.parse({"run"}));
    ASSERT_TRUE(parser.parse({"gather", "-g"}));
    ASSERT_THROW(parser.parse({"run", "gather", "-g"}), TooManyPositionalOptions);
    // TODO: in the previous test gather could be treated as filename and it will be Ok
    ASSERT_TRUE(parser.parse({"run", "--common"}));
    ASSERT_TRUE(parser.parse({"--common", "run", "-d"}));
}

TEST(Parser2, OptionRequired) {
    auto opt = std::make_shared<NamedOption>("--opt1");
    opt->setRequired(true);

    Parser2 parser(opt);
    ASSERT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    ASSERT_TRUE(parser.parse({"--opt1"}));
}

TEST(Parser2, DefaultValue) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);

    Parser2 parser(opt);
    int int_value{0};
    opt->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage(opt, &int_value);

    ASSERT_TRUE(parser.parse({}));
    EXPECT_EQ(int_value, 0); // unchanged

    ASSERT_TRUE(parser.parse({"--opt1"}));
    EXPECT_EQ(int_value, 10); // default value

    ASSERT_TRUE(parser.parse({"--opt1 20"}));
    EXPECT_EQ(int_value, 20); // value from command line

    opt->setRequired(true);
    ASSERT_THROW(parser.parse({}), RequiredOptionIsNotSet);
}

TEST(Parser2, Implicit) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setImplicitValue(10);

    Parser2 parser(opt);
    int int_value{0};
    opt->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage(opt, &int_value);

    ASSERT_TRUE(parser.parse({}));
    EXPECT_EQ(int_value, 10); // implicit value

    ASSERT_TRUE(parser.parse({"--opt1 20"}));
    EXPECT_EQ(int_value, 20); // value from command line

    opt->valueSemantics().setDefaultValue(11);
    ASSERT_TRUE(parser.parse({"--opt1"}));
    EXPECT_EQ(int_value, 11); // default value

    opt->setRequired(true);
    ASSERT_TRUE(parser.parse({})); 
    EXPECT_EQ(int_value, 10); // implicit value
}

TEST(Parser2, MultipleOccurrenceOfNamedOption) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);
    opt->valueSemantics().setImplicitValue(11);
    opt->setMaxOccurreneCount(2);

    Parser2 parser(opt);
    int int_value{0};
    opt->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage(opt, &int_value);
    parser.parse({});
    EXPECT_EQ(int_value, 11);  // implicit value

    parser.parse({"--opt1"});
    EXPECT_EQ(int_value, 10);  // default value

    ASSERT_TRUE(parser.parse("--opt1 20"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    EXPECT_EQ(int_value, 20);
    ASSERT_TRUE(parser.parse("--opt1 20 --opt1 30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 2);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), "30");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "30");
    EXPECT_EQ(int_value, 30);

    ASSERT_TRUE(parser.parse("--opt1 20 --opt1"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 2);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].valueAs<int>(1, 0), 10); // default value
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), ""); // default value
    EXPECT_EQ(parser.storage[opt].rawValues(0), ""); 
    EXPECT_EQ(parser.storage[opt].valueAs<int>(0), 10); // default value
    EXPECT_EQ(int_value, 10);

    ASSERT_THROW(parser.parse("--opt1 20 --opt1 30 --opt1"), MaxOptionOccurrenceIsExceeded);
    ASSERT_THROW(parser.parse("--opt1 20 --opt1 30 --opt1 40"), MaxOptionOccurrenceIsExceeded);
}

TEST(Parser2, MultipleOccurrenceOfPositionalOption) {
    auto opt = std::make_shared<PositionalOptionWithValue<int>>();
    opt->setMaxOccurreneCount(2);

    Parser2 parser(opt);
    int int_value{0};
    opt->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage(opt, &int_value);
    opt->setRequired(true);
    ASSERT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt->setRequired(false);
    ASSERT_TRUE(parser.parse({}));
    //opt->valueSemantics().setDefaultValue(10); // TODO: Checker: positional options should not has default values, only implicit values are allowed
    opt->valueSemantics().setImplicitValue(10);
    ASSERT_FALSE(parser.storage.contains(opt));
    ASSERT_TRUE(parser.parse({}));
    EXPECT_EQ(int_value, 10);
    ASSERT_TRUE(parser.parse("20"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    ASSERT_EQ(parser.storage[opt].valueAs<int>(0), 20);
    EXPECT_EQ(int_value, 20);
    ASSERT_TRUE(parser.parse("20 30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), "30");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "30");
    EXPECT_EQ(int_value, 30);
    ASSERT_THROW(parser.parse("20 30 40"), TooManyPositionalOptions);
}

TEST(Parser2, MultipleValuesOfNamedOptionExact) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    int int_value{0};
    Parser2 parser(opt);
    parser.storage.setExternalStorage(opt, &int_value);

    ASSERT_THROW(parser.parse("--opt1"), ExpectedValue);

    opt->valueSemantics().setImplicitValue(11);
    opt->setMaxOccurreneCount(1);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::EXACT, 2);

    opt->valueSemantics().setExternalStorage(int_value);
    ASSERT_TRUE(parser.parse({}));
    EXPECT_EQ(int_value, 11); // implicit value
    ASSERT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(int_value, 30);
    ASSERT_THROW(parser.parse("--opt1 20,30 --opt1 40,40 --opt1 20,20"), MaxOptionOccurrenceIsExceeded);
    ASSERT_THROW(parser.parse("--opt1 20"), ExpectedExactNumberOfValues);
    ASSERT_THROW(parser.parse("--opt1 20,30 --opt1 40,40 --opt1"), MaxOptionOccurrenceIsExceeded);
}

TEST(Parser2, MultipleValuesOfNamedOptionUpTo) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    Parser2 parser(opt);
    int int_value{0};
    opt->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage(opt, &int_value);

    opt->setNValues(AbstractOptionWithValue::NValuesRole::INFINITE);
    ASSERT_THROW(parser.parse("--opt1"), ExpectedValue);

    opt->valueSemantics().setDefaultValue(10);
    opt->valueSemantics().setImplicitValue(11);
    opt->setMaxOccurreneCount(1);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 2);

    ASSERT_TRUE(parser.parse({}));
    EXPECT_EQ(int_value, 11); // implicit value

    ASSERT_TRUE(parser.parse({"--opt1"}));
    EXPECT_EQ(int_value, 10); // default value

    ASSERT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(int_value, 30);
    ASSERT_TRUE(parser.parse("--opt1 31"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "31");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "31");
    EXPECT_EQ(int_value, 31);
    ASSERT_THROW(parser.parse("--opt1 20 --opt1 30"), MaxOptionOccurrenceIsExceeded);
    // parse command finished with exception but parser storage should be
    // initialized with successfully parsed values
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), "30");
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0), "30");

    ASSERT_THROW(parser.parse("--opt1 20,30 filename"), TooManyPositionalOptions);

    ASSERT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(int_value, 30);
}

TEST(Parser2, MultipleValuesOfNamedOptionInfinite) {
    auto opt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    opt->valueSemantics().setDefaultValue(10);
    opt->valueSemantics().setImplicitValue(11);
    opt->setMaxOccurreneCount(1);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::INFINITE);

    Parser2 parser(opt);
    int int_value{0};
    opt->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage(opt, &int_value);
    ASSERT_TRUE(parser.parse({}));
    EXPECT_EQ(int_value, 11); // implicit value

    ASSERT_TRUE(parser.parse({"--opt1"}));
    EXPECT_EQ(int_value, 10); // default value


    ASSERT_TRUE(parser.parse("--opt1 20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(int_value, 30);
}


TEST(Parser2, MultipleValuesOfPositionalOption) {
    auto opt = std::make_shared<PositionalOptionWithValue<int>>();
    opt->setMaxOccurreneCount(2);
    opt->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 2);

    Parser2 parser(opt);
    int int_value{0};
    opt->valueSemantics().setExternalStorage(int_value);
    parser.storage.setExternalStorage(opt, &int_value);
    opt->setRequired(true);
    ASSERT_THROW(parser.parse({}), RequiredOptionIsNotSet);
    opt->setRequired(false);
    ASSERT_TRUE(parser.parse({}));
    opt->valueSemantics().setImplicitValue(11);
    ASSERT_FALSE(parser.storage.contains(opt));
    ASSERT_TRUE(parser.parse({}));
    EXPECT_EQ(int_value, 11); // implicit value
    ASSERT_TRUE(parser.parse("20"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 1);
    EXPECT_EQ(parser.storage[opt].occurrenceSize(0), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    EXPECT_EQ(parser.storage[opt].valueAs<int>(0), 20);
    EXPECT_EQ(int_value, 20);
    ASSERT_TRUE(parser.parse("20,30"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt].lastOccurrenceSize(), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(1), "30");
    EXPECT_EQ(int_value, 30);
    ASSERT_THROW(parser.parse("20,30,40"), TooManyValuesForOption);
    ASSERT_TRUE(parser.parse("20,30 40"));
    ASSERT_TRUE(parser.storage.contains(opt));
    ASSERT_EQ(parser.storage[opt].occurrenceCount(), 2);
    ASSERT_EQ(parser.storage[opt].occurrenceSize(0), 2);
    EXPECT_EQ(parser.storage[opt].rawValues(0, 0), "20");
    EXPECT_EQ(parser.storage[opt].rawValues(0, 1), "30");
    ASSERT_EQ(parser.storage[opt].occurrenceSize(1), 1);
    EXPECT_EQ(parser.storage[opt].rawValues(1, 0), "40");
    EXPECT_EQ(parser.storage[opt].rawValues(0), "40");
    EXPECT_EQ(int_value, 40);
    ASSERT_THROW(parser.parse("20,30 50 60"), TooManyPositionalOptions);
}

TEST(Parser2, TwoPositionalOptions) {
    auto opt1 = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto opt2 = std::make_shared<PositionalOptionWithValue<int>>();
    opt2->setMaxOccurreneCount(2);
    auto opt = std::make_shared<OptionsGroup>()->addUnlock(opt1)->addUnlock(opt2);

    Parser2 parser(opt);
    ASSERT_TRUE(parser.parse({}));
    ASSERT_TRUE(parser.parse("file1"));
    ASSERT_TRUE(parser.parse("file1 10"));
    ASSERT_TRUE(parser.parse("file1 10 20"));
    ASSERT_THROW(parser.parse("file1 10 20 30"), TooManyPositionalOptions);
}

TEST(Parser2, TwoPositionalOptionsWithDoubleDash) {
    auto opt1 = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto opt2 = std::make_shared<PositionalOptionWithValue<int>>();
    opt1->setMaxOccurreneCount(10);
    opt1->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 10);
    opt2->setNValues(AbstractNamedOptionWithValue::NValuesRole::UPTO, 2);
    auto opt = std::make_shared<OptionsGroup>()->addUnlock(opt1)->addUnlock(opt2);

    Parser2 parser(opt);
    ASSERT_TRUE(parser.parse("file1 file2 file3 -- 10"));
    ASSERT_EQ(parser.storage[opt1].occurrenceCount(), 3);
    ASSERT_EQ(parser.storage[opt1].occurrenceSize(0), 1);
    EXPECT_EQ(parser.storage[opt1].rawValues(0, 0), "file1");
    ASSERT_EQ(parser.storage[opt1].occurrenceSize(1), 1);
    EXPECT_EQ(parser.storage[opt1].rawValues(1, 0), "file2");
    ASSERT_EQ(parser.storage[opt1].occurrenceSize(1), 1);
    EXPECT_EQ(parser.storage[opt1].rawValues(2, 0), "file3");

    ASSERT_EQ(parser.storage[opt2].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[opt2].occurrenceSize(0), 1);
    ASSERT_EQ(parser.storage[opt2].rawValues(0, 0), "10");
}

TEST(Parser2, DoubleDashTerminatesNamedOptions) {
    auto named_opt_1 = std::make_shared<NamedOptionWithValue<int>>("--arg1");
    auto named_opt_2 = std::make_shared<NamedOptionWithValue<int>>("--arg2");
    auto pos_opt = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto opt = std::make_shared<OptionsGroup>()->addUnlock(named_opt_1)->addUnlock(named_opt_2)->addUnlock(pos_opt);

    Parser2 parser(opt);
    ASSERT_TRUE(parser.parse("--arg1 10 -- --arg2"));
    ASSERT_EQ(parser.storage[named_opt_1].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[named_opt_1].occurrenceSize(0), 1);
    EXPECT_EQ(parser.storage[named_opt_1].rawValues(0, 0), "10");
    ASSERT_FALSE(parser.storage.contains(named_opt_2));
    ASSERT_EQ(parser.storage[pos_opt].occurrenceCount(), 1);
    ASSERT_EQ(parser.storage[pos_opt].occurrenceSize(0), 1);
    ASSERT_EQ(parser.storage[pos_opt].rawValues(0, 0), "--arg2");
}

TEST(Parser2, PositionalAndNamed) {
    auto posopt = std::make_shared<PositionalOptionWithValue<std::string>>();
    auto namedopt = std::make_shared<NamedOptionWithValue<int>>("--opt1");
    posopt->setMaxOccurreneCount(2);
    namedopt->setMaxOccurreneCount(2);

    auto opts = std::make_shared<OptionsGroup>()->addUnlock(posopt)->addUnlock(namedopt);

    Parser2 parser(opts);
    ASSERT_TRUE(parser.parse(""));
    ASSERT_TRUE(parser.parse("--opt1 10"));
    ASSERT_TRUE(parser.parse("--opt1 10 filename"));
    ASSERT_TRUE(parser.parse("--opt1 10 --opt1 20 filename"));
    ASSERT_TRUE(parser.parse("--opt1 10 filename1 filename2"));
    ASSERT_TRUE(parser.parse("--opt1 10 --opt1 20 filename1 filename2"));
    ASSERT_THROW(parser.parse("--opt1 10 --opt1 20 filename1 filename2 filename3"), TooManyPositionalOptions);
    ASSERT_TRUE(parser.parse("filename --opt1 10 filename --opt1 20"));
}


TEST(Parser2, NestedAlternatives) {
    auto alt_nested_1 =
        std::make_shared<OneOfPositional>()
            ->addAlternative2(std::make_shared<LiteralString>("alt11")->addUnlock(std::make_shared<NamedOption>("--opt11")))
            ->addAlternative2(std::make_shared<LiteralString>("alt12")->addUnlock(std::make_shared<NamedOption>("--opt12")));
    auto alt_nested_2 =
        std::make_shared<OneOfPositional>()
            ->addAlternative2(std::make_shared<LiteralString>("alt21")->addUnlock(std::make_shared<NamedOption>("--opt21")))
            ->addAlternative2(std::make_shared<LiteralString>("alt22")->addUnlock(std::make_shared<NamedOption>("--opt22")));
    auto opts = std::make_shared<OneOfPositional>()
                    ->addAlternative2(std::make_shared<LiteralString>("alt1")->addUnlock(alt_nested_1))
                    ->addAlternative2(std::make_shared<LiteralString>("alt2")->addUnlock(alt_nested_2));

    Parser2 parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt11 --opt11"));
    ASSERT_NO_THROW(parser.parse("alt1 alt12 --opt12"));
    ASSERT_NO_THROW(parser.parse("alt2 alt21 --opt21"));
    ASSERT_NO_THROW(parser.parse("alt2 alt22 --opt22"));

    ASSERT_THROW(parser.parse("--opt11"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt12 --opt12 --unknown"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt2 --opt21"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt2"), UnexpectedValueForPositionalOption);
    ASSERT_THROW(parser.parse("alt1 alt11 alt12"), TooManyPositionalOptions);
}

TEST(Parser2, NestedAlternativesWithEqualNames) {
    auto alt_nested_1 =
        std::make_shared<OneOfPositional>()
            ->addAlternative2(std::make_shared<LiteralString>("alt1")->addUnlock(std::make_shared<NamedOption>("--opt1")))
            ->addAlternative2(std::make_shared<LiteralString>("alt2")->addUnlock(std::make_shared<NamedOption>("--opt2")));
    auto alt_nested_2 =
        std::make_shared<OneOfPositional>()
            ->addAlternative2(std::make_shared<LiteralString>("alt1")->addUnlock(std::make_shared<NamedOption>("--opt1")))
            ->addAlternative2(std::make_shared<LiteralString>("alt2")->addUnlock(std::make_shared<NamedOption>("--opt2")));
    auto opts = std::make_shared<OneOfPositional>()
                    ->addAlternative2(std::make_shared<LiteralString>("alt1")->addUnlock(alt_nested_1))
                    ->addAlternative2(std::make_shared<LiteralString>("alt2")->addUnlock(alt_nested_2));

    Parser2 parser(opts);
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

    Parser2 parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt11 --opt11"));
    ASSERT_NO_THROW(parser.parse("alt1 alt12 --opt12"));
    ASSERT_NO_THROW(parser.parse("alt2 alt21 --opt21"));
    ASSERT_NO_THROW(parser.parse("alt2 alt22 --opt22"));

    ASSERT_THROW(parser.parse("--opt11"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt12 --opt12 --unknown"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt2 --opt21"), UnknownNamedOption);
    ASSERT_THROW(parser.parse("alt1 alt2"), ValueIsNotAllowed);
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

    Parser2 parser(opts);
    ASSERT_NO_THROW(parser.parse("alt1 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt1 alt2 --opt2"));
    ASSERT_NO_THROW(parser.parse("alt2 alt1 --opt1"));
    ASSERT_NO_THROW(parser.parse("alt2 alt2 --opt2"));
}