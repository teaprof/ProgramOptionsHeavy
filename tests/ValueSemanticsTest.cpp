#include <Backend/ValueSemantics.h>
#include <gtest/gtest.h>

TEST(ValueSematics, IntValueSemantics) {
    ValueSemantics<int> v;
    ASSERT_NO_THROW(v.setValue("10"));
    ASSERT_THROW(v.setValue("a10"), InvalidValueType);    
    ASSERT_THROW(v.setValue("10a"), InvalidValueType);
    ASSERT_THROW(v.setValue("999999999999999999999999999999999999999999999999999999999"), ValueIsOutOfRange);
    v.setMin(-10);
    ASSERT_NO_THROW(v.setValue("-10"));
    ASSERT_NO_THROW(v.setValue("10"));
    ASSERT_THROW(v.setValue("-11"), ValueIsOutOfRange);
    v.setMax(10);
    ASSERT_NO_THROW(v.setValue("-5"));
    ASSERT_NO_THROW(v.setValue("10"));
    ASSERT_THROW(v.setValue("-11"), ValueIsOutOfRange);
    ASSERT_THROW(v.setValue("11"), ValueIsOutOfRange);
    v.setMinMax(-5, 5);
    ASSERT_NO_THROW(v.setValue("0"));
    ASSERT_THROW(v.setValue("-6"), ValueIsOutOfRange);
    ASSERT_THROW(v.setValue("-6"), ValueIsOutOfRange);
}


