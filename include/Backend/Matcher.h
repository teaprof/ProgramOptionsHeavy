#ifndef _BACKEND_MATCHER_H
#define _BACKEND_MATCHER_H

#include "Option.h"
#include "Exceptions.h"
#include "ValueSemantics.h"
#include "ValueStorage.h"
#include "Extra.h"
#include <Grammar/Parser.h>
#include "Checker.h"

#include <cassert>
#include <map>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <queue>
#include <set>

class SingleOptionMatcher : public AbstractOptionVisitor {
    private:
        ArgGrammarParser& grammar_parser_;
    public:
        bool match;
        std::string value;
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        SingleOptionMatcher(ArgGrammarParser& args) : grammar_parser_{args} {}
        std::vector<std::shared_ptr<AbstractOption>> checked_positional_options;

        void visit(std::shared_ptr<AbstractOption> opt) override {
            assert(false);
        }
        void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
            match = false;
            unlocks.clear();
            checked_positional_options.push_back(opt);
            switch(grammar_parser_.current_result.token_type) {
                case ArgGrammarParser::TokenTypes::long_option:
                case ArgGrammarParser::TokenTypes::long_option_eq_value:
                case ArgGrammarParser::TokenTypes::short_option:
                case ArgGrammarParser::TokenTypes::short_option_without_value:
                case ArgGrammarParser::TokenTypes::short_option_eq_value:
                    return;
                case ArgGrammarParser::TokenTypes::value:
                    /* nothing to do */;
            }
            match = true;
            value = grammar_parser_.getValue(opt);
            unlocks = opt->unlocks;
        }
        void visit(std::shared_ptr<LiteralString> opt) override {
            unlocks.clear();
            match = false;
            checked_positional_options.push_back(opt);
            switch(grammar_parser_.current_result.token_type) {
                case ArgGrammarParser::TokenTypes::long_option:                    
                case ArgGrammarParser::TokenTypes::long_option_eq_value:                    
                case ArgGrammarParser::TokenTypes::short_option:
                case ArgGrammarParser::TokenTypes::short_option_without_value:
                case ArgGrammarParser::TokenTypes::short_option_eq_value:
                break;
                case ArgGrammarParser::TokenTypes::value:
                    match = opt->str() == grammar_parser_.current_result.value;
                    break;

            }            
            if(match) {
                unlocks = opt->unlocks;
            };
        }
        void visit(std::shared_ptr<NamedOption> opt) override {
            unlocks.clear();
            match = false;
            switch(grammar_parser_.current_result.token_type) {
                case ArgGrammarParser::TokenTypes::long_option:                    
                case ArgGrammarParser::TokenTypes::long_option_eq_value:
                    match = opt->longName().has_value() && opt->longName().value() == grammar_parser_.current_result.long_option_name;
                    break;
                case ArgGrammarParser::TokenTypes::short_option:
                case ArgGrammarParser::TokenTypes::short_option_without_value:
                case ArgGrammarParser::TokenTypes::short_option_eq_value:
                    match = opt->shortName().has_value() && opt->shortName().value() == grammar_parser_.current_result.short_option_name;
                    break;
                case ArgGrammarParser::TokenTypes::value:
                    break;
            }            
            if(match) {
                unlocks = opt->unlocks;
            };
        }
        void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
            unlocks.clear();
            match = false;
            switch(grammar_parser_.current_result.token_type) {
                case ArgGrammarParser::TokenTypes::long_option:                    
                case ArgGrammarParser::TokenTypes::long_option_eq_value:
                    match = opt->longName().has_value() && opt->longName().value() == grammar_parser_.current_result.long_option_name;
                    break;
                case ArgGrammarParser::TokenTypes::short_option:
                case ArgGrammarParser::TokenTypes::short_option_without_value:
                case ArgGrammarParser::TokenTypes::short_option_eq_value:
                    match = opt->shortName().has_value() && opt->shortName().value() == grammar_parser_.current_result.short_option_name;
                    break;
                case ArgGrammarParser::TokenTypes::value:
                    /* nothing to do*/;
            }            
            if(match) {
                value = grammar_parser_.getValue(opt);
                unlocks = opt->unlocks;
            };
        }
        void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
            assert(false);
        }
        void visit(std::shared_ptr<OptionsGroup2> opt) override {
            assert(false);
            match = true;
            unlocks = opt->unlocks;
        }
        void visit(std::shared_ptr<OneOf> opt) override {
            match = false;
            unlocks.clear();
            for(size_t n = 0; n < opt->alternatives.size(); n++) {
                auto alt = opt->alternatives[n];
                alt->accept(*this);
                if(match) {
                    unlocks = opt->alternatives[n]->unlocks;
                    return;
                }
            }
        }
};

