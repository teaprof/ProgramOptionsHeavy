#ifndef __BACKEND_EXCEPTIONS_H__
#define __BACKEND_EXCEPTIONS_H__

#include "Option.h"
#include <stdexcept>
#include <sstream>
#include <memory>


class BaseOptionError : public std::runtime_error {
    public:
        BaseOptionError(std::shared_ptr<AbstractOption> opt) : std::runtime_error(BaseOptionError::description(opt)), opt_{opt} {
        }
        static std::string description(std::shared_ptr<AbstractOption> opt) {
            std::stringstream str;
            str<<"Option error: ";
            str<<BaseOptionError::displayName(opt);
            return str.str();
        }
        static std::string displayName(std::shared_ptr<AbstractOption> opt) {
            /*if(auto p = std::dynamic_pointer_cast<NamedCommand>(opt)) {
                return p->displayName();
            }*/
            return "";            
        }

        std::shared_ptr<AbstractOption> opt() {
            return opt_;
        }
    private:
        std::shared_ptr<AbstractOption> opt_;
};

class ExpectedOption : public std::runtime_error {
    public:
        ExpectedOption() : std::runtime_error("Expected option") {}
};

class UnknownOption : public std::runtime_error { /// TODO: make abstract base class to avoid direct inheritance from std::runtime_error
    public:
        UnknownOption(std::string str) : std::runtime_error("Unknown option") {}
};

class RequiredOptionIsNotSet : public std::runtime_error {
     public:
        RequiredOptionIsNotSet(std::shared_ptr<AbstractOption> opt) : std::runtime_error("required option is not set") {}
};

class ExpectedValue  : public BaseOptionError {
     public:
        ExpectedValue(std::shared_ptr<AbstractOption> opt) : BaseOptionError(opt) {}
};

class InvalidValueType  : public BaseOptionError {
     public:
        InvalidValueType(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected) : BaseOptionError(opt) {}
};

class InvalidOptionValue  : public BaseOptionError {
     public:
        InvalidOptionValue(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected) : BaseOptionError(opt) {}
};

class ValueIsOutOfRange  : public BaseOptionError {
     public:
        ValueIsOutOfRange(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected) : BaseOptionError(opt) {}
};

class ValueMustMatchRegex : public BaseOptionError {
    public:
        ValueMustMatchRegex(std::shared_ptr<AbstractOption> opt, const std::string& regex) : BaseOptionError(opt) {};
};

class MaxOptionOccurenceIsExceeded  : public BaseOptionError {
     public:
        MaxOptionOccurenceIsExceeded(std::shared_ptr<AbstractOption> opt) : BaseOptionError(opt) {}
};

class DuplicateOption : public std::logic_error {
    public:
        DuplicateOption(std::shared_ptr<AbstractOption> opt) : std::logic_error("option should be specified only once") {}
};

class TooFewPositionalOptions : public std::runtime_error {
    public:
        TooFewPositionalOptions() : std::runtime_error("too few positional options are specified") {}
};

class TooManyPositionalOptions : public std::runtime_error {
    public:
        TooManyPositionalOptions(const std::string& str): std::runtime_error("too many positional options are specified: {str}") {}
};

class OnlyOneChoiseIsAllowed : public BaseOptionError {
    public:
        OnlyOneChoiseIsAllowed(std::shared_ptr<OneOf> opt) : BaseOptionError(opt) {}
};

class MultipleOccurenceOnlyForLastPosopt : public std::logic_error {
    public:
        MultipleOccurenceOnlyForLastPosopt(std::shared_ptr<AbstractPositionalOption> opt) : std::logic_error("123") {}
};
/*
TODO:
From boost::program_options
invalid_command_line_style
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