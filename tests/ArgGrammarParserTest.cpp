#include <Backend/Matcher.h>
#include <gtest/gtest.h>

TEST(ArgGrammarParser, ShortOption) {
    ArgGrammarParser parser1("-x 10");
    EXPECT_FALSE(parser1.eof());
    parser1.getNextOption();
    EXPECT_EQ(parser1.current_result.token_type, ArgGrammarParser::SHORT_OPTION);
    EXPECT_EQ(parser1.current_result.short_option_name, "x");
    EXPECT_EQ(parser1.getValueOpt(nullptr).value(), "10");
    EXPECT_TRUE(parser1.eof());

    ArgGrammarParser parser2("-x=10");
    EXPECT_FALSE(parser2.eof());
    parser2.getNextOption();
    EXPECT_EQ(parser2.current_result.token_type, ArgGrammarParser::SHORT_OPTION_EQ_VALUE);
    EXPECT_EQ(parser2.current_result.short_option_name, "x");
    EXPECT_EQ(parser2.current_result.value, "10");
    EXPECT_EQ(parser2.getValueOpt(nullptr).value(), "10");
    EXPECT_TRUE(parser2.eof());

    ArgGrammarParser parser3("-xyz 10");
    EXPECT_FALSE(parser3.eof());
    parser3.getNextOption();
    EXPECT_EQ(parser3.current_result.token_type, ArgGrammarParser::SHORT_OPTION_WITHOUT_VALUE);
    EXPECT_EQ(parser3.current_result.short_option_name, "x");
    parser3.getNextOption();
    EXPECT_EQ(parser3.current_result.token_type, ArgGrammarParser::SHORT_OPTION_WITHOUT_VALUE);
    EXPECT_EQ(parser3.current_result.short_option_name, "y");
    parser3.getNextOption();
    EXPECT_EQ(parser3.current_result.token_type, ArgGrammarParser::SHORT_OPTION);
    EXPECT_EQ(parser3.current_result.short_option_name, "z");
    EXPECT_EQ(parser3.getValueOpt(nullptr).value(), "10");
    EXPECT_TRUE(parser3.eof());

    ArgGrammarParser parser4("-xyz=10");
    EXPECT_FALSE(parser4.eof());
    parser4.getNextOption();
    EXPECT_EQ(parser4.current_result.token_type, ArgGrammarParser::SHORT_OPTION_WITHOUT_VALUE);
    EXPECT_EQ(parser4.current_result.short_option_name, "x");
    parser4.getNextOption();
    EXPECT_EQ(parser4.current_result.token_type, ArgGrammarParser::SHORT_OPTION_WITHOUT_VALUE);
    EXPECT_EQ(parser4.current_result.short_option_name, "y");
    parser4.getNextOption();
    EXPECT_EQ(parser4.current_result.token_type, ArgGrammarParser::SHORT_OPTION_EQ_VALUE);
    EXPECT_EQ(parser4.current_result.short_option_name, "z");
    EXPECT_EQ(parser4.current_result.value, "10");
    EXPECT_EQ(parser4.getValueOpt(nullptr).value(), "10");
    EXPECT_TRUE(parser4.eof());

    ArgGrammarParser parser5("\\--escaped");
    EXPECT_FALSE(parser5.eof());
    parser5.getNextOption();
    EXPECT_EQ(parser5.current_result.token_type, ArgGrammarParser::VALUE);
    EXPECT_EQ(parser5.getValueOpt(nullptr).value(), "--escaped");
    EXPECT_TRUE(parser5.eof());
}

TEST(ArgGrammarParser, LongOption) {
    ArgGrammarParser parser1("--dim 4");
    EXPECT_FALSE(parser1.eof());
    parser1.getNextOption();
    EXPECT_EQ(parser1.current_result.token_type, ArgGrammarParser::LONG_OPTION);
    EXPECT_EQ(parser1.current_result.long_option_name, "dim");
    EXPECT_EQ(parser1.getValueOpt(nullptr).value(), "4");
    EXPECT_TRUE(parser1.eof());

    ArgGrammarParser parser2("--dim=4");
    EXPECT_FALSE(parser2.eof());
    parser2.getNextOption();
    EXPECT_EQ(parser2.current_result.token_type, ArgGrammarParser::LONG_OPTION_EQ_VALUE);
    EXPECT_EQ(parser2.current_result.long_option_name, "dim");
    EXPECT_EQ(parser2.current_result.value, "4");
    EXPECT_EQ(parser2.getValueOpt(nullptr).value(), "4");
    EXPECT_TRUE(parser2.eof());
}

TEST(ArgGrammarParser, Value) {
    ArgGrammarParser parser1("abracadabra");
    EXPECT_FALSE(parser1.eof());
    parser1.getNextOption();
    EXPECT_EQ(parser1.current_result.token_type, ArgGrammarParser::VALUE);
    EXPECT_EQ(parser1.current_result.value, "abracadabra");
    EXPECT_EQ(parser1.getValueOpt(nullptr).value(), "abracadabra");
    EXPECT_TRUE(parser1.eof());

    ArgGrammarParser parser2(std::vector<std::string>{"abra cadabra"});  // the whole string is treated as a single argument,
                                                                         // space doesn't matter
    EXPECT_FALSE(parser2.eof());
    parser2.getNextOption();
    EXPECT_EQ(parser2.current_result.token_type, ArgGrammarParser::VALUE);
    EXPECT_EQ(parser2.current_result.value, "abra cadabra");
    EXPECT_EQ(parser2.getValueOpt(nullptr).value(), "abra cadabra");
    EXPECT_TRUE(parser2.eof());
}

TEST(ArgGrammarParser, DoubleDash) {
    ArgGrammarParser parser1("--");
    EXPECT_FALSE(parser1.eof());
    parser1.getNextOption();
    EXPECT_EQ(parser1.current_result.token_type, ArgGrammarParser::DOUBLE_DASH);
    EXPECT_THROW(parser1.getValueOpt(nullptr).value(), ExpectedValue);
    EXPECT_TRUE(parser1.eof());

    ArgGrammarParser parser2(std::vector<std::string>{"-- cadabra"});
    EXPECT_FALSE(parser2.eof());
    parser2.getNextOption();
    EXPECT_EQ(parser2.current_result.token_type, ArgGrammarParser::VALUE);
}