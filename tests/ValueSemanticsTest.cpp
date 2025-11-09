#include <Backend/ValueSemantics.h>
#include <gtest/gtest.h>

TEST(ValueSematics, IntValueSemantics) {
    ValueSemantics<int> semantics;
    ASSERT_NO_THROW(semantics.semanticParse("10"));
    ASSERT_THROW(semantics.semanticParse("a10"), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("10a"), InvalidValueType);
    ASSERT_THROW(semantics.semanticParse("99999999999999999999999999999999999999999999999"), ValueIsOutOfRange);
    semantics.setMin(-10);
    ASSERT_NO_THROW(semantics.semanticParse("-10"));
    ASSERT_NO_THROW(semantics.semanticParse("10"));
    ASSERT_THROW(semantics.semanticParse("-11"), ValueIsOutOfRange);
    semantics.setMax(10);
    ASSERT_NO_THROW(semantics.semanticParse("-5"));
    ASSERT_NO_THROW(semantics.semanticParse("10"));
    ASSERT_THROW(semantics.semanticParse("-11"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("11"), ValueIsOutOfRange);
    semantics.setMinMax(-5, 5);
    ASSERT_NO_THROW(semantics.semanticParse("0"));
    ASSERT_THROW(semantics.semanticParse("-6"), ValueIsOutOfRange);
    ASSERT_THROW(semantics.semanticParse("-6"), ValueIsOutOfRange);
}


