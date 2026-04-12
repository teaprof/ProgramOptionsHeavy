#ifndef CHECKER_PAIRWISECHECKERS_H
#define CHECKER_PAIRWISECHECKERS_H

#include <Backend/Option.h>
#include <Checker/Exceptions.h>

class NamesCollisionChecker {
    public:
        static bool checkNamesCollision(std::shared_ptr<AbstractOption> first, std::shared_ptr<AbstractOption> second) {
            /// TODO: rename to checkNamesCollision
            auto first_named = std::dynamic_pointer_cast<NamedOption>(first);
            auto second_named = std::dynamic_pointer_cast<NamedOption>(second);
            if(first_named && second_named) {
                if(first_named->longName() && second_named->longName()) {
                    if(*first_named->longName() == *second_named->longName()) {
                        return false;
}
                }
                if(first_named->shortName() && second_named->shortName()) {
                    if(*first_named->shortName() == *second_named->shortName()) {
                        return false;
}
                }
            }
            return true;
        }
};

class DuplicateOptionPtrChecker {
    public:
        static bool checkDuplicates(std::shared_ptr<AbstractOption> opt1, std::shared_ptr<AbstractOption> opt2) {
            return opt1 != opt2;
        }
};

class PairwiseChecker {
    public:
        static void check(std::shared_ptr<AbstractOption> opt1, std::shared_ptr<AbstractOption> opt2) {
            if(!DuplicateOptionPtrChecker::checkDuplicates(opt1, opt2)) {
                throw DuplicateOptionPtrDetected(); // not tested
            }
            if(!NamesCollisionChecker::checkNamesCollision(opt1, opt2)) {
                throw DuplicateOptionName();
            }
        }
};

#endif // CHECKER_PAIRWISECHECKERS_H