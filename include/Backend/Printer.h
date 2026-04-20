#ifndef BACKEND_PRINTER_H
#define BACKEND_PRINTER_H

#include <cassert>
#include <iostream>
#include <sstream>
#include <stack>

#include "Option.h"
#include "ValueSemantics.h"

template <class T>
std::ostream& operator<<(std::ostream& out, const std::optional<T>& v) {
    if (v.has_value()) {
        out << *v;
    } else {
        out << "(empty)";
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
    template <class T>
    std::ostream& operator<<(T&& arg) {
        std::cout << prefix_ << arg;
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
        prn_ << "AbstractOption" << "\n";
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        prn_ << "AbstractPositionalOption" << "\n";
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        prn_ << "Named: " << "--" << opt->longName() << " : -" << opt->shortName() << "\n";
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
        prn_ << "Literal \"" << opt->str() << "\"\n";
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        prn_ << "NamedWithValue: " << "--" << opt->longName() << " : -" << opt->shortName() << "\n";
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
        prn_ << "AbstractPositionalOptionWithValue" << "\n";
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<OptionsGroup2> opt) override {
        prn_ << "OptionsGroup2" << "\n";
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<OneOfPositional> opt) override {
        prn_ << "OneOfPositional" << "\n";
        prn_++;
        size_t counter = 0;
        for (size_t idx = 0; idx < opt->alternativesSize(); idx++) {
            std::shared_ptr<AbstractOption> it = opt->alternative(idx);
            prn_ << "Alternative " << counter << "\n";
            prn_++;
            it->accept(*this);
            prn_--;
            counter++;
        }
        prn_--;
        printUnlocks(opt);
    }
    void visit(std::shared_ptr<OneOfNamed> opt) override {
        prn_ << "OneOfNamed" << "\n";
        prn_++;
        size_t counter = 0;
        for (size_t idx = 0; idx < opt->alternativesSize(); idx++) {
            std::shared_ptr<AbstractOption> it = opt->alternative(idx);
            prn_ << "Alternative " << counter << "\n";
            prn_++;
            it->accept(*this);
            prn_--;
            counter++;
        }
        prn_--;
        printUnlocks(opt);
    }

   private:
    TabbedPrinter prn_;
    void printUnlocks(std::shared_ptr<AbstractOption> opt) {
        if (opt->unlocksCount() == 0) {
            return;
        }
        // prn++;
        // prn<<"unlocks:"<<"\n";
        prn_++;
        for (auto it : opt->unlocks()) {
            it->accept(*this);
        }
        prn_--;
        // prn--;
    }
};

#endif