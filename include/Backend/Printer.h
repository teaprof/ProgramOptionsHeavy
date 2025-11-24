#ifndef __BACKEND_PRINTER_H__
#define __BACKEND_PRINTER_H__

#include "ValueSemantics.h"
#include "Option.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stack>

template<class T>
std::ostream& operator<<(std::ostream& out, const std::optional<T>& v) {
    if(v.has_value()) {
        out<<*v;
    } else {
        out<<"(empty)";
    }
    return out;
}

class TabbedPrinter {
    public:
        void operator++(int dummy) {
            prefix_stack_.push(prefix_);
            prefix_ = std::string("  ") + prefix_;
        }
        void operator--(int dummy) {
            prefix_ = prefix_stack_.top();
            prefix_stack_.pop();
        }
        template<class T>
        std::ostream& operator<<(T&& arg) {
            std::cout<<prefix_<<arg;
            return std::cout;
        }
    private:
        std::stack<std::string> prefix_stack_;
        std::string prefix_;
};

class Printer : public AbstractOptionVisitor {
    public:
        std::string prefix;

        void visit(std::shared_ptr<AbstractOption> opt) override {
            prn<<"AbstractOption"<<"\n";
            printUnlocks(opt);
        }
        void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
            prn<<"AbstractPositionalOption"<<"\n";
            printUnlocks(opt);
        }
        void visit(std::shared_ptr<NamedOption> opt) override {
            prn<<"Named: "<<"--"<<opt->longName()<<" : -"<<opt->shortName()<<"\n";
            printUnlocks(opt);
        }
        void visit(std::shared_ptr<LiteralString> opt) override {
            prn<<"Literal \""<<opt->str()<<"\"\n";
            printUnlocks(opt);
        }
        void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
            prn<<"NamedWithValue: "<<"--"<<opt->longName()<<" : -"<<opt->shortName()<<"\n";
            printUnlocks(opt);
        }
        void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
            prn<<"AbstractPositionalOptionWithValue"<<"\n";
            printUnlocks(opt);
        }
        void visit(std::shared_ptr<OptionsGroup2> opt) override {
            prn<<"OptionsGroup2"<<"\n";
            printUnlocks(opt);
        }
        void visit(std::shared_ptr<OneOf> opt) override {
            prn<<"OneOf"<<"\n";
            prn++;
            size_t counter = 0;
            for(auto it : opt->alternatives) {
                prn<<"Alternative "<<counter<<"\n";
                prn++;
                it->accept(*this);
                prn--;
                counter++;
            }
            prn--;
            printUnlocks(opt);
        }
    private:
        TabbedPrinter prn;
        void printUnlocks(std::shared_ptr<AbstractOption> opt) {
            if(opt->unlocks.empty())
                return;
            //prn++;
            //prn<<"unlocks:"<<"\n";
            prn++;
            for(auto it : opt->unlocks) {
                it->accept(*this);
            }
            prn--;
            //prn--;
        }
};

#endif