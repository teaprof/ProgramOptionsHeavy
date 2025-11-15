#ifndef __BACKEND_CHECKER_H__
#define __BACKEND_CHECKER_H__

#include "Option.h"
#include "Exceptions.h"
#include <sstream>
#include <stdexcept>
#include <set>
#include <queue>

class Checker : public AbstractOptionVisitor {
// TODO checker should check 
// - if the default value is within [min, max]
// - if the default value satisfies regex

public:    
    std::set<std::shared_ptr<AbstractOption>> encountered;

    void visit(std::shared_ptr<AbstractOption> opt) override {
        addVisited(opt);
        for(auto it : opt->unlocks)        
            remaining_options.push(it);
        next();
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
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
        if(has_positional_option_with_multiple_occurrence) {
            throw MultipleOccurenceOnlyForLastPosopt(opt);
        }
        if(opt->maxOccurrence() != 1) {
            has_positional_option_with_multiple_occurrence = true;
        }
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OptionsGroup> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OneOf> opt) override {
        visit(std::static_pointer_cast<AbstractOption>(opt));
        auto encountered_safe = encountered;        
        for(auto alt : opt->alternatives) {
            if(auto p = std::dynamic_pointer_cast<OptionsGroup>(alt)) {
                throw IncorrectAlternative(alt);
            }
            alt->accept(*this);
            encountered = encountered_safe;
        }        
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
private:    
    std::queue<std::shared_ptr<AbstractOption>> remaining_options;    
    bool has_positional_option_with_multiple_occurrence{false};

    void addVisited(std::shared_ptr<AbstractOption> opt) {
        if(encountered.contains(opt)) {   
            /// returned to option opt         
            throw DuplicateOption(opt);
        }
        encountered.insert(opt);
    }

    void checkCompatibility(std::shared_ptr<NamedOption> opt) {
        for(auto unlock: encountered) 
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
#endif