class BaseMatcher {
    protected:
        std::vector<std::shared_ptr<AbstractOption>> remaining_options;
        std::vector<std::shared_ptr<AbstractOption>> used_options;
        std::set<std::shared_ptr<AbstractOption>> already_joined;
        std::map<std::shared_ptr<AbstractOption>, size_t> opts_counter_;
        std::shared_ptr<AbstractOption> options_;
    public:
        ValuesStorage storage;

        BaseMatcher(std::shared_ptr<AbstractOption> options) : options_{options} {
            Checker checker;
            options->accept(checker);
        }

        void clear() {
            remaining_options.clear();
            used_options.clear();
            already_joined.clear();
            storage.clear();
            opts_counter_.clear();
            joinOptionsTo({options_}, remaining_options);
        }

        bool eatNextToken(ArgGrammarParser& args, SingleOptionMatcher& matcher) {
            args.getNextOption();                
            bool option_matched = false;
            bool arg_is_value = args.current_result.token_type == ArgGrammarParser::value; 
            matcher.checked_positional_options.clear();
            for(auto it : remaining_options) {
                it->accept(matcher);
                if(matcher.match) {
                    std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value;
                    setValue(it, matcher, unlocked_by_value);
                    joinOptionsTo(unlocked_by_value, remaining_options);
                    if(!already_joined.contains(it)) {
                        joinOptionsTo(matcher.unlocks, remaining_options); // TODO: what should we do if this option was unpacked (multiple occurrence)
                        already_joined.insert(it);
                    }
                    if(optionEncountered(it) == it->maxOccurrence()) {
                        std::erase(remaining_options, it);
                        used_options.push_back(it);
                    }
                    option_matched = true;
                    break;
                };
            }
            // Process parsing error
            if(!option_matched) {
                // maybe this options is correct but occurred more than allowed number of times
                for(auto it : used_options) {
                    // skip positional options
                    if(auto p = std::dynamic_pointer_cast<AbstractPositionalOption>(it)) {
                        continue;                            
                    }
                    // analyze named options - check if this option was already in use
                    it->accept(matcher);
                    if(matcher.match) {
                        checkMaxOccurrence(it); // should throw
                        assert(false);
                    }
                }
                // check for correct use of the positional options
                if(args.current_result.token_type == ArgGrammarParser::value)  {
                    // TODO it could be LiteralString
                    if(matcher.checked_positional_options.size() == 0) {
                        // no positional option have been expected
                        throw TooManyPositionalOptions(args.getRawOptionString());
                    }
                    if(matcher.checked_positional_options.size() == 1) {
                        // positional option was possible, but it doesn't match
                        const auto& opt = matcher.checked_positional_options.front();
                        if(auto p = std::dynamic_pointer_cast<LiteralString>(opt)) {                            
                            throw IncorrectLiteralString(p, args.current_result.value);
                        }                        
                        throw UnexpectedValueForPositionalOption(args.getRawOptionString());
                    }
                    // If OneOf was encountered more than one positional option can be checked
                    throw UnexpectedValueForPositionalOption(args.getRawOptionString());
                } else {
                    throw UnknownNamedOption(args.getRawOptionString());
                }
            }
            return true;
        }

