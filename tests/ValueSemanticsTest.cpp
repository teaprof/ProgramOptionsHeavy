#include <Backend/ValueSemantics.h>
#include <gtest/gtest.h>

TEST(ValueSematics, TrimTest) {
    using dest_type = TypedSemanticParseResult<int>;
    ValueSemantics<int> semantics;
    semantics.setMinMax(-10, 10);
    auto r = std::static_pointer_cast<dest_type>(semantics.semanticParse("  10  "));
    ASSERT_EQ(r->value, 10);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("  9"));
    ASSERT_EQ(r->value, 9);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("8  "));
    ASSERT_EQ(r->value, 8);
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
    auto r = std::static_pointer_cast<dest_type>(semantics.semanticParse("10"));
    ASSERT_EQ(r->value, 10);
    ASSERT_THROW(semantics.semanticParse("a10"), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("10a"), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("99999999999999999999999999999999999999999999999"), ValueIsOutOfRange);
    semantics.setMin(-10);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("-10"));
    ASSERT_EQ(r->value, -10);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("10"));
    ASSERT_EQ(r->value, 10);
    ASSERT_THROW(semantics.semanticParse("-11"), ValueIsOutOfRange);
    semantics.setMax(10);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("-5"));
    ASSERT_EQ(r->value, -5);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("10"));
    ASSERT_EQ(r->value, 10);
    ASSERT_THROW(semantics.semanticParse("-11"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("11"), ValueIsOutOfRange);
    semantics.setMinMax(-5, 5);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("0"));
    ASSERT_EQ(r->value, 0);
    ASSERT_THROW(semantics.semanticParse("-6"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("-6"), ValueIsOutOfRange);
}

TEST(ValueSematics, UnsignedInt) {
    using dest_type = TypedSemanticParseResult<unsigned int>;
    ValueSemantics<unsigned int> semantics;
    auto r = std::static_pointer_cast<dest_type>(semantics.semanticParse("10"));
    ASSERT_EQ(r->value, 10);
    ASSERT_THROW(semantics.semanticParse("-10"), ValueIsOutOfRange);
}

TEST(ValueSematics, Float) {
    using dest_type = TypedSemanticParseResult<float>;
    ValueSemantics<float> semantics;
    auto r = std::static_pointer_cast<dest_type>(semantics.semanticParse("1.2"));
    ASSERT_NEAR(r->value, 1.2, 1e-5);
    semantics.setMinMax(-1.0, 1.0);
    ASSERT_THROW(semantics.semanticParse("-1.2"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("1.2"), ValueIsOutOfRange);
}

TEST(ValueSematics, String) {
    using dest_type = TypedSemanticParseResult<std::string>;
    ValueSemantics<std::string> semantics;
    auto r = std::static_pointer_cast<dest_type>(semantics.semanticParse(" abdacadabra "));
    ASSERT_EQ(r->value, " abdacadabra ");
    semantics.setRegex(std::regex("[A-Z]+"), "[A-Z]+");
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("ABRADABRA"));
    ASSERT_EQ(r->value, "ABRADABRA");
    ASSERT_THROW(semantics.semanticParse("123"), ValueMustMatchRegex);
}

TEST(ValueSematics, Bool) {
    using dest_type = TypedSemanticParseResult<bool>;
    ValueSemantics<bool> semantics;
    auto r = std::static_pointer_cast<dest_type>(semantics.semanticParse(" TRUE"));
    ASSERT_TRUE(r->value);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("FALSE "));
    ASSERT_FALSE(r->value);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse(" True"));
    ASSERT_TRUE(r->value);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("False "));
    ASSERT_FALSE(r->value);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse(" true"));
    ASSERT_TRUE(r->value);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("false "));
    ASSERT_FALSE(r->value);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse(" 1"));
    ASSERT_TRUE(r->value);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("0 "));
    ASSERT_FALSE(r->value);
}

TEST(ValueSematics, UnlockByValue) {
    using dest_type = TypedSemanticParseResult<int>;
    ValueSemantics<int> semantics;
    semantics.unlocks(0);
    semantics.unlocks(1).push_back(nullptr);
    auto r = std::static_pointer_cast<dest_type>(semantics.semanticParse("0"));
    ASSERT_EQ(r->unlocks.size(), 0);
    r = std::static_pointer_cast<dest_type>(semantics.semanticParse("1"));
    ASSERT_EQ(r->unlocks.size(), 1);
    ASSERT_EQ(r->unlocks[0], nullptr);
}


TEST(ValueSematics, InvalidValue) {
    using dest_type = TypedSemanticParseResult<int>;
    ValueSemantics<int> semantics;
    semantics.unlocks(0);
    semantics.unlocks(1).push_back(nullptr);
    ASSERT_THROW(semantics.semanticParse("2"), InvalidOptionValue);
}
