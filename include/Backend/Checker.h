#ifndef __BACKEND_CHECKER_H__
#define __BACKEND_CHECKER_H__

#include "Option.h"
#include <sstream>
#include <stdexcept>

class Checker : public AbstractOptionVisitor {
public:
    std::vector<std::shared_ptr<AbstractOption>> unlocks;

    void visit(std::shared_ptr<AbstractOption> opt) override {
        throw std::runtime_error("AbstractOption should not be used directly");
    }
    void visit(std::shared_ptr<AbstractNamedOption> opt) override {
        checkCompatibility(opt);
        unlocks.push_back(opt);
        //visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractNamedCommand> opt) override {
        checkCompatibility(opt);
        unlocks.push_back(opt);
        //visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        unlocks.push_back(opt);        
        //visit(std::static_pointer_cast<AbstractOption>(opt));
    }
    void visit(std::shared_ptr<Compatibles> opt) override {
        for(auto unlock: opt->unlocks) {
            unlock->accept(*this);
        }
    }
    void visit(std::shared_ptr<Alternatives> opt) override {
        size_t cur_size = unlocks.size();
        for(auto alt : opt->alternatives) {
            alt->accept(*this);
            unlocks.erase(unlocks.begin() + cur_size, unlocks.end());
        }
    }
private:    
    void checkCompatibility(std::shared_ptr<AbstractNamedOption> opt) {
        for(auto unlock: unlocks) 
            if(checkCompatibility(opt, unlock) == false) {
                std::stringstream str;
                str<<"Duplicate option found!"; //todo: "<<opt.asstring()"
                throw std::runtime_error(str.str());
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