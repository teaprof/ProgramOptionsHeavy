#ifndef CHECKER_UNOCHECKERS_H
#define CHECKER_UNOCHECKERS_H

#include<Backend/Option.h>
#include<Backend/ValueSemantics.h>

class DefaultValueChecker { /// TODO rename
// TODO checker should check 
// - if the default value is within [min, max]
// - if the default value satisfies regex
// - if the default value is contained in Unlocks
// - the same for implicit value
    public:
        static void checkDefaultValue(std::shared_ptr<AbstractOptionWithValue> opt) {
            if(opt->baseValueSemantics().hasDefaultValue()) {
            }
        }
};

class Unochecker : public AbstractOptionVisitor {    
    void visit(std::shared_ptr<AbstractOption> opt) {
        //nothing to do
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) {
        //nothing to do
    }
    void visit(std::shared_ptr<NamedOption> opt) {
        //nothing to do
    }
    void visit(std::shared_ptr<LiteralString> opt) {
        //nothing to do
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) {
        /// TODO: if nValuesRole == exact, valueRequired() should be true
        /// TODO: if nValuesRole == exact, nValues() should be greater than 0
        DefaultValueChecker::checkDefaultValue(opt);
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) {
        DefaultValueChecker::checkDefaultValue(opt);
    }
    void visit(std::shared_ptr<OptionsGroup2> opt) {
        //nothing to do
    }
    void visit(std::shared_ptr<OneOf> opt) {
        //nothing to do
    }
};


#endif // CHECKER_UNOCHECKERS_H