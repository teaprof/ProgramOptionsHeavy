#include <Backend/Matcher.h>
#include <gtest/gtest.h>

TEST(ArgGrammarParser, ShortOption) {    
    ArgGrammarParser parser1("-x 10");
    EXPECT_FALSE(parser1.eof());
    parser1.getNextOption();
    EXPECT_EQ(parser1.current_result.token_type, ArgGrammarParser::short_option);
    EXPECT_EQ(parser1.current_result.short_option_name, "x");
    EXPECT_EQ(parser1.getValue(), "10");
    EXPECT_TRUE(parser1.eof());

    ArgGrammarParser parser2("-x=10");
    EXPECT_FALSE(parser2.eof());
    parser2.getNextOption();
    EXPECT_EQ(parser2.current_result.token_type, ArgGrammarParser::short_option_eq_value);
    EXPECT_EQ(parser2.current_result.short_option_name, "x");
    EXPECT_EQ(parser2.current_result.value, "10");
    EXPECT_EQ(parser2.getValue(), "10");
    EXPECT_TRUE(parser2.eof());

    ArgGrammarParser parser3("-xyz 10");
    EXPECT_FALSE(parser3.eof());
    parser3.getNextOption();
    EXPECT_EQ(parser3.current_result.token_type, ArgGrammarParser::short_option_without_value);
    EXPECT_EQ(parser3.current_result.short_option_name, "x");
    parser3.getNextOption();
    EXPECT_EQ(parser3.current_result.token_type, ArgGrammarParser::short_option_without_value);
    EXPECT_EQ(parser3.current_result.short_option_name, "y");
    parser3.getNextOption();
    EXPECT_EQ(parser3.current_result.token_type, ArgGrammarParser::short_option);
    EXPECT_EQ(parser3.current_result.short_option_name, "z");
    EXPECT_EQ(parser3.getValue(), "10");
    EXPECT_TRUE(parser3.eof());

    ArgGrammarParser parser4("-xyz=10");
    EXPECT_FALSE(parser4.eof());
    parser4.getNextOption();
    EXPECT_EQ(parser4.current_result.token_type, ArgGrammarParser::short_option_without_value);
    EXPECT_EQ(parser4.current_result.short_option_name, "x");
    parser4.getNextOption();
    EXPECT_EQ(parser4.current_result.token_type, ArgGrammarParser::short_option_without_value);
    EXPECT_EQ(parser4.current_result.short_option_name, "y");
    parser4.getNextOption();
    EXPECT_EQ(parser4.current_result.token_type, ArgGrammarParser::short_option_eq_value);
    EXPECT_EQ(parser4.current_result.short_option_name, "z");
    EXPECT_EQ(parser4.current_result.value, "10");
    EXPECT_EQ(parser4.getValue(), "10");
    EXPECT_TRUE(parser4.eof());
}

TEST(ArgGrammarParser, LongOption) {    
    ArgGrammarParser parser1("--dim 4");
    EXPECT_FALSE(parser1.eof());
    parser1.getNextOption();
    EXPECT_EQ(parser1.current_result.token_type, ArgGrammarParser::long_option);
    EXPECT_EQ(parser1.current_result.long_option_name, "dim");
    EXPECT_EQ(parser1.getValue(), "4");
    EXPECT_TRUE(parser1.eof());


    ArgGrammarParser parser2("--dim=4");
    EXPECT_FALSE(parser2.eof());
    parser2.getNextOption();
    EXPECT_EQ(parser2.current_result.token_type, ArgGrammarParser::long_option_eq_value);
    EXPECT_EQ(parser2.current_result.long_option_name, "dim");
    EXPECT_EQ(parser2.current_result.value, "4");
    EXPECT_EQ(parser2.getValue(), "4");
    EXPECT_TRUE(parser2.eof());
}

TEST(ArgGrammarParser, Value) {    
    ArgGrammarParser parser1("abracadabra");
    EXPECT_FALSE(parser1.eof());
    parser1.getNextOption();
    EXPECT_EQ(parser1.current_result.token_type, ArgGrammarParser::value);
    EXPECT_EQ(parser1.current_result.value, "abracadabra");
    EXPECT_EQ(parser1.getValue(), "abracadabra");
    EXPECT_TRUE(parser1.eof());


    ArgGrammarParser parser2(std::vector<std::string>{"abra cadabra"});
    EXPECT_FALSE(parser2.eof());
    parser2.getNextOption();
    EXPECT_EQ(parser2.current_result.token_type, ArgGrammarParser::value);
    EXPECT_EQ(parser2.current_result.value, "abra cadabra");
    EXPECT_EQ(parser2.getValue(), "abra cadabra");
    EXPECT_TRUE(parser2.eof());
}