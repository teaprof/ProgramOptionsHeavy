#ifndef __BACKEND_COMPLETER_2_H__
#define __BACKEND_COMPLETER_2_H__

#include "Option.h"
#include "Matcher.h"

#include <regex>
#include <iostream>

class CompleterVisitor : public AbstractOptionVisitor {
        std::vector<std::string>& results_;
        std::optional<std::regex> regex_;
    public:
        CompleterVisitor(std::vector<std::string>& results, const std::string& regexstr = "") : results_{results} {
            if(regexstr.size() > 0)
                regex_ = std::regex(regexstr);
        }
        void visit(std::shared_ptr<AbstractOption>) override {
            // nothing to do
        }
        void visit(std::shared_ptr<LiteralString> opt) override {
            if(regex_.has_value()) {
                if(!std::regex_match(opt->str(), regex_.value()))
                    return;
            }
            results_.push_back(opt->str());
        }
        void visit(std::shared_ptr<NamedOption> opt) override {
            if(regex_.has_value()) {
                if(opt->longName()) {
                    std::string long_name = std::string("--") + *opt->longName();
                    if(std::regex_match(long_name, regex_.value()))
                        results_.push_back(long_name);
                    return;
                }
                if(opt->shortName()) {
                    std::string short_name = std::string("-") + *opt->shortName();
                    if(std::regex_match(short_name, regex_.value())) 
                        results_.push_back(short_name);
                }
                return;
            }

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
            std::string regexstr;

            clear();

            //Parse all arguments except the last
            try {
                while(args.getNextIndex() + 1 < args.size())
                    BaseMatcher::parseNext(args);
            } catch (BaseOptionError& err) {
                return results;
            }

            //Parse the last argument
            try {
                if(!args.eof())
                    BaseMatcher::parseNext(args);
                assert(args.eof());
            } catch (ExpectedValue& err) {
                return {};
            } catch (BaseOptionError& err) {
                switch(args.current_result.token_type)  {
                    case ArgGrammarParser::short_option:
                    case ArgGrammarParser::short_option_eq_value:
                    case ArgGrammarParser::short_option_without_value:
                    case ArgGrammarParser::long_option_eq_value:
                        // option is completed but unknown
                        return {};
                    case ArgGrammarParser::long_option:
                        regexstr = std::string("--") + args.current_result.long_option_name + ".*";
                        break;
                    case ArgGrammarParser::value:
                        regexstr = args.current_result.value + ".*";

                }
            } catch (...) {
                assert(false);
            }

            CompleterVisitor visitor(results, regexstr);
            for(auto &opt : this->remaining_options) {
                opt->accept(visitor);
            }
            return results;

        }
};

#endif