#ifndef __CHECKER_EXCEPTIONS_H__
#define __CHECKER_EXCEPTIONS_H__

#include <Backend/Exceptions.h>

class CheckerException : public BaseOptionError {
    public:
        CheckerException() : BaseOptionError(nullptr) {}
};

class DuplicateOptionPtrDetected: public CheckerException {

    public:
        DuplicateOptionPtrDetected() {}
};
class RecursionDetected : public CheckerException {
    public:
        RecursionDetected() {}
};
class DuplicateOptionName : public CheckerException {
    public:
        //DuplicateOption(std::shared_ptr<AbstractOption> opt) : std::logic_error("option should be specified only once") {}
        DuplicateOptionName() {}
};
#endif //__CHECKER_EXCEPTIONS_H__