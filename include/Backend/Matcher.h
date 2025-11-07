#ifndef _BACKEND_MATCHER_H
#define _BACKEND_MATCHER_H

#include "Option.h"

#include <cassert>
#include <map>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <boost/spirit/include/classic.hpp>


class ArgumentsTokenizer {
        std::vector<std::string> args_;
        size_t idx{0};
        size_t subidx{0};
        static constexpr std::string long_option_regex = "--[A-Za-z0-9_]+";
        static constexpr std::string short_options_regex = "-[A-Za-z0-9]";
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

        grammar:
        ARGS_SEP
        arg = SHORT_NAMES | SHORT_NAME_EQ_VALUE| LONG_NAME | LONG_NAME_EQ_VALUE | VALUE 
        args = arg | args ARGS_SEP arg
        */
    public:    
        enum TokenType {
            long_option,
            long_option_eq_value,
            short_option,
            short_option_eq_value,
            value
        };

        template<class ... T>
        ArgumentsTokenizer(T ... arguments) : args_{arguments...} {
            if(!eof()) {
                startParse();
            }
        }

        void operator++() {
            idx++;
            assert(idx <= args_.size());
        }
        bool eof() {
            return idx == args_.size();
        }

        const std::string& val() {
            assert(idx <= args_.size());
            return args_[idx];
        }
    private:
        void startParse() {            
            if(std::regex_match(args_[idx], std::regex(long_option_regex))) {
            }
        }
};

class SingleOptionMatcher : public AbstractOptionVisitor {
    private:
        ArgumentsTokenizer& args_;
    public:
        std::optional<size_t> match_index;
        std::vector<std::shared_ptr<AbstractOption>> unlocks;
        SingleOptionMatcher(ArgumentsTokenizer& args) : args_{args} {}

        void visit(std::shared_ptr<AbstractOption> opt) override {
            assert(false);
        }
        void visit(std::shared_ptr<AbstractNamedOption> opt) override {
            assert(args_.val().size() > 0);
            match_index = std::nullopt;
            unlocks.clear();
            bool match_long = opt->longName().has_value() && opt->longName().value() == args_.val();
            bool match_short = opt->shortName().has_value() && opt->shortName().value() == args_.val();
            if(match_long || match_short) {
                match_index = 0;
                if(opt->valueRegex()) {
                    /// \todo: match value or read default value
                }
                unlocks = opt->unlocks;
            };
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
                ++args;
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