        void parseNext(ArgGrammarParser& args) {
            SingleOptionMatcher matcher(args);
            eatNextToken(args, matcher);
        }

        void parse(ArgGrammarParser args) {
            clear();
            SingleOptionMatcher matcher(args);
            while(!args.eof()) {
                eatNextToken(args, matcher);
            }
        }
    protected:
        size_t optionEncountered(std::shared_ptr<AbstractOption> opt) {
            size_t counter = 0;
            if(opts_counter_.count(opt) > 0) {
                counter = opts_counter_[opt];
            }
            return counter;
        }
        void joinOptionsTo(const std::vector<std::shared_ptr<AbstractOption>>& src_options, 
            std::vector<std::shared_ptr<AbstractOption>>& dst_options) {
            for(auto it : src_options) {
                if(auto p = std::dynamic_pointer_cast<OptionsGroup2>(it)) {
                    joinOptionsTo(p->unlocks, dst_options);
                } else {
                    dst_options.push_back(it);
                }                
            }
        }
        void checkMaxOccurrence(std::shared_ptr<AbstractOption> opt)  {
            size_t counter = optionEncountered(opt);
            if(counter >= opt->maxOccurrence()) {
                if(auto p = std::dynamic_pointer_cast<OneOf>(opt)) {
                    throw OnlyOneChoiseIsAllowed(p);
                }
                throw MaxOptionOccurenceIsExceeded(opt);
            }

        }
        void setValue(std::shared_ptr<AbstractOption> opt, const SingleOptionMatcher& matcher, std::vector<std::shared_ptr<AbstractOption>>& unlocked_by_value) { 
            checkMaxOccurrence(opt);
            if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                std::any val = p->baseValueSemantics().semanticParse(matcher.value);
                unlocked_by_value = p->baseValueSemantics().getUnlocks();
                storage.addValue(p, matcher.value, val);
            }
            opts_counter_[opt]++;;
        }
};

class Matcher : public BaseMatcher {
    public:

        Matcher(std::shared_ptr<AbstractOption> options) : BaseMatcher{options} {}

        
        bool parse(ArgGrammarParser args) {
            BaseMatcher::parse(args);
            applyDefaultValues();
            checkUnusedRequiredOptions();
            return true;
        }
    private:
        void checkUnusedRequiredOptions() {
            // check that all required options are used
            for(auto p : remaining_options)
            {
                if(p->required() && opts_counter_.count(p) == 0) {
                    if(auto q = std::dynamic_pointer_cast<AbstractPositionalOption>(p)) {
                        throw TooFewPositionalOptions(); /// TODO how many pos options are expected
                    } else {
                        throw RequiredOptionIsNotSet(q);
                    }
                }
            }
        }

        void applyDefaultValues() {
            for(auto opt : remaining_options)
            {
                if(opts_counter_.count(opt) > 0) {
                    continue;
                }
                if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                    if(!p->baseValueSemantics().hasDefaultValue()) {
                        if(opt->required())
                            throw RequiredOptionIsNotSet(opt);
                    } else {
                        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value;
                        setDefaultValue(p, unlocked_by_value);
                        assert(unlocked_by_value.empty()); // TODO: process unlocks for default values of options
                        //joinOptionsTo(unlocked_by_value, remaining_options);
                    }
                };
            }
        }

        void setDefaultValue(std::shared_ptr<AbstractOptionWithValue> opt, std::vector<std::shared_ptr<AbstractOption>>& unlocked_by_value) { 
            auto abs_opt = std::dynamic_pointer_cast<AbstractOption>(opt);
            checkMaxOccurrence(abs_opt);
            std::any v = opt->baseValueSemantics().setToDefault();
            unlocked_by_value = opt->baseValueSemantics().getUnlocks();
            storage.addValue(opt, "", v); /// TODO here should be default value
            //storage.setDefault(opt, true);
            opts_counter_[abs_opt]++;;
        }
};

#endif