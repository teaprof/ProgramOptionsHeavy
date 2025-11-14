#ifndef __BACKEND_COMPLETER_2_H__
#define __BACKEND_COMPLETER_2_H__

#include "Option.h"
#include "Matcher.h"

class CompleterVisitor : public AbstractOptionVisitor {
        std::vector<std::string>& results_;
    public:
        CompleterVisitor(std::vector<std::string>& results) : results_{results} {}
        void visit(std::shared_ptr<AbstractOption>) override {
            // nothing to do
        }
        void visit(std::shared_ptr<LiteralString> opt) override {
            results_.push_back(opt->str());
        }
        void visit(std::shared_ptr<NamedOption> opt) override {
            std::string res;
            if(opt->longName())
                res = std::string("--") + *opt->longName();
            else
                res = std::string("-") + *opt->shortName();
            results_.push_back(res);

        }
        void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
            visit(std::static_pointer_cast<NamedOption>(opt));
        }
        void visit(std::shared_ptr<AbstractPositionalOption>) override {
            /* not implemented yet */
        }
        void visit(std::shared_ptr<OptionsGroup> opt) override {
            for(auto& p : opt->unlocks) {
                p->accept(*this);
            }
        }
        void visit(std::shared_ptr<OneOf> opt) override {
            for(auto& p : opt->alternatives) {
                p->accept(*this);
            }
        }

};

class Completer : public BaseMatcher {
    public:
        Completer(std::shared_ptr<AbstractOption> opt) : BaseMatcher(opt) {}

        std::vector<std::string> getCompletionVariants(ArgGrammarParser args) {
            std::vector<std::string> results;
            std::vector<std::string> regexstr;
            try {
                BaseMatcher::parse(args);
            } catch (UnknownOption) {
                // TODO check that it is last option 
                regexstr = args.
            } catch (BaseOptionError& err) {
                return results;
            }
            CompleterVisitor visitor(results);
            for(auto &opt : this->remaining_options) {
                opt->accept(visitor);
            }
            return results;

        }
};

#endif