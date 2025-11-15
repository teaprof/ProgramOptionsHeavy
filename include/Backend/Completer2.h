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
        void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
            assert(false);
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

            // Cases
            // exename run -d 10 -> NO ERROR -> REMAINING_OPTIONS
            // exename run --d -> UNKNOWN_OPTION -> REMAINING OPTIONS WITH REGEX
            // exename run -d -> EXPECTED_VALUE -> TAKE PREV OPTION
            // exename run -- -> POSITIONAL ARG STARTING WITH -- -> REMAINING LONG OPTIONS
            // exename run - -> REMAINING LONG AND SHORT OPTIONS

            //Parse all arguments except the last, but the last argument can be still consumed if the previous one requires the value
            try {
                while(args.getNextIndex() + 1 < args.size())                    
                    BaseMatcher::parseNext(args);            
            } catch (BaseOptionError& err) {
                // in case of error return empty results
                return results;
            }

            //Parse the last argument
            try {
                if(!args.eof())
                    BaseMatcher::parseNext(args);
                assert(args.eof());
            } catch (const UnexpectedValueForPositionalOption& err) {
                regexstr = args.current_result.value + ".*";
            } catch (const IncorrectLiteralString& err) {
                //return {err.literal_string_->str()};
                regexstr = args.current_result.value + ".*";
            } catch (const ExpectedValue& err) {
                return {};
            } catch (const UnknownNamedOption& err) {
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
            } catch (const BaseOptionError& err) {

            } catch (const std::runtime_error& err) {

            } catch (...) {
                //std::cerr<<__LINE__<<std::endl;
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