#ifndef __CHECKER_EXCEPTIONS_H__
#define __CHECKER_EXCEPTIONS_H__

#include <Backend/Exceptions.h>
#include <stdexcept>

class CheckerException : public std::logic_error {
    public:
        CheckerException(const char* str) : std::logic_error(str) {}
};

class DuplicateOptionPtrDetected: public CheckerException {

    public:
        DuplicateOptionPtrDetected() : CheckerException("Duplicate raw ptr option detected") {}
};
class RecursionDetected : public CheckerException {
    public:
        RecursionDetected() : CheckerException("Possible recursion detected") {}
};
class DuplicateOptionName : public CheckerException {
    public:
        //DuplicateOption(std::shared_ptr<AbstractOption> opt) : std::logic_error("option should be specified only once") {}
        DuplicateOptionName() : CheckerException("Duplicate option name detected") {}
};
#endif //__CHECKER_EXCEPTIONS_H__