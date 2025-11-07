enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(poheavy_tests 
    program_mode_options_test.cpp 
    completer_test.cpp 
    CheckerTest.cpp
    MatcherTest.cpp
    ArgumentsTokenizerTest.cpp)
target_link_libraries(poheavy_tests GTest::gtest_main Boost::program_options)
target_include_directories(poheavy_tests PUBLIC GTEST_INCLUDE_DIRS)

gtest_discover_tests(poheavy_tests)