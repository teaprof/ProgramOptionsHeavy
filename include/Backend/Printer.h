#ifndef __BACKEND_PRINTER_H__
#define __BACKEND_PRINTER_H__

#include "Option.h"

#include <cassert>
#include <iostream>
#include <sstream>

template<class T>
std::ostream& operator<<(std::ostream& out, const std::optional<T>& v) {
    if(v.has_value()) {
        out<<*v;
    } else {
        out<<"(empty)";
    }
    return out;
}

class Printer : public AbstractOptionVisitor {
    public:
        std::string prefix;

        void visit(std::shared_ptr<AbstractOption> opt) override {
            std::cout<<"AbstractOption"<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string("  ") + prefix;
            std::cout<<prefix<<"unlocks:"<<std::endl;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
        void visit(std::shared_ptr<AbstractNamedOption> opt) override {
            std::cout<<prefix<<opt->longName()<<" : "<<opt->shortName()<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string("  ") + prefix;
            std::cout<<prefix<<"unlocks:"<<std::endl;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
        void visit(std::shared_ptr<AbstractNamedCommand> opt) override {
            /// todo: implement this function instead of the following:
            visit(std::static_pointer_cast<AbstractNamedOption>(opt));
        }
        void visit(std::shared_ptr<AbstractPositionalOption>) override {
            assert(false);
        }
        void visit(std::shared_ptr<Compatibles> opt) override {
            std::cout<<"Compatibles"<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string("  ") + prefix;
            std::cout<<prefix<<"unlocks:"<<std::endl;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
        void visit(std::shared_ptr<Alternatives> opt) override {
            std::cout<<"Alternatives"<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string("  ") + prefix;
            for(auto it : opt->alternatives) {
                it->accept(*this);
            }
            std::cout<<prefix<<"unlocks:"<<std::endl;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
};

#endif