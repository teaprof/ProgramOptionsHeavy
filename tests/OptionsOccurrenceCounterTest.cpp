#include <Backend/Parser.h>
#include <gtest/gtest.h>

// Test initial state: no options have been encountered yet
TEST(OptionsOccurrenceCounterTest, InitialState) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");

    // optionEncountered returns 0 for any option that hasn't been counted yet
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(0));

    // canAcceptNewOccurrence returns true since actual_count (0) < maxOccurrence (1)
    EXPECT_TRUE(counter.canAcceptNewOccurrence(opt));
}

// Test first occurrence with default maxOccurrence (1)
TEST(OptionsOccurrenceCounterTest, FirstOccurrenceReachesMax) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");

    // First occurrence: counter becomes 1, which equals maxOccurrence (1), so returns true
    bool result = counter.increaseOptionOccurrenceCounter(opt);
    EXPECT_TRUE(result);
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(1));

    // After reaching max, canAcceptNewOccurrence returns false
    EXPECT_FALSE(counter.canAcceptNewOccurrence(opt));
}

// Test multiple occurrences with custom maxOccurrence (3)
TEST(OptionsOccurrenceCounterTest, CustomMaxOccurrence) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");
    opt->setMaxOccurreneCount(3);

    // 1st occurrence: 1 < 3, returns false
    EXPECT_FALSE(counter.increaseOptionOccurrenceCounter(opt));
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(1));
    EXPECT_TRUE(counter.canAcceptNewOccurrence(opt));

    // 2nd occurrence: 2 < 3, returns false
    EXPECT_FALSE(counter.increaseOptionOccurrenceCounter(opt));
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(2));
    EXPECT_TRUE(counter.canAcceptNewOccurrence(opt));

    // 3rd occurrence: 3 == 3, returns true
    EXPECT_TRUE(counter.increaseOptionOccurrenceCounter(opt));
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(3));
    EXPECT_FALSE(counter.canAcceptNewOccurrence(opt));
}

// Test that exceeding maxOccurrence throws MaxOptionOccurrenceIsExceeded
TEST(OptionsOccurrenceCounterTest, ExceedMaxOccurrenceThrows) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");
    // default maxOccurrence is 1

    // First occurrence reaches max
    counter.increaseOptionOccurrenceCounter(opt);

    // Second occurrence exceeds max -> throws
    ASSERT_THROW(counter.increaseOptionOccurrenceCounter(opt), MaxOptionOccurrenceIsExceeded);
}

// Test that exceeding maxOccurrence on a OneOfPositional throws OnlyOneChoiseIsAllowed
TEST(OptionsOccurrenceCounterTest, OneOfPositionalExceededThrowsOnlyOneChoice) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<OneOfPositional>();
    // default maxOccurrence is 1

    // First occurrence reaches max
    counter.increaseOptionOccurrenceCounter(opt);

    // Second occurrence exceeds max -> throws OnlyOneChoiseIsAllowed
    ASSERT_THROW(counter.increaseOptionOccurrenceCounter(opt), OnlyOneChoiseIsAllowed);
}

// Test that clear() resets all counters
TEST(OptionsOccurrenceCounterTest, ClearResetsCounters) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");

    counter.increaseOptionOccurrenceCounter(opt);
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(1));

    counter.clear();
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(0));
    EXPECT_TRUE(counter.canAcceptNewOccurrence(opt));
}

// Test that counters for different options are independent
TEST(OptionsOccurrenceCounterTest, IndependentOptionCounters) {
    OptionsOccurrenceCounter counter;
    auto opt1 = std::make_shared<NamedOption>("--opt1", "-1");
    auto opt2 = std::make_shared<NamedOption>("--opt2", "-2");
    opt1->setMaxOccurreneCount(3); // allow multiple occurrences for opt1

    // Increment opt1 twice
    counter.increaseOptionOccurrenceCounter(opt1);
    counter.increaseOptionOccurrenceCounter(opt1);

    // opt1 should have count 2, opt2 should still be 0
    EXPECT_EQ(counter.optionEncountered(opt1), static_cast<size_t>(2));
    EXPECT_EQ(counter.optionEncountered(opt2), static_cast<size_t>(0));
}

// Test canAcceptNewOccurrence across various states
TEST(OptionsOccurrenceCounterTest, CanAcceptNewOccurrenceScenarios) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");
    opt->setMaxOccurreneCount(3);

    // Initial: 0 < 3 -> can accept
    EXPECT_TRUE(counter.canAcceptNewOccurrence(opt));

    // After 1st occurrence: 1 < 3 -> can accept
    counter.increaseOptionOccurrenceCounter(opt);
    EXPECT_TRUE(counter.canAcceptNewOccurrence(opt));

    // After 2nd occurrence: 2 < 3 -> can accept
    counter.increaseOptionOccurrenceCounter(opt);
    EXPECT_TRUE(counter.canAcceptNewOccurrence(opt));

    // After 3rd occurrence: 3 == 3 -> cannot accept
    counter.increaseOptionOccurrenceCounter(opt);
    EXPECT_FALSE(counter.canAcceptNewOccurrence(opt));
}

// Test that optionEncountered returns 0 for options never registered
TEST(OptionsOccurrenceCounterTest, OptionNotEncounteredReturnsZero) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");

    // No calls to increaseOptionOccurrenceCounter -> should be 0
    EXPECT_EQ(counter.optionEncountered(opt), static_cast<size_t>(0));
}

// Test the exact boundary: maxOccurrence of 0 means no occurrences allowed
TEST(OptionsOccurrenceCounterTest, MaxOccurrenceZero) {
    OptionsOccurrenceCounter counter;
    auto opt = std::make_shared<NamedOption>("--test", "-t");
    opt->setMaxOccurreneCount(0);

    // canAcceptNewOccurrence: 0 < 0 -> false
    EXPECT_FALSE(counter.canAcceptNewOccurrence(opt));

    // First occurrence: counter becomes 1, 1 > 0 -> throws
    ASSERT_THROW(counter.increaseOptionOccurrenceCounter(opt), MaxOptionOccurrenceIsExceeded);
}