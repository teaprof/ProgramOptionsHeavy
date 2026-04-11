#include <Backend/Matcher.h>
#include <gtest/gtest.h>

TEST(ArgumentsLexer, TestBlock1) {
    auto res = ArgLexer::lex("--dim", false);
    EXPECT_EQ(res.type, ArgLexer::LONG_OPTION);
    EXPECT_EQ(res.long_option_name, "dim");
    res = ArgLexer::lex("--dim-1", false);
    EXPECT_EQ(res.type, ArgLexer::LONG_OPTION);
    EXPECT_EQ(res.long_option_name, "dim-1");
    res = ArgLexer::lex("--dim_1", false);
    EXPECT_EQ(res.type, ArgLexer::LONG_OPTION);
    EXPECT_EQ(res.long_option_name, "dim_1");
    res = ArgLexer::lex("--dim+1", false);
    EXPECT_EQ(res.type, ArgLexer::VALUE);
    EXPECT_EQ(res.value, "--dim+1");
    res = ArgLexer::lex("--dim -a", false); // this treater as a single string, white space doesn't matt, falseer
    EXPECT_EQ(res.type, ArgLexer::VALUE);
    EXPECT_EQ(res.value, "--dim -a");
    res = ArgLexer::lex("--", false);
    EXPECT_EQ(res.type, ArgLexer::DOUBLE_DASH);
    res = ArgLexer::lex("---a", false);
    EXPECT_EQ(res.type, ArgLexer::VALUE);
    EXPECT_EQ(res.value, "---a");
}    

TEST(ArgumentsLexer, TestBlock2) {
    auto res = ArgLexer::lex("--dim", false);
    EXPECT_EQ(res.type, ArgLexer::LONG_OPTION);
    EXPECT_EQ(res.long_option_name, "dim");
    res = ArgLexer::lex("--dim=4", false);
    EXPECT_EQ(res.type, ArgLexer::LONG_OPTION_EQ_VALUE);
    EXPECT_EQ(res.long_option_name, "dim");
    EXPECT_EQ(res.value, "4");
    res = ArgLexer::lex("-d", false);
    EXPECT_EQ(res.type, ArgLexer::SHORT_OPTIONS);
    EXPECT_EQ(res.short_option_names, "d");
    res = ArgLexer::lex("-xyz", false);
    EXPECT_EQ(res.type, ArgLexer::SHORT_OPTIONS);
    EXPECT_EQ(res.short_option_names, "xyz");    
    res = ArgLexer::lex("-xyz=4", false);
    EXPECT_EQ(res.type, ArgLexer::SHORT_OPTIONS_EQ_VALUE);
    EXPECT_EQ(res.short_option_names, "xyz");
    EXPECT_EQ(res.value, "4");
}

TEST(ArgumentsLexer, TestBlock3) {
    auto res = ArgLexer::lex("--dim", true);
    EXPECT_EQ(res.type, ArgLexer::VALUE);
    res = ArgLexer::lex("--dim=10", true);
    EXPECT_EQ(res.type, ArgLexer::VALUE);
    res = ArgLexer::lex("-d", true);
    EXPECT_EQ(res.type, ArgLexer::VALUE);
    res = ArgLexer::lex("-d=10", true);
    EXPECT_EQ(res.type, ArgLexer::VALUE);
}    

