#include <Backend/Matcher.h>
#include <gtest/gtest.h>

#include <boost/spirit/include/qi.hpp>

TEST(ArgumentsParser, Test1) {
    auto minus = boost::spirit::qi::char_('-');
    auto short_options_group = minus >> boost::spirit::qi::alnum;
    auto long_option = boost::spirit::qi::repeat(2)[minus]  >> boost::spirit::qi::alnum;
}