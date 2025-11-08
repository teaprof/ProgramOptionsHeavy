#ifndef __BACKEND_EXCEPTIONS_H__
#define __BACKEND_EXCEPTIONS_H__

#include "Option.h"
#include <stdexcept>
#include <sstream>


class BasicOptionError : public std::runtime_error {
    public:
        BasicOptionError(std::shared_ptr<AbstractOption> opt) : std::runtime_error(BasicOptionError::description(opt)), opt_{opt} {
        }
        static std::string description(std::shared_ptr<AbstractOption> opt) {
            std::stringstream str;
            str<<"Option error: ";
            str<<BasicOptionError::displayName(opt);
            return str.str();
        }
        static std::string displayName(std::shared_ptr<AbstractOption> opt) {
            if(auto p = std::dynamic_pointer_cast<AbstractNamedCommand>(opt)) {
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

/*
OptionIsRequired
UnknownOption
ExpectedOption
ExpectedValue
ValueTypeIsIncorrect
ValueDomainError
OptionShouldBeSpecifiedOnlyOnce

CycleIsFound logic_error
*/

#endif