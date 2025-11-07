#ifndef _BACKEND_MATCHER_H
#define _BACKEND_MATCHER_H

#include "Option.h"

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
        --long_name_wo_value
        --long_name=value
        --long_name value
        -s
        -s value
        -s=value
        -xyz
        -xyz value  
        -xyz positional_name
        */
    private:
        static constexpr char long_option_pattern[] = "^--([A-Za-z0-9_\\-]+)";
        static constexpr char long_option_eq_value_pattern[] = "^--([A-Za-z0-9_\\-]+)=(.*)";
        static constexpr char short_options_pattern[] = "^-([A-Za-z0-9]+)";
        static constexpr char short_options_eq_value_pattern[] = "^-([A-Za-z0-9]+)=(.*)";
        //static constexpr char eq_pattern[] = "=";
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

        /*static Result lex(const std::string& arg) {
            std::string current_arg = arg;
            std::smatch match;
            std::regex long_option_ex(long_option_pattern);
            std::regex short_options_ex(short_options_pattern);
            std::regex eq_ex(eq_pattern);
            Result res;
            if(std::regex_search(current_arg, match, long_option_ex)) {
                res.long_option_name = current_arg.substr(0, match[0].length());
                res.long_option_name = res.long_option_name.substr(2, std::string::npos); // remove leading "--""
                current_arg = current_arg.substr(match.length(), std::string::npos);
                if(std::regex_search(current_arg, match, eq_ex)) {
                    current_arg = current_arg.substr(match.length(), std::string::npos);
                    res.value = current_arg;
                    res.type = TokenType::long_option_eq_value;
                } else {
                    if(current_arg.empty()) {
                        res.type = TokenType::long_option;
                    } else {
                        res.type = TokenType::value;
                        res.value = arg;
                    }
                }
            } else if(std::regex_search(current_arg, match, short_options_ex)) {
                res.short_option_names = current_arg.substr(0, match.length());
                res.short_option_names = res.short_option_names.substr(1, std::string::npos);  // remove leading "-""
                current_arg = current_arg.substr(match.length(), std::string::npos);
                if(std::regex_search(current_arg, match, eq_ex)) {
                    current_arg = current_arg.substr(match.length(), std::string::npos);
                    res.value = current_arg;
                    res.type = TokenType::short_options_eq_value;
                } else {
                    if(current_arg.empty()) {
                        res.type = TokenType::short_options;
                    } else {
                        res.type = TokenType::value;
                        res.value = arg;
                    }
                }
            } else {
                res.value = arg;
                res.type = TokenType::value;
            }
            return res;
        }        */
        static Result lex(const std::string& arg) {
            std::string current_arg = arg;
            std::smatch match;
            std::regex long_option_ex(long_option_pattern);
            std::regex short_options_ex(short_options_pattern);
            std::regex long_option_eq_value_ex(long_option_eq_value_pattern);
            std::regex short_options_eq_value_ex(short_options_eq_value_pattern);
            Result res;
            if(std::regex_match(arg, match, long_option_ex)) {                
                for(size_t n = 0; n < match.size(); n++)                
                    std::cout<<n<<" : "<<match[n]<<std::endl;
                assert(match.size() == 2);
                res.long_option_name = match[1];
                res.type = long_option;
            } else 
            if(std::regex_match(arg, match, long_option_eq_value_ex)) {
                for(size_t n = 0; n < match.size(); n++)                
                    std::cout<<n<<" : "<<match[n]<<std::endl;
                assert(match.size() == 3);
                res.long_option_name = match[1];
                res.value = match[2];
                res.type = long_option_eq_value;
            } else 
            if(std::regex_match(arg, match, short_options_ex)) {
                for(size_t n = 0; n < match.size(); n++)                
                    std::cout<<n<<" : "<<match[n]<<std::endl;
                assert(match.size() == 2);
                res.short_option_names = match[1];
                res.type = short_options;
            } else 
            if(std::regex_match(arg, match, short_options_eq_value_ex)) {
                for(size_t n = 0; n < match.size(); n++)                
                    std::cout<<n<<" : "<<match[n]<<std::endl;
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


class ArgumentsTokenizer {
        std::vector<std::string> args_;
        size_t idx{0};
        size_t subidx{0};
    public:
        enum class TokenTypes {
            long_option, // --long_option
            long_option_eq_value, // --long_option=value
            short_option, // -x
            short_option_without_value, // when short option is specified like -xyz, the `y` option can't have value
            short_option_eq_value, // -s=value
            value
        };        
        struct Result {
            TokenTypes token_type;
            std::string long_option_name; // without leading "--"
            std::string short_option_name; // without leading "-"
            std::string value;
        };
        Result current_result;        
        std::queue<Result> results;     


        template<class ... T>
        ArgumentsTokenizer(T ... arguments) : args_{arguments...} {
        }

        ArgumentsTokenizer(const std::string& str) {
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

        std::string getValue() {
            switch(current_result.token_type) {
                case TokenTypes::short_option_without_value: {
                    std::stringstream str;
                    str<<"Option -"<<current_result.short_option_name<<" requires a value";
                    throw std::runtime_error(str.str());
                }                    
                case TokenTypes::long_option:
                case TokenTypes::short_option:
                    assert(!eof());
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
        ArgumentsTokenizer& tokenizer_;
    public:
        std::optional<size_t> match_index;
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        SingleOptionMatcher(ArgumentsTokenizer& args) : tokenizer_{args} {}

        void visit(std::shared_ptr<AbstractOption> opt) override {
            assert(false);
        }
        void visit(std::shared_ptr<AbstractNamedOption> opt) override {
            /*match_index = std::nullopt;
            unlocks.clear();
            bool match_long = opt->longName().has_value() && opt->longName().value() == tokenizer_.val();
            bool match_short = opt->shortName().has_value() && opt->shortName().value() == tokenizer_.val();
            if(match_long || match_short) {
                match_index = 0;
                if(opt->valueRegex()) {

                }
                unlocks = opt->unlocks;
            };*/
        }
        void visit(std::shared_ptr<AbstractPositionalOption>) override {
            match_index = std::nullopt;
            unlocks.clear();
            assert(false);
        }
        void visit(std::shared_ptr<Compatibles> opt) override {
            match_index = 0;            
            unlocks = opt->unlocks;
        }
        void visit(std::shared_ptr<Alternatives> opt) override {
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

class Parser {
    public:
        struct Value {
    
        };

        std::map<std::shared_ptr<AbstractOption>, Value> values;
        std::shared_ptr<AbstractOption> options_;

        Parser(std::shared_ptr<AbstractOption> options) : options_{options} {}            
        
        bool parse(ArgumentsTokenizer args) {
            values.clear();
            std::vector<std::shared_ptr<AbstractOption>> rest_options_;            
            if(auto q = std::dynamic_pointer_cast<Compatibles>(options_)) {
                rest_options_ = options_->unlocks;
            } else {
                rest_options_.push_back(options_);
            }
            while(!args.eof()) {
                SingleOptionMatcher matcher(args);
                bool option_matched = false;
                for(auto it = rest_options_.begin(); it != rest_options_.end(); it++) {
                    (*it)->accept(matcher);
                    if(matcher.match_index.has_value()) {
                        if(values.contains(*it)) {
                            assert(false); // duplicate parameter
                        }
                        values.insert(std::make_pair(*it, Value{}));
                        rest_options_.erase(it);
                        rest_options_.insert(rest_options_.end(), matcher.unlocks.begin(), matcher.unlocks.end());
                        option_matched = true;
                        break;
                    };
                }
                // unknown option or duplicate option
                if(!option_matched)
                    return false;
                //++args;
            }
            // check that all required options are used
            for(auto p : rest_options_)
            {
                if(p->required()) {
                    if(auto q = std::dynamic_pointer_cast<AbstractNamedOption>(p)) {
                        std::stringstream str;
                        str << "Option " << displayName(q) << " is required";
                        throw std::runtime_error(str.str());
                    }
                    if(auto q = std::dynamic_pointer_cast<AbstractPositionalOption>(p)) {
                        //throw std::runtime_error("At least "<<num<<" positional options are required");
                        throw std::runtime_error("Too few positional options are specified"); /// \todo: correct message
                    }
                    if(auto q = std::dynamic_pointer_cast<Alternatives>(p)) {
                        throw std::runtime_error("Alternative is not specifoed"); /// \todo: correct message
                    }
                }
            }
            return true;
        }
    private:
        std::string displayName(std::shared_ptr<AbstractNamedOption> opt) {
            if(opt->longName())
                return *opt->longName();
            return *opt->shortName();
        }
};



#endif 