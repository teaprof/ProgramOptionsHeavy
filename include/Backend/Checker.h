#ifndef __BACKEND_CHECKER_H__
#define __BACKEND_CHECKER_H__

#include "Option.h"
#include "Exceptions.h"
#include <sstream>
#include <stdexcept>
#include <set>
#include <queue>

class AbstractWalkee : public AbstractOptionVisitor {
    public:
        virtual void pushState() = 0;
        virtual void popState() = 0;
        virtual void restoreTopState() = 0;
};

class Walker : public AbstractOptionVisitor {
public:    
    Walker(AbstractWalkee& walkee) : walkee_ {walkee} {}
    void visit(std::shared_ptr<AbstractOption> opt) override {
        opt->accept(walkee_);
        for(auto it : opt->unlocks)
            it->accept(*this);
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    /*void visit(std::shared_ptr<AbstractOptionWithValue> opt) {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }*/
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OptionsGroup2> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OneOf> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
        walkee_.pushState();
        for(auto alt : opt->alternatives) {
            alt->accept(*this);
            walkee_.restoreTopState();
        }
        walkee_.popState();
    }
private:    
    AbstractWalkee& walkee_;
};

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

class NamesCollisionChecker {
    public:
        static void checkNamesCollision(std::vector<std::shared_ptr<AbstractOption>> opts, std::shared_ptr<NamedOption> opt) {
            for(const auto& it : opts)
                if(!checkNamesCollision(opt, it)) {
                    throw DuplicateOption(opt);
                }
        }
    private:
        static bool checkNamesCollision(std::shared_ptr<NamedOption> first, std::shared_ptr<AbstractOption> second) {
            /// TODO: rename to checkNamesCollision
            auto second_named = std::dynamic_pointer_cast<NamedOption>(second);
            if(second_named) {
                if(first->longName() && second_named->longName()) {
                    if(*first->longName() == *second_named->longName())
                        return false;
                }
                if(first->shortName() && second_named->shortName()) {
                    if(*first->shortName() == *second_named->shortName())
                        return false;
                }
            }
            return true;
        }
};

class DuplicateOptionPtrChecker {
    public:
        static void checkDuplicates(std::vector<std::shared_ptr<AbstractOption>> opts, std::shared_ptr<AbstractOption> opt) {
            for(const auto& it : opts)
                if(opt == it) 
                    throw DuplicateOptionPtrDetected();
        }
};

class PositionalOptionsChecker {
    public:
        static void checkPositionalOptionCouldBeResolved(std::vector<std::shared_ptr<AbstractOption>> opts) {
            std::vector<std::shared_ptr<AbstractPositionalOption>> positional;
            for(const auto& it : opts)
                if(auto p = std::dynamic_pointer_cast<AbstractPositionalOption>(it)) {
                    positional.push_back(p);
                }
            /// TODO check that SLAE could be solved
            /*if(state_.has_positional_option_with_multiple_occurrence) {
                throw MultipleOccurenceOnlyForLastPosopt(opt);
            }
            if(opt->maxOccurrence() != 1) {
                state_.has_positional_option_with_multiple_occurrence = true;
            }
            visit(std::static_pointer_cast<AbstractOption>(opt));*/
        }
};

class CheckerWalkee : public AbstractWalkee {
public:
    void visit(std::shared_ptr<AbstractOption> opt) override {
        DuplicateOptionPtrChecker::checkDuplicates(state_.encountered, opt);
        state_.encountered.push_back(opt);
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
        /// TODO: what checks should be made for the literal string?
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
        DefaultValueChecker::checkDefaultValue(opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        NamesCollisionChecker::checkNamesCollision(state_.encountered, opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        DefaultValueChecker::checkDefaultValue(opt);
        visit(std::static_pointer_cast<NamedOption>(opt));
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
        // note: positional options are checked in topState()
    }
    void visit(std::shared_ptr<OptionsGroup2> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OneOf> opt) override {
        // TODO: implement this
    }
    void pushState() override {
        stack_.push(state_);
    }
    void popState() override {
        stack_.pop();
    }
    void restoreTopState() override {
        PositionalOptionsChecker::checkPositionalOptionCouldBeResolved(state_.encountered);
        state_ = stack_.top();
    }
private:
    struct State {
        std::vector<std::shared_ptr<AbstractOption>> encountered;
        bool has_positional_option_with_multiple_occurrence{false};
    };
    State state_;
    std::stack<State> stack_;
};

class Checker : public Walker {
// TODO checker should check 
// - if the default value is within [min, max]
// - if the default value satisfies regex
// - if the default value is contained in Unlocks
// - the same for implicit value
// - hidden names of positional options differes from other names
public:
    Checker() : Walker(walkee_) {}
private:
    CheckerWalkee walkee_;

};
#endif