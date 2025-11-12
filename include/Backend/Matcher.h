#ifndef _BACKEND_MATCHER_H
#define _BACKEND_MATCHER_H

#include "Option.h"
#include "Exceptions.h"
#include "ValueSemantics.h"
#include "ValueStorage.h"
#include "Extra.h"

#include <cassert>
#include <map>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <queue>
#include <set>
#include <boost/spirit/include/classic.hpp>


/* Todo:
1. support for escape character: "--a" is named option, but "\--a" is value converted to "--a"
2. support for "--": all args after it are treated as positional 
*/

class ArgLexer {
        /*
        Possible tokens:
        --long_name
        --long_name=value        
        -xyz
        -xyz=value  
        value   
        -- // unimplemented yet
        */
       /// todo: add "--" support (which signalizes that after it all arguments are treated as positional options)
    private:
        static constexpr char long_option_pattern[] = "^--([A-Za-z0-9_\\-]+)"; // like --long_name
        static constexpr char long_option_eq_value_pattern[] = "^--([A-Za-z0-9_\\-]+)=(.*)"; // like --long_name=value
        static constexpr char short_options_pattern[] = "^-([A-Za-z0-9]+)"; // like -xyz
        static constexpr char short_options_eq_value_pattern[] = "^-([A-Za-z0-9]+)=(.*)"; //like -xyz=value
    public:
        enum TokenType {
            long_option, // --long_option
            long_option_eq_value, // --long_option=value
            short_options, // -xyz
            short_options_eq_value, // -xyz=value
            value // value (for example, positional option)
        };
        struct Result {
            TokenType type;
            std::string long_option_name; // without leading "--"
            std::string short_option_names; // without leading "-"
            std::string value;
        };

        static Result lex(const std::string& arg) {
            std::string current_arg = arg;
            std::smatch match;
            std::regex long_option_ex(long_option_pattern);
            std::regex short_options_ex(short_options_pattern);
            std::regex long_option_eq_value_ex(long_option_eq_value_pattern);
            std::regex short_options_eq_value_ex(short_options_eq_value_pattern);
            Result res;
            if(std::regex_match(arg, match, long_option_ex)) {                
                assert(match.size() == 2);
                res.long_option_name = match[1];
                res.type = long_option;
            } else 
            if(std::regex_match(arg, match, long_option_eq_value_ex)) {
                assert(match.size() == 3);
                res.long_option_name = match[1];
                res.value = undecorateValue(match[2]);
                res.type = long_option_eq_value;
            } else 
            if(std::regex_match(arg, match, short_options_ex)) {
                assert(match.size() == 2);
                res.short_option_names = match[1];
                res.type = short_options;
            } else 
            if(std::regex_match(arg, match, short_options_eq_value_ex)) {
                assert(match.size() == 3);
                res.short_option_names = match[1];
                res.value = undecorateValue(match[2]);
                res.type = short_options_eq_value;
            } else {
                res.value = undecorateValue(arg);
                res.type = value;
            }
            return res;
        } 
        static std::string undecorateValue(const std::string& str) {
            if(str.length() == 0 || str[0] != '\\')
                return str;
            return str.substr(1, std::string::npos);
        }
};


class ArgGrammarParser {
        std::vector<std::string> args_;
        size_t idx{0};
        size_t subidx{0};
    public:
        enum TokenTypes {
            long_option, 
            long_option_eq_value,
            short_option,
            short_option_without_value,
            short_option_eq_value,
            value
        };        
        struct Result {
            TokenTypes token_type;
            std::string long_option_name; 
            std::string short_option_name;
            std::string value;
        };
        Result current_result;        
        std::queue<Result> results;     


        template<class ... Type>
        ArgGrammarParser(Type ... arguments) : args_{arguments...} {
        }

        ArgGrammarParser(const char* str) {
            // split str into separate words
            std::stringstream ss(str);
            std::string word;
            while (ss >> word) {
                args_.push_back(word);
            }
        }

