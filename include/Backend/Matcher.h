#ifndef _BACKEND_MATCHER_H
#define _BACKEND_MATCHER_H

#include "Option.h"
#include "Exceptions.h"
#include "ValueSemantics.h"

#include <cassert>
#include <map>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <queue>
#include <boost/spirit/include/classic.hpp>

class ArgLexer {
        /*
        Examples of possible tokens:
        --long_name
        --long_name=value        
        -xyz
        -xyz=value  
        value
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
                res.value = match[2];
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
                res.value = match[2];
                res.type = short_options_eq_value;
            } else {
                res.value = arg;
                res.type = value;
            }
            return res;
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


        template<class ... T>
        ArgGrammarParser(T ... arguments) : args_{arguments...} {
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

        std::string getValue(std::shared_ptr<AbstractOption> opt) {
            switch(current_result.token_type) {
                case TokenTypes::short_option_without_value: {
                    throw ExpectedValue(opt);
                }                    
                case TokenTypes::long_option:
                case TokenTypes::short_option:
                    if(eof()) {
                        throw ExpectedValue(opt);
                    }
                    return args_[idx++];
                case TokenTypes::value:
                case TokenTypes::short_option_eq_value:
                case TokenTypes::long_option_eq_value:
                    return current_result.value;
            }
                        
        }
};

class SingleOptionMatcher : public AbstractOptionVisitor {
    private:
        ArgGrammarParser& grammar_parser_;
    public:
        std::optional<size_t> match_index;
        std::string value;
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        SingleOptionMatcher(ArgGrammarParser& args) : grammar_parser_{args} {}

        void visit(std::shared_ptr<AbstractOption> opt) override {
            assert(false);
        }
        void visit(std::shared_ptr<NamedOption> opt) override {
            match_index = std::nullopt;
            unlocks.clear();
            bool match = false;
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
                match_index = 0;
                unlocks = opt->unlocks;
            };
        }
        void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
            match_index = std::nullopt;
            unlocks.clear();
            bool match = false;
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
                match_index = 0;
                if(opt->valueRequired()) {
                    value = grammar_parser_.getValue(opt);
                }
                unlocks = opt->unlocks;
            };
        }
        void visit(std::shared_ptr<NamedCommand> opt) override {
            match_index = std::nullopt;
            unlocks.clear();
            bool match = false;
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
                match_index = 0;                
                unlocks = opt->unlocks;
            };
        }
        void visit(std::shared_ptr<PositionalOption> opt) override {
            match_index = std::nullopt;
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
            match_index = 0;
            value = grammar_parser_.getValue(opt);
            unlocks = opt->unlocks;
        }
        void visit(std::shared_ptr<OptionsGroup> opt) override {
            match_index = 0;            
            unlocks = opt->unlocks;
        }
        void visit(std::shared_ptr<OneOf> opt) override {
            match_index = std::nullopt;
            unlocks.clear();
            for(size_t n = 0; n < opt->alternatives.size(); n++) {
                opt->alternatives[n]->accept(*this);
                if(match_index.has_value()) {
                    match_index = n;
                    unlocks = opt->alternatives[n]->unlocks;
                    return;
                }
            }
        }
};

struct Value {
    std::vector<std::string> values;
    bool is_default;
};

class Parser {
    public:

        std::map<std::shared_ptr<AbstractOption>, Value> values;
        std::shared_ptr<AbstractOption> options_;

        Parser(std::shared_ptr<AbstractOption> options) : options_{options} {}            
        
        bool parse(ArgGrammarParser args) {
            values.clear();
            std::vector<std::shared_ptr<AbstractOption>> rest_options_;            
            if(auto q = std::dynamic_pointer_cast<OptionsGroup>(options_)) {
                rest_options_ = options_->unlocks;
            } else {
                rest_options_.push_back(options_);
            }            
            while(!args.eof()) {
                args.getNextOption(); 
                SingleOptionMatcher matcher(args);
                bool option_matched = false;
                for(auto it = rest_options_.begin(); it != rest_options_.end(); it++) {
                    (*it)->accept(matcher);
                    if(matcher.match_index.has_value()) {
                        setValue(*it, matcher);
                        rest_options_.erase(it);
                        rest_options_.insert(rest_options_.end(), matcher.unlocks.begin(), matcher.unlocks.end());
                        option_matched = true;
                        break;
                    };
                }
                // unknown option or duplicate option
                if(!option_matched)
                    return false;
            }
            // check that all required options are used
            for(auto p : rest_options_)
            {
                if(p->required()) {
                    if(auto q = std::dynamic_pointer_cast<NamedOption>(p)) {
                        std::stringstream str;
                        throw RequiredOptionIsNotSet(q);
                    }
                    if(auto q = std::dynamic_pointer_cast<PositionalOption>(p)) {
                        throw TooFewPositionalOptions(); /// \todo: how many pos options are expected
                    }
                    if(auto q = std::dynamic_pointer_cast<OneOf>(p)) {
                        throw RequiredOptionIsNotSet(q);
                    }
                }
            }
            return true;
        }
    private:
        std::string displayName(std::shared_ptr<NamedOption> opt) {
            if(opt->longName())
                return *opt->longName();
            return *opt->shortName();
        }

        void setValue(std::shared_ptr<AbstractOption> opt, const SingleOptionMatcher& matcher) { 
            if(auto p = std::dynamic_pointer_cast<AbstractNamedOptionWithValue>(opt)) {
                Value& v = values[opt];
                if(v.values.size() >= p->maxOccurrence()) {
                    throw MaxOptionOccurenceIsExceeded(p);
                }
                if(p->valueRequired()) {
                    v.values.push_back(matcher.value);
                }
                v.is_default = false;
                p->baseValueSemantics().setValue(matcher.value);
            } else {
                if(values[opt].values.size() >= p->maxOccurrence()) {
                    throw MaxOptionOccurenceIsExceeded(p);
                }
                values.insert(std::make_pair(opt, Value()));
            }
        }
};



#endif 