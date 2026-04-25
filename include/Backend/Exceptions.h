#ifndef BACKEND_EXCEPTIONS_H
#define BACKEND_EXCEPTIONS_H

#include <memory>
#include <sstream>
#include <stdexcept>

#include "Option.h"

class BaseError : public std::runtime_error {
   public:
    BaseError(const std::string& str) : std::runtime_error(description(str)) {}
    static std::string description(const std::string& str) {
        return std::string("Option error: ") + str;
    }
   private:
};

class BaseOptionError : public BaseError {
   public:
    BaseOptionError(const std::string& message, std::shared_ptr<AbstractOption> opt) : BaseError(description(message, opt)), opt_{opt} {}
    static std::string description(const std::string& message, std::shared_ptr<AbstractOption> opt) {
        std::stringstream str;
        str << "Option error: "<<message<<" ";
        str << BaseOptionError::displayName(opt);
        return str.str();
    }

    static std::string displayName(std::shared_ptr<AbstractOption> opt) {
        return ""; // TODO: implement
    }

    std::shared_ptr<AbstractOption> opt() { return opt_; }

   private:
    std::shared_ptr<AbstractOption> opt_;
};

// Options group 1 TODO: rename this
class UnknownNamedOption : public BaseError {                                               
   public:
    UnknownNamedOption(std::string str) : BaseError(std::string("Unknown  named option: " + str)) {}
};

class MaxOptionOccurrenceIsExceeded : public BaseOptionError {
   public:
    MaxOptionOccurrenceIsExceeded(std::shared_ptr<AbstractOption> opt) : BaseOptionError("max option occurrence is exceeded", opt) {}
};

class OptionDoesntAcceptValue : public BaseOptionError { // TODO: NOT USED IN TESTS
   public:
    OptionDoesntAcceptValue() : BaseOptionError("option doesn't accept value", nullptr) {}
};

class OnlyOneChoiseIsAllowed : public BaseOptionError { // TODO: check how this is used
                                                        // TODO: NOT USED IN TESTS
   public:
    OnlyOneChoiseIsAllowed(std::shared_ptr<OneOfPositional> opt)
        : BaseOptionError("only one choice is allowed", std::dynamic_pointer_cast<AbstractOption>(opt)) {}
};


// Options group 2 TODO: rename this

class BaseOptionWithValueError : public BaseError {
   public:
    BaseOptionWithValueError(const std::string& message, std::shared_ptr<AbstractOptionWithValue> opt) : BaseError(description(message, opt)), opt_{opt} {}
    static std::string description(const std::string& message, std::shared_ptr<AbstractOptionWithValue> opt) {
        std::stringstream str;
        str << "Option error: "<<message<<" ";
        str << displayName(opt);
        return str.str();
    }

    static std::string displayName(std::shared_ptr<AbstractOptionWithValue> opt) {
        return ""; // TODO: implement
    }

    std::shared_ptr<AbstractOptionWithValue> opt() { return opt_; }

   private:
    std::shared_ptr<AbstractOptionWithValue> opt_;
};

class TooFewValuesForOption : public BaseOptionWithValueError { // TODO: NOT USED IN TESTS
   public:
    TooFewValuesForOption(std::shared_ptr<AbstractOptionWithValue> opt) : 
    BaseOptionWithValueError("too few values for option", opt) {} 
};

class ExpectedValue : public TooFewValuesForOption {
   public:
    ExpectedValue(std::shared_ptr<AbstractOptionWithValue> opt) : TooFewValuesForOption(opt) {}
};

class ExpectedExactNumberOfValues : public BaseOptionWithValueError {
   public:
    ExpectedExactNumberOfValues(std::shared_ptr<AbstractOptionWithValue> opt) : 
    BaseOptionWithValueError("expected exact number of values", opt) {}  // too few values for options, expected at least N or exact N
};

class TooManyValuesForOption : public BaseOptionWithValueError {
   public:
    TooManyValuesForOption(std::shared_ptr<AbstractOptionWithValue> opt) : 
    BaseOptionWithValueError("too many values for option", opt) {} 
};

class UnexpectedValueForPositionalOption : public BaseError {
   public:
    UnexpectedValueForPositionalOption(const std::string& value)
        : BaseError(std::string("unexpected value for positional option: ") + value) {} 
};

// POSITIONAL OPTIONS

class TooFewPositionalOptions : public BaseError { // TODO: NOT USED IN TESTS
   public:
    TooFewPositionalOptions() : BaseError("too few positional options") {} 
};

class TooManyPositionalOptions : public BaseError {
   public:
    TooManyPositionalOptions(const std::string& str)
        : BaseError(std::string("too many positional options: ") + str) {} 
};

// Exceptions thrown while during checks after all arguments were parsed
class RequiredOptionIsNotSet : public BaseOptionError {
   public:
    RequiredOptionIsNotSet(std::shared_ptr<AbstractOption> opt)
        : BaseOptionError("required option is not set", opt) {}
};


// Exceptions used by ValueSemantics
class InvalidValueType : public BaseOptionError {
   public:
    InvalidValueType(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected)
        : BaseOptionError("invalid value type", opt) {} // TODO: use received and expected
};

class InvalidOptionValue : public BaseOptionError { // TODO: check where it is used and improve implementation
   public:
    InvalidOptionValue(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected)
        : BaseOptionError("invalid value type", opt) {}
};

class ValueIsOutOfRange : public BaseOptionError {
   public:
    ValueIsOutOfRange(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected)
        : BaseOptionError("value is out of range", opt) {}
};

class ValueMustMatchRegex : public BaseOptionError {
   public:
    ValueMustMatchRegex(std::shared_ptr<AbstractOption> opt, const std::string& regex) : BaseOptionError("value must match regex", opt){};
};

/*
TODO:
From boost::program_options
invalid_command_line_style
error_with_option_name
multiple_occurrences
multiple_values
error_with_no_option_name
unknown_option
ambiguous_option
validation_error
invalid_option_value
invalid_bool_value
*/

#endif