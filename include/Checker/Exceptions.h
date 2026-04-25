#ifndef CHECKER_EXCEPTIONS_H
#define CHECKER_EXCEPTIONS_H

#include <Backend/Exceptions.h>

#include <stdexcept>

class CheckerException : public std::logic_error {
   public:
    CheckerException(const char* str) : std::logic_error(str) {}
};

class DuplicateOptionPtrDetected : public CheckerException {
   public:
    DuplicateOptionPtrDetected() : CheckerException("Duplicate raw ptr option detected") {}
};
class RecursionDetected : public CheckerException {
   public:
    RecursionDetected() : CheckerException("Possible recursion detected") {}
};
class DuplicateOptionName : public CheckerException {
   public:
    // DuplicateOption(std::shared_ptr<AbstractOption> opt) :
    // std::logic_error("option should be specified only once")
    // {}
    DuplicateOptionName() : CheckerException("Duplicate option name detected") {}
};

class MultipleOccurenceOnlyForLastPosopt : public CheckerException {  /// TODO: may be unused
   public:
    MultipleOccurenceOnlyForLastPosopt(std::shared_ptr<AbstractPositionalOption> opt) : 
    CheckerException("123") {} //TODO: implement
};

#endif  // CHECKER_EXCEPTIONS_H