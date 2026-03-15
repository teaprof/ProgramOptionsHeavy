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

class WalkerA {
    public:
        WalkerA() {

        }                 
        class Combinator {
            public:
            struct Leaf {
                Leaf(std::shared_ptr<AbstractOption> v) : val{v} {}
                std::shared_ptr<AbstractOption> val;
                std::vector<std::shared_ptr<Leaf>> children;
            };

            Combinator(std::shared_ptr<AbstractOption> opt): top_{opt}, cur_{top_} {
                processNextUnlock(0);
            }
            std::shared_ptr<Leaf> processNextUnlock(std::shared_ptr<Leaf> top, size_t n) {
                std::shared_ptr<Leaf> res{nullptr};
                if(n == top->val->unlocks.size()) {
                    return;
                }
                if(auto p = std::dynamic_pointer_cast<OneOf>(top->val)) {
                    for(auto& q : p->alternatives) {
                        auto child = std::make_shared<Leaf>(q);
                        processNextUnlock(child, 0);                        
                    }
                } else {
                    //current_sequence_.push_back(opt_->unlocks[n]);
                    auto child = std::make_shared<Leaf>(opt_->unlocks[n]);
                    parent->children.push_back(child);
                    processNextUnlock(child, n++);
                    //current_sequence_.pop_back();
                } 
            }
            std::vector<std::vector<std::shared_ptr<AbstractOption>>>& allCombination() {
                return all_combinations_;
            }
            private:
            bool update_all_combs_;
            std::shared_ptr<Leaf> top_;
            std::shared_ptr<Leaf> cur_;
        };

        class Iterator {
            public:
            Iterator(std::shared_ptr<AbstractOption> opt): opt_(opt), combinator_(opt) {}
            void operator++() {
                cur_idx++;                
            }
            Iterator& begin() {
                std::vector<std::shared_ptr<AbstractOption>> current_sequence_;
                Combinator comb(opt_, current_sequence_);
                comb.fill();
                all_combinations_ = std::move(comb.all_combinations);
                cur_idx = 0;                
                return *this;                
            }
            Iterator end() {
                return Iterator(nullptr, current_sequence_);
            }
            bool operator!=(const Iterator& other) {
                assert(other.opt_ == nullptr); // this implementation works only in thic case
                return opt_ == other.opt_;
            }
            std::vector<std::shared_ptr<AbstractOption>>& operator*() {
                return all[cur_idx];
            }
            private:
            Combinator combinator_;
            size_t cur_idx{0};
            std::shared_ptr<AbstractOption> opt_;            
        };
        
        Iterator begin() {

        }
        Iterator end() {

        }
        
}

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