#ifndef GRAMMAR_PARSER_H
#define GRAMMAR_PARSER_H

#include <Backend/Exceptions.h>  // TODO: split this file into thematic parts
#include <Backend/Option.h>      // TODO this file is included only to pass AbstractOption variable to the exception constructor

#include <cassert>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string_view>
#include <vector>

/* Todo:
1. support for escape character: "--a" is named option, but "\--a" is value
converted to "--a"
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
    /// todo: add "--" support (which signalizes that after it all arguments are
    /// treated as positional options)
   public:
    static constexpr char long_option_pattern[] = "^--([A-Za-z0-9_][A-Za-z0-9_\\-]*)$";                // like --long_name
    static constexpr char long_option_eq_value_pattern[] = "^--([A-Za-z0-9_][A-Za-z0-9_\\-]*)=(.*)$";  // like --long_name=value
    static constexpr char short_options_pattern[] = "^-([A-Za-z0-9]+)$";                               // like -xyz
    static constexpr char short_options_eq_value_pattern[] = "^-([A-Za-z0-9]+)=(.*)$";                 // like -xyz=value
    static constexpr char double_dash_pattern[] = "^--$";                                              // like -xyz=value
    enum TokenType {
        LONG_OPTION,             // --long_option
        LONG_OPTION_EQ_VALUE,    // --long_option=value
        SHORT_OPTIONS,           // -xyz
        SHORT_OPTIONS_EQ_VALUE,  // -xyz=value
        DOUBLE_DASH,             // literally '--'
        VALUE                    // value (for example, positional option)
    };
    struct Result {
        TokenType type;
        std::string long_option_name;    // without leading "--"
        std::string short_option_names;  // without leading "-"
        std::string value;
    };

    static Result lex(const std::string& arg, bool match_only_positional) {
        std::string current_arg = arg;
        std::smatch match;
        std::regex long_option_ex(long_option_pattern);
        std::regex short_options_ex(short_options_pattern);
        std::regex long_option_eq_value_ex(long_option_eq_value_pattern);
        std::regex short_options_eq_value_ex(short_options_eq_value_pattern);
        std::regex double_dash_ex(double_dash_pattern);
        Result res;
        if (!match_only_positional && std::regex_match(arg, match, long_option_ex)) {
            assert(match.size() == 2);
            res.long_option_name = match[1];
            res.type = LONG_OPTION;
        } else if (!match_only_positional && std::regex_match(arg, match, long_option_eq_value_ex)) {
            assert(match.size() == 3);
            res.long_option_name = match[1];
            res.value = undecorateValue(match[2]);
            res.type = LONG_OPTION_EQ_VALUE;
        } else if (!match_only_positional && std::regex_match(arg, match, short_options_ex)) {
            assert(match.size() == 2);
            res.short_option_names = match[1];
            res.type = SHORT_OPTIONS;
        } else if (!match_only_positional && std::regex_match(arg, match, short_options_eq_value_ex)) {
            assert(match.size() == 3);
            res.short_option_names = match[1];
            res.value = undecorateValue(match[2]);
            res.type = SHORT_OPTIONS_EQ_VALUE;
        } else if (std::regex_match(arg, match,
                                    double_dash_ex)) {  // TODO: does we need to check
                                                        // match_only_positional flag here?
            res.type = DOUBLE_DASH;
        } else {
            res.value = undecorateValue(arg);
            res.type = VALUE;
        }
        return res;
    }
    static std::string undecorateValue(const std::string& str) {
        if (str.empty() || str[0] != '\\') {
            return str;
        }
        return str.substr(1, std::string::npos);
    }
};

class ArgGrammarParser {
    std::vector<std::string> args_;
    size_t idx_{0};

   public:
    bool match_only_positional{false};  // when double dash "--" is encountered among the argument
                                        // list, all following options are treated as positional
    enum TokenTypes {
        LONG_OPTION,                 // like --option
        LONG_OPTION_EQ_VALUE,        // like --option=value
        SHORT_OPTION,                // like y option in "-xy" (can be followed by a value)
        SHORT_OPTION_WITHOUT_VALUE,  // like x option in "-xy" (can't be
                                     // followed by a value)
        SHORT_OPTION_EQ_VALUE,       // like y option in "-xy=value"
        DOUBLE_DASH,                 // literally '--', used to indicate that all trailing
                                     // options should be treated in special manner
        VALUE                        // anything else which cannot be interpreted as a long option or
                                     // a chain of the short options
    };
    struct Result {
        TokenTypes token_type;
        std::string long_option_name;
        std::string short_option_name;
        std::string value;
        size_t index;
    };
    Result current_result;

    template <class... Type>
    ArgGrammarParser(Type... arguments) : args_{arguments...} {}

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

    bool eof() { return idx_ == args_.size() && results_.empty(); }

    Result getNextOption() {
        assert(eof() == false);
        if (results_.empty()) {
            assert(idx_ < args_.size());
            size_t cur_idx = idx_;
            auto lex_result = ArgLexer::lex(args_[idx_++], match_only_positional);
            switch (lex_result.type) {
                case ArgLexer::LONG_OPTION:
                    results_.push_back(Result{TokenTypes::LONG_OPTION, lex_result.long_option_name, "", "", cur_idx});
                    break;
                case ArgLexer::LONG_OPTION_EQ_VALUE:
                    results_.push_back(
                        Result{TokenTypes::LONG_OPTION_EQ_VALUE, lex_result.long_option_name, "", lex_result.value, cur_idx});
                    break;
                case ArgLexer::SHORT_OPTIONS: {
                    for (size_t n = 0; n + 1 < lex_result.short_option_names.size(); n++) {
                        std::string str{lex_result.short_option_names[n]};
                        results_.push_back(Result{TokenTypes::SHORT_OPTION_WITHOUT_VALUE, "", str, ""});
                    }
                    std::string str{lex_result.short_option_names.back()};
                    results_.push_back(Result{TokenTypes::SHORT_OPTION, "", str, "", cur_idx});
                    break;
                }
                case ArgLexer::SHORT_OPTIONS_EQ_VALUE: {
                    for (size_t n = 0; n + 1 < lex_result.short_option_names.size(); n++) {
                        std::string str{lex_result.short_option_names[n]};
                        results_.push_back(Result{TokenTypes::SHORT_OPTION_WITHOUT_VALUE, "", str, "", cur_idx});
                    }
                    std::string str{lex_result.short_option_names.back()};
                    results_.push_back(Result{TokenTypes::SHORT_OPTION_EQ_VALUE, "", str, lex_result.value, cur_idx});
                    break;
                }
                case ArgLexer::DOUBLE_DASH: {
                    results_.push_back(Result{TokenTypes::DOUBLE_DASH, "", "", "", cur_idx});
                    break;
                }
                case ArgLexer::VALUE:
                    results_.push_back(Result{TokenTypes::VALUE, "", "", lex_result.value, cur_idx});
            }
        }
        current_result = std::move(results_.front());
        results_.pop_front();
        return current_result;
    }

    void ungetOption() { results_.push_front(current_result); }

    /// the raw option name is used in error messages to address the option by
    /// the name that was actually passed (short or long)
    std::string getRawOptionName() const {
        switch (current_result.token_type) {
            case TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case TokenTypes::SHORT_OPTION_EQ_VALUE:
            case TokenTypes::SHORT_OPTION:
                return current_result.long_option_name;

            case TokenTypes::LONG_OPTION:
            case TokenTypes::LONG_OPTION_EQ_VALUE:
                return current_result.short_option_name;

            case TokenTypes::DOUBLE_DASH:
                return "--";

            case TokenTypes::VALUE:
                return current_result.value;
        }
        return "";  // to suppress warning "no return value"
    }

    std::optional<std::string> getValueOpt(std::shared_ptr<AbstractOption> opt) {  /// todo: opt is only used to print an error message
        switch (current_result.token_type) {
            case TokenTypes::SHORT_OPTION_WITHOUT_VALUE: {
                //throw ExpectedValue(opt);
                return std::nullopt;
            }
            case TokenTypes::LONG_OPTION:
            case TokenTypes::SHORT_OPTION: {
                if (eof()) {
                    return std::nullopt;
                    //throw ExpectedValue(opt);
                }
                ArgLexer::Result lex_result = ArgLexer::lex(args_[idx_++], match_only_positional);
                if (lex_result.type == ArgLexer::VALUE) {
                    return lex_result.value;
                }
                return std::nullopt;
                //throw ExpectedValue(opt);
            }
            case TokenTypes::DOUBLE_DASH: {
                return std::nullopt;
                //throw ExpectedValue(opt);
            }
            case TokenTypes::VALUE:
            case TokenTypes::SHORT_OPTION_EQ_VALUE:
            case TokenTypes::LONG_OPTION_EQ_VALUE:
                return current_result.value;
        }
        return std::nullopt;  // to suppress warning "no return value"
    }
    size_t getNextIndex() const {
        // return arg index that will be parsed on next call of getNextOption()
        return idx_;
    }
    size_t size() { return args_.size(); }

   private:
    std::deque<Result> results_;  // todo: remove
};

#endif
