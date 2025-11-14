#ifndef _BACKEND_MATCHER_H
#define _BACKEND_MATCHER_H

#include "Option.h"
#include "Exceptions.h"
#include "ValueSemantics.h"
#include "ValueStorage.h"
#include "Extra.h"
#include "Parser.h"

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
        //std::optional<size_t> match_index;
        bool match;
        std::string value;
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        SingleOptionMatcher(ArgGrammarParser& args) : grammar_parser_{args} {}

        void visit(std::shared_ptr<AbstractOption> opt) override {
            assert(false);
        }
        void visit(std::shared_ptr<LiteralString> opt) override {
            unlocks.clear();
            match = false;
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
/*        void visit(std::shared_ptr<NamedCommand> opt) override {
            //match_index = std::nullopt;
            unlocks.clear();
            match = false;
            switch(grammar_parser_.current_result.token_type) {
                case ArgGrammarParser::TokenTypes::long_option:                    
                case ArgGrammarParser::TokenTypes::long_option_eq_value:                    
                case ArgGrammarParser::TokenTypes::short_option:
                case ArgGrammarParser::TokenTypes::short_option_without_value:
                case ArgGrammarParser::TokenTypes::short_option_eq_value:
                case ArgGrammarParser::TokenTypes::value:
                    if(grammar_parser_.current_result.value == opt->longName()) {
                        match = true;
                    }                    
            }            
            if(match) {
                //match_index = 0;
                unlocks = opt->unlocks;
            };
        }*/

        void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
            match = false;
            unlocks.clear();
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
        void visit(std::shared_ptr<OptionsGroup> opt) override {
            assert(false);
            match = true;
            unlocks = opt->unlocks;
        }
        void visit(std::shared_ptr<OneOf> opt) override {
            //match_index = std::nullopt;
            match = false;
            unlocks.clear();
            for(size_t n = 0; n < opt->alternatives.size(); n++) {
                auto alt = opt->alternatives[n];
                alt->accept(*this);
                if(match) {
                    //match_index = n;
                    // TODO test for nested alternatives
                    unlocks = opt->alternatives[n]->unlocks;
                    return;
                }
            }
        }
};


class Matcher {
        std::vector<std::shared_ptr<AbstractOption>> remaining_options;
        std::vector<std::shared_ptr<AbstractOption>> used_options;
        std::set<std::shared_ptr<AbstractOption>> already_joined;
    public:
        std::shared_ptr<AbstractOption> options_;
        ValuesStorage storage;

        Matcher(std::shared_ptr<AbstractOption> options) : options_{options} {}

        bool eatNextToken(ArgGrammarParser& args, SingleOptionMatcher& matcher) {
            args.getNextOption();                
            bool option_matched = false;
            for(auto it : remaining_options) {
                // TODO remove used options but fallback to them to generate error message
                bool is_positional = (std::dynamic_pointer_cast<AbstractPositionalOption>(it) != nullptr);
                it->accept(matcher);
                if(matcher.match) {
                    std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value;
                    setValue(it, matcher, unlocked_by_value);
                    jointOptionsTo(unlocked_by_value, remaining_options);
                    if(!already_joined.contains(it)) {
                        jointOptionsTo(matcher.unlocks, remaining_options); // TODO: what should we do if this option was unpacked (multiple occurrence)
                        already_joined.insert(it);
                    }
                    option_matched = true;
                    if(optionEncountered(it) == it->maxOccurrence()) {
                        std::erase(remaining_options, it);
                        used_options.push_back(it);
                    }
                    break;
                }                    
            }
            if(!option_matched) {
                // maybe this options is correct but occurred more than allowed number of times
                for(auto it : used_options) {
                    if(auto p = std::dynamic_pointer_cast<AbstractPositionalOption>(it)) {
                        continue;                            
                    }
                    it->accept(matcher);
                    if(matcher.match) {
                        checkMaxOccurrence(it); // should throw
                        assert(false);
                    }
                }
                if(args.current_result.token_type == ArgGrammarParser::value)  {
                    throw TooManyPositionalOptions(args.getRawOptionString());
                } else {
                    throw UnknownOption(args.getRawOptionString());
                }
            }
            return true;
        }
        
        bool parse(ArgGrammarParser args) {
            remaining_options.clear();
            used_options.clear();
            already_joined.clear();
            storage.clear();
            opts_counter_.clear();
            jointOptionsTo({options_}, remaining_options);
            SingleOptionMatcher matcher(args);
            while(!args.eof()) {
                eatNextToken(args, matcher);
            }
            // Apply default values
            for(auto opt : remaining_options)
            {
                if(opts_counter_.count(opt) > 0) {
                    continue;
                }
                if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                    if(p->baseValueSemantics().defaultValue() == nullptr) {
                        if(opt->required())
                            throw RequiredOptionIsNotSet(opt);
                    } else {
                        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value;
                        setDefaultValue(p, unlocked_by_value);
                        assert(unlocked_by_value.empty()); // TODO: process unlocks for default values of options
                        //jointOptionsTo(unlocked_by_value, remaining_options);
                    }
                };
            }
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
            return true;
        }
    private:
        std::map<std::shared_ptr<AbstractOption>, size_t> opts_counter_;
        size_t optionEncountered(std::shared_ptr<AbstractOption> opt) {
            size_t counter = 0;
            if(opts_counter_.count(opt) > 0) {
                counter = opts_counter_[opt];
            }
            return counter;
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
        std::string displayName(std::shared_ptr<NamedOption> opt) {
            if(opt->longName())
                return *opt->longName();
            return *opt->shortName();
        }

        void setValue(std::shared_ptr<AbstractOption> opt, const SingleOptionMatcher& matcher, std::vector<std::shared_ptr<AbstractOption>>& unlocked_by_value) { 
            checkMaxOccurrence(opt);
            if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                auto semantic_parse_result = p->baseValueSemantics().semanticParse(matcher.value);
                unlocked_by_value = semantic_parse_result->unlocks;
                storage.addValue(p, matcher.value, semantic_parse_result);
            }
            opts_counter_[opt]++;;
        }
        void setDefaultValue(std::shared_ptr<AbstractOptionWithValue> opt, std::vector<std::shared_ptr<AbstractOption>>& unlocked_by_value) { 
            auto abs_opt = std::dynamic_pointer_cast<AbstractOption>(opt);
            assert(abs_opt != nullptr);
            checkMaxOccurrence(abs_opt);
            auto semantic_parse_result = opt->baseValueSemantics().defaultValue();
            unlocked_by_value = semantic_parse_result->unlocks;
            assert(semantic_parse_result != nullptr);
            storage.addValue(opt, "", semantic_parse_result);
            storage.setDefault(opt, true);
            opts_counter_[abs_opt]++;;
        }
        void jointOptionsTo(const std::vector<std::shared_ptr<AbstractOption>>& src_options, 
            std::vector<std::shared_ptr<AbstractOption>>& dst_options) {
            for(auto it : src_options) {
                if(auto p = std::dynamic_pointer_cast<OptionsGroup>(it)) {
                    jointOptionsTo(p->unlocks, dst_options);
                } else {
                    dst_options.push_back(it);
                }                
            }
        }
};

#endif