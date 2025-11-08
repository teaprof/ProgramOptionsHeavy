#ifndef __BACKEND_EXCEPTIONS_H__
#define __BACKEND_EXCEPTIONS_H__

#include "Option.h"
#include <stdexcept>
#include <sstream>


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
            if(auto p = std::dynamic_pointer_cast<NamedCommand>(opt)) {
                return p->displayName();
            }
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

class UnknownOption : public BaseOptionError {
    public:
        UnknownOption(std::shared_ptr<AbstractOption> opt) : BaseOptionError(opt) {}
};

class RequiredOptionIsNotSet : public BaseOptionError {
     public:
        RequiredOptionIsNotSet(std::shared_ptr<AbstractOption> opt) : BaseOptionError(opt) {}
};

class ExpectedValue  : public BaseOptionError {
     public:
        ExpectedValue(std::shared_ptr<AbstractOption> opt) : BaseOptionError(opt) {}
};

class ValueTypeIsIncorrect  : public BaseOptionError {
     public:
        ValueTypeIsIncorrect(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected) : BaseOptionError(opt) {}
};


class ValueDomainError  : public BaseOptionError {
     public:
        ValueDomainError(std::shared_ptr<AbstractOption> opt, const std::string& received, const std::string& expected) : BaseOptionError(opt) {}
};


class OptionShouldBeSpecifiedOnlyOnce  : public BaseOptionError {
     public:
        OptionShouldBeSpecifiedOnlyOnce(std::shared_ptr<AbstractOption> opt) : BaseOptionError(opt) {}
};

/// TODO unused class, remove
class CycleIsFound : public std::logic_error { 
    public:
        CycleIsFound(std::shared_ptr<AbstractOption> opt) : std::logic_error("option should be specified only once") {}
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
        TooManyPositionalOptions() : std::runtime_error("too few positional options are specified") {}
};

// TODO: OnlyOneChoiseIsAllowed (for alternative)

/*
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