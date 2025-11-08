#ifndef __BACKEND_CHECKER_H__
#define __BACKEND_CHECKER_H__

#include "Option.h"
#include "Exceptions.h"
#include <sstream>
#include <stdexcept>
#include <set>

class Checker : public AbstractOptionVisitor {
public:    
    std::vector<std::shared_ptr<AbstractOption>> unlocks;

    void visit(std::shared_ptr<AbstractOption> opt) override {
        for(auto u : opt->unlocks) {
            u->accept(*this);
        }        
    }
    void visit(std::shared_ptr<AbstractNamedOption> opt) override {
        addVisited(opt);
        checkCompatibility(opt);
        unlocks.push_back(opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractNamedCommand> opt) override {
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
    void visit(std::shared_ptr<Compatibles> opt) override {
        addVisited(opt);
        visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<Alternatives> opt) override {
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

    void checkCompatibility(std::shared_ptr<AbstractNamedOption> opt) {
        for(auto unlock: unlocks) 
            if(checkCompatibility(opt, unlock) == false) {
                std::stringstream str;
                throw DuplicateOption(opt);
            }
    }
    bool checkCompatibility(std::shared_ptr<AbstractNamedOption> first, std::shared_ptr<AbstractOption> second) {
        auto second_named = std::dynamic_pointer_cast<AbstractNamedOption>(second);
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