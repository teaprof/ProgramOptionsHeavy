#include <Backend/Matcher.h>
#include <gtest/gtest.h>

TEST(ArgumentsLexer, SimpleTests) {
    auto res = ArgLexer::lex("--dim");
    EXPECT_EQ(res.type, ArgLexer::long_option);
    EXPECT_EQ(res.long_option_name, "dim");
    res = ArgLexer::lex("--dim-1");
    EXPECT_EQ(res.type, ArgLexer::long_option);
    EXPECT_EQ(res.long_option_name, "dim-1");
    res = ArgLexer::lex("--dim_1");
    EXPECT_EQ(res.type, ArgLexer::long_option);
    EXPECT_EQ(res.long_option_name, "dim_1");
    res = ArgLexer::lex("--dim+1");
    EXPECT_EQ(res.type, ArgLexer::value);
    EXPECT_EQ(res.value, "--dim+1");
    res = ArgLexer::lex("--dim -a");
    EXPECT_EQ(res.type, ArgLexer::value);
    EXPECT_EQ(res.value, "--dim -a");
}

TEST(ArgumentsLexer, ComplexTests) {
    auto res = ArgLexer::lex("--dim");
    EXPECT_EQ(res.type, ArgLexer::long_option);
    EXPECT_EQ(res.long_option_name, "dim");
    res = ArgLexer::lex("--dim=4");
    EXPECT_EQ(res.type, ArgLexer::long_option_eq_value);
    EXPECT_EQ(res.long_option_name, "dim");
    EXPECT_EQ(res.value, "4");
    res = ArgLexer::lex("-d");
    EXPECT_EQ(res.type, ArgLexer::short_options);
    EXPECT_EQ(res.short_option_names, "d");
    res = ArgLexer::lex("-xyz");
    EXPECT_EQ(res.type, ArgLexer::short_options);
    EXPECT_EQ(res.short_option_names, "xyz");    
    res = ArgLexer::lex("-xyz=4");
    EXPECT_EQ(res.type, ArgLexer::short_options_eq_value);
    EXPECT_EQ(res.short_option_names, "xyz");
    EXPECT_EQ(res.value, "4");
}

