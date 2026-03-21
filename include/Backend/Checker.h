#ifndef __BACKEND_CHECKER_H__
#define __BACKEND_CHECKER_H__

#include "Option.h"
#include "Exceptions.h"
#include <sstream>
#include <stdexcept>
#include <set>
#include <queue>
//#include <generator>

class AbstractWalkee : public AbstractOptionVisitor {
    public:
        virtual void pushState() = 0;
        virtual void popState() = 0;
        virtual void restoreTopState() = 0;
};

class Combinator {
    public:
    std::vector<std::shared_ptr<AbstractOption>> current_combination_;
    std::vector<size_t> current_path_;

    Combinator(std::shared_ptr<AbstractOption> opt): mtop_{opt} {
    }
    void init(std::shared_ptr<AbstractOption> top, size_t n) {
        if(n == top->unlocks.size()) {
            return;
        }
        if(auto p = std::dynamic_pointer_cast<OneOf>(top->unlocks[n])) {
            assert(p->alternatives.size() > 0); //TODO: raise exception
            initOneOf(p, 0);
            init(top, n+1);
        } else {
            current_combination_.push_back(top->unlocks[n]);
            current_path_.push_back(0); // this value has not matter
            init(top->unlocks[n], 0);
            init(top, n + 1);
        } 
    }
    void initOneOf(std::shared_ptr<OneOf> one_of, size_t alt_number) {
        assert(alt_number < one_of->alternatives.size());
        auto top_ = one_of->alternatives[alt_number];
        current_combination_.push_back(one_of);
        current_path_.push_back(alt_number);
        init(one_of, 0);
        if(auto p = std::dynamic_pointer_cast<OneOf>(top_)) {
            initOneOf(p, 0);
        } else {
            current_combination_.push_back(top_);
            current_path_.push_back(0);
            init(top_, 0);
        }
    }
    void init() {
        init(mtop_, 0);
    }
    size_t pass(std::shared_ptr<AbstractOption> top, size_t n, size_t pos) {
        if(n == top->unlocks.size()) {
            return pos;
        }
        if(auto p = std::dynamic_pointer_cast<OneOf>(top->unlocks[n])) {
            assert(p->alternatives.size() > 0); //TODO: raise exception
            size_t old_pos{pos};
            pos = passOneOf(p, pos);
            pos = pass(top, n+1, pos);
            return pos;
        } else {
            assert(current_combination_[pos] == top->unlocks[n]);
            assert(current_path_[pos] == 0);
            return pass(top, n + 1, pos + 1);
        } 
    }

    bool increment(std::shared_ptr<AbstractOption> top, size_t n, size_t pos) {
        if(n == top->unlocks.size()) {
            return false;
        }
        assert(current_combination_[pos] == top->unlocks[n]);
        if(auto p = std::dynamic_pointer_cast<OneOf>(top->unlocks[n])) {
            assert(p->alternatives.size() > 0); //TODO: raise exception
            size_t old_pos{pos};
            pos = passOneOf(p, pos);
            if(increment(top, n+1, pos)) {
                return true;
            }
            if(incrementOneOf(p, old_pos)) {
                init(top, n+1);
                return true;
            }
            return false;
        } else {
            assert(current_combination_[pos] == top->unlocks[n]);
            assert(current_path_[pos] == 0);
            return increment(top, n + 1, pos + 1);
        } 
    }
    bool incrementOneOf(std::shared_ptr<OneOf> one_of, size_t pos) {
        /// TODO: First, the one_of's children should be processed
        assert(current_combination_[pos] == one_of);
        size_t alt_num = current_path_[pos] + 1;
        if(alt_num == one_of->alternatives.size()) {
            return false;
        };
        current_combination_.erase(current_combination_.begin() + pos, current_combination_.end());
        current_path_.erase(current_path_.begin() + pos, current_path_.end());
        /// todo: try to increment one_of->alternatives[alt_num]->unlocks
        initOneOf(one_of, alt_num);
        return true;
    }
    size_t passOneOf(std::shared_ptr<OneOf> one_of, size_t pos) {
        size_t alt_number = current_path_[pos];
        assert(alt_number < one_of->alternatives.size());
        assert(current_combination_[pos] == one_of);
        auto top = one_of->alternatives[alt_number];
        if(auto p = std::dynamic_pointer_cast<OneOf>(top)) {
            return passOneOf(p, pos+1);
        } else {
            assert(top == current_combination_[pos+1]);
            return pass(top, 0, pos+2);
        }
    }
    bool increment() {
        if(current_path_.size() == 0) {
            return false;
        }
        //return increment(current_path_.size() - 1);
        return increment(mtop_, 0, 0);
    }
    private:
    bool update_all_combs_;
    std::shared_ptr<AbstractOption> mtop_;
    std::stack<std::shared_ptr<AbstractOption>> stack_;
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