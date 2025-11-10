#ifndef __BACKEND_CHECKER_H__
#define __BACKEND_CHECKER_H__

#include "Option.h"
#include "Exceptions.h"
#include <sstream>
#include <stdexcept>
#include <set>

class Checker : public AbstractOptionVisitor {
// TODO checker should check 
// - if the default value is within [min, max]
// - if the default value satisfies regex
// - only the last positional option can have multiple occurrence
// - such positional option should be placed at the end of command line (should be implemented in matcher)

public:    
    std::vector<std::shared_ptr<AbstractOption>> unlocks;

    void visit(std::shared_ptr<AbstractOption> opt) override {
        for(auto u : opt->unlocks) {
            u->accept(*this);
        }        
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        addVisited(opt);
        checkCompatibility(opt);
        unlocks.push_back(opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        visit(std::static_pointer_cast<NamedOption>(opt));
    }
    void visit(std::shared_ptr<NamedCommand> opt) override {
        addVisited(opt);
        checkCompatibility(opt);
        unlocks.push_back(opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        addVisited(opt);
        unlocks.push_back(opt);        
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OptionsGroup> opt) override {
        addVisited(opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<OneOf> opt) override {
        size_t cur_size = unlocks.size();
        auto visited_old = visited;
        for(auto alt : opt->alternatives) {
            alt->accept(*this);
            unlocks.erase(unlocks.begin() + cur_size, unlocks.end());
            visited = visited_old;
        }
    }
private:    
    std::set<std::shared_ptr<AbstractOption>> visited;

    void addVisited(std::shared_ptr<AbstractOption> opt) {
        if(visited.contains(opt)) {   
            /// returned to option opt         
            throw DuplicateOption(opt);
        }
        visited.insert(opt);
    }

    void checkCompatibility(std::shared_ptr<NamedOption> opt) {
        for(auto unlock: unlocks) 
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