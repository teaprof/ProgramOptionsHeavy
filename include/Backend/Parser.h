#ifndef __BACKEND_PARSER_H__
#define __BACKEND_PARSER_H__

#include "Option.h"

#include <cassert>
#include <map>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <queue>
#include <set>

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
    public:
        static constexpr char long_option_pattern[] = "^--([A-Za-z0-9_\\-]+)"; // like --long_name
        static constexpr char long_option_eq_value_pattern[] = "^--([A-Za-z0-9_\\-]+)=(.*)"; // like --long_name=value
        static constexpr char short_options_pattern[] = "^-([A-Za-z0-9]+)"; // like -xyz
        static constexpr char short_options_eq_value_pattern[] = "^-([A-Za-z0-9]+)=(.*)"; //like -xyz=value
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
        size_t idx_{0};
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
            size_t index;
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
            return idx_ == args_.size() && results.empty();
        }

        Result getNextOption() {
            assert(eof() == false);
            if(results.empty()) {
                assert(idx_ < args_.size());
                size_t cur_idx = idx_;
                auto lex_result = ArgLexer::lex(args_[idx_++]);
                switch(lex_result.type) {
                    case ArgLexer::long_option:
                        results.push(Result{TokenTypes::long_option, lex_result.long_option_name, "", "", cur_idx});
                        break;
                    case ArgLexer::long_option_eq_value:
                        results.push(Result{TokenTypes::long_option_eq_value, lex_result.long_option_name, "", lex_result.value, cur_idx});
                        break;
                    case ArgLexer::short_options: {
                        for(size_t n = 0; n + 1 < lex_result.short_option_names.size(); n++) {
                            std::string str{lex_result.short_option_names[n]};
                            results.push(Result{TokenTypes::short_option_without_value, "", str, ""});
                        }
                        std::string str{lex_result.short_option_names.back()};
                        results.push(Result{TokenTypes::short_option, "", str, "", cur_idx});
                        break;
                        }
                    case ArgLexer::short_options_eq_value: {
                        for(size_t n = 0; n + 1 < lex_result.short_option_names.size(); n++) {
                            std::string str{lex_result.short_option_names[n]};
                            results.push(Result{TokenTypes::short_option_without_value, "", str, "", cur_idx});
                        }
                        std::string str{lex_result.short_option_names.back()};
                        results.push(Result{TokenTypes::short_option_eq_value, "", str, lex_result.value, cur_idx});
                        break;
                        }
                    case ArgLexer::value:
                        results.push(Result{TokenTypes::value, "", "", lex_result.value, cur_idx});
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
                    ArgLexer::Result lex_result = ArgLexer::lex(args_[idx_++]);
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
        size_t getNextIndex() {
            // return arg index that will be parsed on next call of getNextOption()
            return idx_;
        }
        size_t size() {
            return args_.size();
        }
};


#endif