        ArgGrammarParser(const std::string& str) {
            // split str into separate words
            std::stringstream ss(str);
            std::string word;
            while (ss >> word) {
                args_.push_back(word);
            }
        }

        bool eof() {
            return idx == args_.size() && results.empty();
        }

        Result getNextOption() {
            assert(eof() == false);
            if(results.empty()) {
                assert(idx < args_.size());
                auto lex_result = ArgLexer::lex(args_[idx++]);
                switch(lex_result.type) {
                    case ArgLexer::long_option:
                        results.push(Result{TokenTypes::long_option, lex_result.long_option_name, "", ""});
                        break;
                    case ArgLexer::long_option_eq_value:
                        results.push(Result{TokenTypes::long_option_eq_value, lex_result.long_option_name, "", lex_result.value});
                        break;
                    case ArgLexer::short_options: {
                        for(size_t n = 0; n + 1 < lex_result.short_option_names.size(); n++) {
                            std::string str{lex_result.short_option_names[n]};
                            results.push(Result{TokenTypes::short_option_without_value, "", str, ""});
                        }
                        std::string str{lex_result.short_option_names.back()};
                        results.push(Result{TokenTypes::short_option, "", str, ""});
                        break;
                        }
                    case ArgLexer::short_options_eq_value: {
                        for(size_t n = 0; n + 1 < lex_result.short_option_names.size(); n++) {
                            std::string str{lex_result.short_option_names[n]};
                            results.push(Result{TokenTypes::short_option_without_value, "", str, ""});
                        }
                        std::string str{lex_result.short_option_names.back()};
                        results.push(Result{TokenTypes::short_option_eq_value, "", str, lex_result.value});
                        break;
                        }
                    case ArgLexer::value:
                        results.push(Result{TokenTypes::value, "", "", lex_result.value});
                }
            }
            current_result = std::move(results.front());
            results.pop();
            return current_result;
        }

        std::string getRawOptionString() {
            switch(current_result.token_type) {
                case TokenTypes::short_option_without_value:
                case TokenTypes::short_option_eq_value:
                case TokenTypes::short_option:
                    return current_result.long_option_name;

                case TokenTypes::long_option:
                case TokenTypes::long_option_eq_value:
                    return current_result.short_option_name;
                
                case TokenTypes::value:
                    return current_result.value;
            }
            return ""; // to suppress warning "no return value"
        }

        std::string getValue(std::shared_ptr<AbstractOption> opt) {
            switch(current_result.token_type) {
                case TokenTypes::short_option_without_value: {
                    throw ExpectedValue(opt);
                }                    
                case TokenTypes::long_option:
                case TokenTypes::short_option: {
                    if(eof()) {
                        throw ExpectedValue(opt);
                    }
                    ArgLexer::Result lex_result = ArgLexer::lex(args_[idx++]);
                    if(lex_result.type == ArgLexer::value) {
                        return lex_result.value;
                    }
                    //return args_[idx++]; // force next arg as value
                    throw ExpectedValue(opt);
                }
                case TokenTypes::value:
                case TokenTypes::short_option_eq_value:
                case TokenTypes::long_option_eq_value:
                    return current_result.value;
            }
            return ""; // to suppress warning "no return value"
        }
};

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
                    if(auto p = std::dynamic_pointer_cast<NamedCommand>(opt)) {
                        match = opt->longName().has_value() && opt->longName().value() == grammar_parser_.current_result.value;
                    };
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

class Parser {
    public:
        std::shared_ptr<AbstractOption> options_;
        ValuesStorage storage;

        Parser(std::shared_ptr<AbstractOption> options) : options_{options} {}
        
        bool parse(ArgGrammarParser args) {
            storage.clear();
            opts_counter_.clear();
            std::vector<std::shared_ptr<AbstractOption>> remaining_options;
            std::vector<std::shared_ptr<AbstractOption>> used_options;
            std::set<std::shared_ptr<AbstractOption>> already_joined;
            jointOptionsTo({options_}, remaining_options);
            SingleOptionMatcher matcher(args);
            while(!args.eof()) {
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