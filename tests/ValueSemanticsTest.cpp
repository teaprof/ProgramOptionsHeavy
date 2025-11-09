#include <Backend/ValueSemantics.h>
#include <gtest/gtest.h>

TEST(ValueSematics, IntValueSemantics) {
    ValueSemantics<int> semantics;
    int value;
    ASSERT_NO_THROW(semantics.setValue2("10", &value));
    ASSERT_EQ(value, 10);
    ASSERT_THROW(semantics.setValue2("a10", &value), InvalidValueType);    
    ASSERT_EQ(value, 10); // value should not be changed in case of throw
    ASSERT_THROW(semantics.setValue2("10a", &value), InvalidValueType);
    ASSERT_EQ(value, 10);
    ASSERT_THROW(semantics.setValue2("999999999999999999999999999999999999999999999999999999999", &value), ValueIsOutOfRange);
    ASSERT_EQ(value, 10);
    semantics.setMin(-10);
    ASSERT_NO_THROW(semantics.setValue2("-10", &value));
    ASSERT_EQ(value, -10);
    ASSERT_NO_THROW(semantics.setValue2("10", &value));
    ASSERT_EQ(value, 10);
    ASSERT_THROW(semantics.setValue2("-11", &value), ValueIsOutOfRange);
    ASSERT_EQ(value, 10);
    semantics.setMax(10);
    ASSERT_NO_THROW(semantics.setValue2("-5", &value));
    ASSERT_EQ(value, -5);
    ASSERT_NO_THROW(semantics.setValue2("10", &value));
    ASSERT_EQ(value, 10);
    ASSERT_THROW(semantics.setValue2("-11", &value), ValueIsOutOfRange);
    ASSERT_EQ(value, 10);
    ASSERT_THROW(semantics.setValue2("11", &value), ValueIsOutOfRange);
    ASSERT_EQ(value, 10);
    semantics.setMinMax(-5, 5);
    ASSERT_NO_THROW(semantics.setValue2("0", &value));
    ASSERT_EQ(value, 0);
    ASSERT_THROW(semantics.setValue2("-6", &value), ValueIsOutOfRange);
    ASSERT_EQ(value, 0);
    ASSERT_THROW(semantics.setValue2("-6", &value), ValueIsOutOfRange);
    ASSERT_EQ(value, 0);
}


