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
        virtual void topState() = 0;
};

class Walker : public AbstractOptionVisitor {
public:    
    Walker(AbstractWalkee& walkee) : walkee_ {walkee} {}
    void visit(std::shared_ptr<AbstractOption> opt) override {
        process(opt);
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
        process(opt);
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
        process(opt);
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        process(opt);
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        process(opt);
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        process(opt);
    }
    void visit(std::shared_ptr<OptionsGroup2> opt) override {
        process(opt);
    }
    void visit(std::shared_ptr<OneOf> opt) override {
        process(opt);
        walkee_.pushState();
        for(auto alt : opt->alternatives) {
            alt->accept(*this);
            walkee_.topState();
        }
        walkee_.popState();
    }
private:    
    std::queue<std::shared_ptr<AbstractOption>> remaining_options;
    AbstractWalkee& walkee_;

    void process(std::shared_ptr<AbstractOption> opt) {
        opt->accept(walkee_);
        for(auto it : opt->unlocks)        
            remaining_options.push(it);
        next();
    }
    void next() {
        std::shared_ptr<AbstractOption> head = nullptr;
        while(head == nullptr && !remaining_options.empty()) {
            head = remaining_options.front();
            remaining_options.pop();
        }
        if(head != nullptr)
            head->accept(*this);
    }
};

class CheckerWalkee : public AbstractWalkee {
// TODO checker should check 
// - if the default value is within [min, max]
// - if the default value satisfies regex
// - if the default value is contained in Unlocks
// - the same for implicit value
public:    

    void visit(std::shared_ptr<AbstractOption> opt) override {
        addVisited(opt);
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
        if(state_.has_positional_option_with_multiple_occurrence) {
            throw MultipleOccurenceOnlyForLastPosopt(opt);
        }
        if(opt->maxOccurrence() != 1) {
            state_.has_positional_option_with_multiple_occurrence = true;
        }
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        checkCompatibility(opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        visit(std::static_pointer_cast<NamedOption>(opt));
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        assert(false);
    }
    void visit(std::shared_ptr<OptionsGroup2> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OneOf> opt) override {
    }
    void pushState() override {
        stack_.push(state_);
    }
    void popState() override {
        stack_.pop();
    }
    void topState() override {
        state_ = stack_.top();
    }
private:
    struct State {
        std::set<std::shared_ptr<AbstractOption>> encountered;
        bool has_positional_option_with_multiple_occurrence{false};
    };
    State state_;
    std::stack<State> stack_;

    void addVisited(std::shared_ptr<AbstractOption> opt) {
        if(state_.encountered.contains(opt)) {   
            /// returned to option opt         
            throw DuplicateOption(opt);
        }
        state_.encountered.insert(opt);
    }

    void checkCompatibility(std::shared_ptr<NamedOption> opt) {
        for(auto unlock: state_.encountered) 
            if(checkCompatibility(opt, unlock) == false) {
                std::stringstream str;
                throw DuplicateOption(opt);
            }
    }
    bool checkCompatibility(std::shared_ptr<NamedOption> first, std::shared_ptr<AbstractOption> second) {
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

class Checker : public Walker {
// TODO checker should check 
// - if the default value is within [min, max]
// - if the default value satisfies regex
// - if the default value is contained in Unlocks
// - the same for implicit value
public:
    Checker() : Walker(walkee_) {}
private:
    CheckerWalkee walkee_;

};
#endif