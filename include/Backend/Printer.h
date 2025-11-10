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
        void visit(std::shared_ptr<NamedOption> opt) override {
            std::cout<<prefix<<opt->longName()<<" : "<<opt->shortName()<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string("  ") + prefix;
            std::cout<<prefix<<"unlocks:"<<std::endl;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
        void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
            visit(std::static_pointer_cast<NamedOption>(opt));
        }
        void visit(std::shared_ptr<NamedCommand> opt) override {
            /// todo: implement this function instead of the following:
            visit(std::static_pointer_cast<NamedOption>(opt));
        }
        void visit(std::shared_ptr<AbstractPositionalOption>) override {
            assert(false);
        }
        void visit(std::shared_ptr<OptionsGroup> opt) override {
            std::cout<<"OptionsGroup"<<std::endl;
            auto old_prefix{prefix};
            prefix = std::string("  ") + prefix;
            std::cout<<prefix<<"unlocks:"<<std::endl;
            for(auto it : opt->unlocks) {                
                it->accept(*this);
            }
            prefix = old_prefix;
        }
        void visit(std::shared_ptr<OneOf> opt) override {
            std::cout<<"OneOf"<<std::endl;
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