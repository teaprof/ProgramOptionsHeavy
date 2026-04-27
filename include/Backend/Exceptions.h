#ifndef BACKEND_EXCEPTIONS_H
#define BACKEND_EXCEPTIONS_H

#include <memory>
#include <sstream>
#include <stdexcept>

#include "Option.h"


/* 
* Base classes: 
    class BaseError : public std::runtime_error
    class BaseOptionError : public BaseError

* Exceptions that are raised during parsing command line
n-  class UnknownNamedOption : public BaseError
np  class MaxOptionOccurrenceIsExceeded : public BaseOptionError
n-  class OptionDoesntAcceptValue : public BaseOptionError // TODO: NOT USED IN TESTS
    class OnlyOneChoiseIsAllowed : public BaseOptionError // TODO: check how this is used
* Exceptions raised during checks after all arguments were parsed
np  class RequiredOptionIsNotSet : public BaseOptionError

* Exceptions raised when number of values passed for a certain option is incorrect
    class BaseOptionWithValueError : public BaseError
np  class TooFewValuesForOption : public BaseOptionWithValueError // TODO: NOT USED IN TESTS
n-  class ExpectedValue : public TooFewValuesForOption
np  class ExpectedExactNumberOfValues : public BaseOptionWithValueError
np  class TooManyValuesForOption : public BaseOptionWithValueError
-p  class UnexpectedValueForPositionalOption : public BaseError  // TODO: may be remove this exception?

* Exceptions raised when parsing positional options
-p  class TooFewPositionalOptions : public BaseError // TODO: NOT USED IN TESTS
-p  class TooManyPositionalOptions : public BaseError

Legend:
    n-: applicable only for named options
    -p: applicable only for positional options
    np: applicable both for positional and named options


* Exceptions used by ValueSemantics
    class InvalidValueType : public BaseOptionError
    class ValueIsNotAllowed : public BaseOptionError // TODO: check where it is used and improve implementation
    class ValueIsOutOfRange : public BaseOptionError 
    class ValueMustMatchRegex : public BaseOptionError 
*/


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

// Exceptions that are raised during parsing command line

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

// Exceptions raised during checks after all arguments were parsed
class RequiredOptionIsNotSet : public BaseOptionError {
   public:
    RequiredOptionIsNotSet(std::shared_ptr<AbstractOption> opt)
        : BaseOptionError("required option is not set", opt) {}
};


// Exceptions raised when number of values passed for a certain option is incorrect

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

// Exceptions raised when parsing positional options

class TooFewPositionalOptions : public BaseError { // TODO: NOT USED IN TESTS
   public:
    TooFewPositionalOptions() : BaseError("too few positional options") {} 
};

class TooManyPositionalOptions : public BaseError {
   public:
    TooManyPositionalOptions(const std::string& str)
        : BaseError(std::string("too many positional options: ") + str) {} 
};


// Exceptions used by ValueSemantics
class InvaludValue : public BaseOptionError {
    public:
        InvaludValue(const std::string& message, std::shared_ptr<AbstractOption> opt) : BaseOptionError(message, opt) {}
};

class InvalidValueType : public InvaludValue {
   public:
    InvalidValueType(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected)
        : InvaludValue("invalid value type", opt) {} // TODO: use received and expected
};

class ValueIsNotAllowed : public InvaludValue { // TODO: check where it is used and improve implementation
   public:
    ValueIsNotAllowed(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected)
        : InvaludValue("invalid value type", opt) {}
};

class ValueIsOutOfRange : public InvaludValue {
   public:
    ValueIsOutOfRange(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected)
        : InvaludValue("value is out of range", opt) {}
};

class ValueMustMatchRegex : public InvaludValue {
   public:
    ValueMustMatchRegex(std::shared_ptr<AbstractOption> opt, const std::string& regex) : InvaludValue("value must match regex", opt){};
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