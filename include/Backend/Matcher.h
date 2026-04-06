#ifndef _BACKEND_MATCHER_H
#define _BACKEND_MATCHER_H

#include "Option.h"
#include "Exceptions.h"
#include "ValueSemantics.h"
#include "ValueStorage.h"
#include <Grammar/Parser.h>
#include <Checker/Checker.h>

#include <cassert>
#include <map>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <queue>
#include <set>

///TODO: ambiguous behavior: what should we do in the following cases:
/// program -d=1 2 filename
//  there are two possible cases: 
// 1. d = [1, 2],   pos = filename
// 2. d = [1],      pos = [2, filename]
// Some solutions:
// 1. Eat as many values as possible. If value can't be converted to Int the value list is finished
// 2. Use `--` (double dash) to indicate the end of the value list


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
            // should not never visit abstract object
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
                case ArgGrammarParser::TokenTypes::double_dash:
                    return;
                case ArgGrammarParser::TokenTypes::value:
                    /* nothing to do */;
            }
            match = true;
            value = grammar_parser_.getValue(opt); // todo: try to read as many values as possible
            unlocks = opt->unlocks(); // todo: avoid copying of the vector
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
                case ArgGrammarParser::TokenTypes::double_dash:
                break;
                case ArgGrammarParser::TokenTypes::value:
                    match = opt->str() == grammar_parser_.current_result.value;
                    break;

            }            
            if(match) {
                unlocks = opt->unlocks(); // todo: avoid copying of a vector
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
                case ArgGrammarParser::TokenTypes::double_dash:
                case ArgGrammarParser::TokenTypes::value:
                    break;
            }            
            if(match) {
                unlocks = opt->unlocks(); // todo: avoid copying of a vector
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
                case ArgGrammarParser::TokenTypes::double_dash:
                case ArgGrammarParser::TokenTypes::value:
                    /* nothing to do*/;
            }            
            if(match) {
                value = grammar_parser_.getValue(opt); // todo: try to read as many values as possible
                unlocks = opt->unlocks(); // todo: avoid copying of a vector
            };
        }
        void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
            // should not never visit abstract object
            assert(false);
        }
        void visit(std::shared_ptr<OptionsGroup2> opt) override {
            assert(false);
            match = true;
            unlocks = opt->unlocks(); // todo: avoid copying of a vector
        }
        void visit(std::shared_ptr<OneOf> opt) override {
            match = false;
            unlocks.clear();
            for(size_t n = 0; n < opt->alternatives.size(); n++) {
                auto alt = opt->alternatives[n];
                alt->accept(*this);
                if(match) {
                    unlocks = opt->alternatives[n]->unlocks(); // todo: avoid copying of a vector
                    return;
                }
            }
        }

        void setPositionalOnlyFlag(bool value) {
            grammar_parser_.match_only_positional = value;
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
        KeyValueStorage storage;

        BaseMatcher(std::shared_ptr<AbstractOption> options) : options_{options} {
            //Checker checker;
            //options->accept(checker);
        }

        void clear() {
            remaining_options.clear();
            used_options.clear();
            already_joined.clear();
            storage.clear();
            opts_counter_.clear();
            joinOptionsTo({options_}, remaining_options);
        }

        bool eatNextValueIfCan(ArgGrammarParser& args, SingleOptionMatcher& matcher, std::shared_ptr<AbstractOptionWithValue> opt) {
            //check if opt can accept one more value
            bool can_accept = false;
            bool should_accept = false;
            switch(opt->nValuesRole()) {
                case AbstractNamedOptionWithValue::NValuesRole::exact: {                    
                    size_t actual_count = storage[opt].lastOccurrenceSize();
                    size_t required_count = opt->nValues();
                    can_accept = (actual_count < required_count);
                    should_accept = can_accept;
                    break;
                }
                case AbstractNamedOptionWithValue::NValuesRole::upto: {
                    size_t actual_count = storage[opt].lastOccurrenceSize();
                    size_t max_count = opt->nValues();
                    can_accept = (actual_count < max_count);
                    should_accept = false;
                    break;
                }
                case AbstractNamedOptionWithValue::NValuesRole::infinite: {
                    can_accept = true;
                    should_accept = false;
                    break;
                }
            }
            if(can_accept == false) {
                return false;
            }
            args.getNextOption();
            bool arg_is_value = args.current_result.token_type == ArgGrammarParser::value;
            bool arg_is_double_dash = args.current_result.token_type == ArgGrammarParser::double_dash;
            if(!arg_is_value) {
                if(!arg_is_double_dash)                
                    args.ungetOption();
                if(should_accept) {
                    throw TooFewValuesForOption(); // todo: print how many options should be (expected N or at least N)
                }
                return false;
            }
            std::vector<std::shared_ptr<AbstractOption>> unlocked_by_values;        
            addValueToCurrentOccurrence(opt, args.current_result.value, unlocked_by_values);
            // todo: unused unlocked_by_values
            return true;
        }

        std::shared_ptr<AbstractOption> eatNextToken(ArgGrammarParser& args, SingleOptionMatcher& matcher) {
            args.getNextOption();
            if(args.current_result.token_type == ArgGrammarParser::TokenTypes::double_dash) {
                matcher.setPositionalOnlyFlag(true);
                return nullptr;
            }
            bool option_matched = false;            
            matcher.checked_positional_options.clear();
            std::shared_ptr<AbstractOption> res = nullptr;
            for(auto it : remaining_options) {
                it->accept(matcher);
                if(matcher.match) {
                    std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value;
                    addValueToNewOccurrence(it, matcher, unlocked_by_value);
                    joinOptionsTo(unlocked_by_value, remaining_options);
                    if(!already_joined.contains(it)) {
                        joinOptionsTo(matcher.unlocks, remaining_options); // TODO: what should we do if this option was unpacked (multiple occurrence)
                        already_joined.insert(it);
                    }
                    if(optionEncountered(it) == it->maxOccurrence()) {
                        // if the number of occurences of this option is exausted
                        // then remove this option from the list of remaining options
                        std::erase(remaining_options, it);
                        used_options.push_back(it);
                    }
                    option_matched = true;
                    res = it;
                    break;
                };
            }
            // Process parsing error
            if(!option_matched) {
                /// TODO: assert(arg_is_double_dash == false)
                /*bool arg_is_double_dash = args.current_result.token_type == ArgGrammarParser::double_dash; 
                if(arg_is_double_dash) {
                    // double dash should be simply ignored
                    return nullptr;
                }*/
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
                        throw TooManyPositionalOptions(args.getRawOptionName());
                    }
                    if(matcher.checked_positional_options.size() == 1) {
                        // positional option was possible, but it doesn't match
                        const auto& opt = matcher.checked_positional_options.front();
                        if(auto p = std::dynamic_pointer_cast<LiteralString>(opt)) {                            
                            throw IncorrectLiteralString(p, args.current_result.value);
                        }                        
                        throw UnexpectedValueForPositionalOption(args.getRawOptionName());
                    }
                    // If OneOf was encountered more than one positional option can be checked
                    throw UnexpectedValueForPositionalOption(args.getRawOptionName());
                } else {
                    throw UnknownNamedOption(args.getRawOptionName());
                }
            }
            return res;
        }

        void parseNext(ArgGrammarParser& args) {
            SingleOptionMatcher matcher(args);
            auto opt = eatNextToken(args, matcher);
            if(opt) {
                if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                    while(!args.eof() && eatNextValueIfCan(args, matcher, p)) {};
                    if(args.eof()) {
                        checkIfOptionIsCompleted(p);
                    }
                }
            };
        }

        void parse(ArgGrammarParser args) { // TODO:  rename (Parser is another class)
            clear();
            SingleOptionMatcher matcher(args);
            while(!args.eof()) {
                //eatNextToken(args, matcher);
                parseNext(args);
            }
        }
    protected:
        /// how many times the specified option has been encountered in the already parsed context
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
                    joinOptionsTo(p->unlocks(), dst_options);  // todo: avoid copying of a vector
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
        void addValueToNewOccurrence(std::shared_ptr<AbstractOption> opt, const SingleOptionMatcher& matcher, std::vector<std::shared_ptr<AbstractOption>>& unlocked_by_value) { 
            checkMaxOccurrence(opt);
            if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                std::any val = p->baseValueSemantics().semanticParse(matcher.value);
                unlocked_by_value = p->baseValueSemantics().getUnlocks();
                // todo: unused unlocked_by_value
                storage.addValue(p, matcher.value, val);
            }
            opts_counter_[opt]++;
        }
        void addValueToCurrentOccurrence(std::shared_ptr<AbstractOptionWithValue> opt, const std::string& value, std::vector<std::shared_ptr<AbstractOption>>& unlocked_by_value) {
            std::any val = opt->baseValueSemantics().semanticParse(value);
            storage.addValueToCurrentOccurence(opt, value, val);
        }
        void checkIfOptionIsCompleted(std::shared_ptr<AbstractOptionWithValue> opt) { // TODO: rename to something like checkIfValueListIsCompleted
            if(opt->nValuesRole() == AbstractOptionWithValue::NValuesRole::exact) {
                assert(storage.contains(opt));
                size_t actual = storage[opt].lastOccurrenceSize();
                size_t expected = opt->nValues();
                if(actual < expected) {
                    throw TooFewValuesForOption(); // todo print message
                }
            }
            if(opt->valueRequired()) {
                size_t actual = storage[opt].lastOccurrenceSize();
                if(actual == 0) {
                    throw ExpectedValue(nullptr);
                }

            }
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
                        throw TooFewPositionalOptions(); /// TODO print how many pos options are expected
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