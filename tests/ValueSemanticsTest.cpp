#include <Backend/ValueSemantics.h>
#include <gtest/gtest.h>

TEST(ValueSematics, TrimTest) {
    using dest_type = TypedSemanticParseResult<int>;
    ValueSemantics<int> semantics;
    semantics.setMinMax(-10, 10);
    semantics.semanticParse("  10  ");
    ASSERT_EQ(semantics.value(), 10);
    semantics.semanticParse("  9");
    ASSERT_EQ(semantics.value(), 9);
    semantics.semanticParse("8  ");
    ASSERT_EQ(semantics.value(), 8);
    ASSERT_THROW(semantics.semanticParse("  a10"), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("10a  "), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("  99999999999999999999999999999999999999999999999"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("11"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("  11"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("11  "), ValueIsOutOfRange);
}


TEST(ValueSematics, Int) {
    using dest_type = TypedSemanticParseResult<int>;
    ValueSemantics<int> semantics;
    semantics.semanticParse("10");
    ASSERT_EQ(semantics.value(), 10);
    ASSERT_THROW(semantics.semanticParse("a10"), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("10a"), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("99999999999999999999999999999999999999999999999"), ValueIsOutOfRange);
    semantics.setMin(-10);
    semantics.semanticParse("-10");
    ASSERT_EQ(semantics.value(), -10);
    semantics.semanticParse("10");
    ASSERT_EQ(semantics.value(), 10);
    ASSERT_THROW(semantics.semanticParse("-11"), ValueIsOutOfRange);
    semantics.setMax(10);
    semantics.semanticParse("-5");
    ASSERT_EQ(semantics.value(), -5);
    semantics.semanticParse("10");
    ASSERT_EQ(semantics.value(), 10);
    ASSERT_THROW(semantics.semanticParse("-11"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("11"), ValueIsOutOfRange);
    semantics.setMinMax(-5, 5);
    semantics.semanticParse("0");
    ASSERT_EQ(semantics.value(), 0);
    ASSERT_THROW(semantics.semanticParse("-6"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("-6"), ValueIsOutOfRange);
}

TEST(ValueSematics, UnsignedInt) {
    using dest_type = TypedSemanticParseResult<unsigned int>;
    ValueSemantics<unsigned int> semantics;
    semantics.semanticParse("10");
    ASSERT_EQ(semantics.value(), 10);
    ASSERT_THROW(semantics.semanticParse("-10"), ValueIsOutOfRange);
}

TEST(ValueSematics, Float) {
    using dest_type = TypedSemanticParseResult<float>;
    ValueSemantics<float> semantics;
    semantics.semanticParse("1.2");
    ASSERT_NEAR(semantics.value(), 1.2, 1e-5);
    semantics.setMinMax(-1.0, 1.0);
    ASSERT_THROW(semantics.semanticParse("-1.2"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("1.2"), ValueIsOutOfRange);
}

TEST(ValueSematics, String) {
    using dest_type = TypedSemanticParseResult<std::string>;
    ValueSemantics<std::string> semantics;
    semantics.semanticParse(" abdacadabra ");
    ASSERT_EQ(semantics.value(), " abdacadabra ");
    semantics.setRegex(std::regex("[A-Z]+"), "[A-Z]+");
    semantics.semanticParse("ABRADABRA");
    ASSERT_EQ(semantics.value(), "ABRADABRA");
    ASSERT_THROW(semantics.semanticParse("123"), ValueMustMatchRegex);
}

TEST(ValueSematics, Bool) {
    using dest_type = TypedSemanticParseResult<bool>;
    ValueSemantics<bool> semantics;
    semantics.semanticParse(" TRUE");
    ASSERT_TRUE(semantics.value());
    semantics.semanticParse("FALSE ");
    ASSERT_FALSE(semantics.value());
    semantics.semanticParse(" True");
    ASSERT_TRUE(semantics.value());
    semantics.semanticParse("False ");
    ASSERT_FALSE(semantics.value());
    semantics.semanticParse(" true");
    ASSERT_TRUE(semantics.value());
    semantics.semanticParse("false ");
    ASSERT_FALSE(semantics.value());
    semantics.semanticParse(" 1");
    ASSERT_TRUE(semantics.value());
    semantics.semanticParse("0 ");
    ASSERT_FALSE(semantics.value());
}

TEST(ValueSematics, UnlockByValue) {
    using dest_type = TypedSemanticParseResult<int>;
    ValueSemantics<int> semantics;
    semantics.unlocks(0);
    semantics.unlocks(1).push_back(nullptr);
    semantics.semanticParse("0");
    ASSERT_EQ(semantics.getUnlocks().size(), 0);
    semantics.semanticParse("1");
    ASSERT_EQ(semantics.getUnlocks().size(), 1);
    ASSERT_EQ(semantics.getUnlocks()[0], nullptr);
}


TEST(ValueSematics, InvalidValue) {
    using dest_type = TypedSemanticParseResult<int>;
    ValueSemantics<int> semantics;
    semantics.unlocks(0);
    semantics.unlocks(1).push_back(nullptr);
    semantics.setOnlyAllowedValues(true);
    ASSERT_THROW(semantics.semanticParse("2"), InvalidOptionValue);
}
