#ifndef BACKEND_MATCHER_H
#define BACKEND_MATCHER_H

#include <Checker/Checker.h>
#include <Lexer/Lexer.h>

#include <cassert>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string_view>
#include <vector>

#include "Exceptions.h"
#include "Option.h"
#include "ValueSemantics.h"
#include "ValueStorage.h"

class SingleOptionMatcher : public AbstractOptionVisitor {
   private:
    ArgGrammarParser& grammar_parser_;

   public:
    bool match;
    std::optional<std::string> value;
    std::vector<std::shared_ptr<AbstractOption>> unlocks;
    SingleOptionMatcher(ArgGrammarParser& args) : grammar_parser_{args} {}

    void visit(std::shared_ptr<AbstractOption> opt) override {
        // should not never visit abstract object
        assert(false);
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
        match = false;
        unlocks.clear();
        switch (grammar_parser_.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
                return;
            case ArgGrammarParser::TokenTypes::VALUE:
                /* nothing to do */;
        }
        match = true;
        value = grammar_parser_.getValueOpt();
        unlocks = opt->unlocks();               // todo: avoid copying of the vector
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
        unlocks.clear();
        match = false;
        switch (grammar_parser_.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
                break;
            case ArgGrammarParser::TokenTypes::VALUE:
                match = opt->str() == grammar_parser_.current_result.value;
                break;
        }
        if (match) {
            value = std::nullopt;
            unlocks = opt->unlocks();  // todo: avoid copying of a vector
        };
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        unlocks.clear();
        match = false;
        switch (grammar_parser_.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
                match = opt->longName().has_value() && opt->longName().value() == grammar_parser_.current_result.long_option_name;
                break;
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
                match =
                    opt->shortName().has_value() && opt->shortName().value() == grammar_parser_.current_result.short_option_name;
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
            case ArgGrammarParser::TokenTypes::VALUE:
                break;
        }
        if (match) {
            value = std::nullopt;
            unlocks = opt->unlocks();  // todo: avoid copying of a vector
        };
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        unlocks.clear();
        match = false;
        switch (grammar_parser_.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
                match = opt->longName().has_value() && opt->longName().value() == grammar_parser_.current_result.long_option_name;
                break;
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
                match =
                    opt->shortName().has_value() && opt->shortName().value() == grammar_parser_.current_result.short_option_name;
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
            case ArgGrammarParser::TokenTypes::VALUE:
                /* nothing to do*/;
        }
        if (match) {
            value = grammar_parser_.getValueOpt();
            unlocks = opt->unlocks();  // todo: avoid copying of a vector
        };
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        // should not never visit abstract object
        assert(false);
    }
    void visit(std::shared_ptr<OptionsGroup> opt) override {
        assert(false);
        match = true;
        value = std::nullopt;
        unlocks = opt->unlocks();  // todo: avoid copying of a vector
    }
    void visit(std::shared_ptr<OneOfPositional> opt) override {
        match = false;
        unlocks.clear();
        for (size_t n = 0; n < opt->alternativesSize(); n++) {
            auto alt = opt->alternative(n);
            alt->accept(*this);
            if (match) {
                value = std::nullopt;
                unlocks = opt->alternative(n)->unlocks();  // todo: avoid copying of a vector
                return;
            }
        }
    }
    void visit(std::shared_ptr<OneOfNamed> opt) override {
        match = false;
        unlocks.clear();
        for (size_t n = 0; n < opt->alternativesSize(); n++) {
            auto alt = opt->alternative(n);
            alt->accept(*this);
            if (match) {
                value = std::nullopt;
                unlocks = opt->alternative(n)->unlocks();  // todo: avoid copying of a vector
                return;
            }
        }
    }

    void setPositionalOnlyFlag(bool value) { grammar_parser_.match_only_positional = value; }
};

class BaseParser {
   protected:
    std::vector<std::shared_ptr<AbstractOption>> remaining_options_;
    // std::vector<std::shared_ptr<AbstractOption>> used_options_;
    std::set<std::shared_ptr<AbstractOption>> already_joined_;
    std::map<std::shared_ptr<AbstractOption>, size_t> opts_counter_;
    std::set<std::shared_ptr<AbstractOptionWithValue>> opts_with_implicit_value_;
    std::shared_ptr<AbstractOption> options_;
    size_t cur_positional_option_idx_{0};

    //std::vector<std::vector<std::shared_ptr<AbstractOption>>> mutual_exclusive_options_;

   public:
    KeyValueStorage storage;

    BaseParser(std::shared_ptr<AbstractOption> options) : options_{options} {
        // Checker checker;
        // options->accept(checker);
    }

    void clear() {
        remaining_options_.clear();
        // used_options_.clear();
        already_joined_.clear();
        storage.clear();
        opts_counter_.clear();
        cur_positional_option_idx_ = 0;
        joinOptionsTo({options_}, remaining_options_);
    }

    std::shared_ptr<AbstractPositionalOption> eatNextPositionalOption(ArgGrammarParser& args, SingleOptionMatcher& matcher) {
        auto find_next_positional = [&](size_t idx)->auto {
            std::shared_ptr<AbstractPositionalOption> p;
            while(idx < remaining_options_.size()) {
                auto opt = remaining_options_[idx];
                p = std::dynamic_pointer_cast<AbstractPositionalOption>(opt);
                if (p != nullptr) {
                    break;
                }
                idx++;
            }
            return idx;
        };
        cur_positional_option_idx_ = find_next_positional(cur_positional_option_idx_);        
        if(cur_positional_option_idx_ == remaining_options_.size()) {
            throw TooManyPositionalOptions("");
        }
        auto opt = remaining_options_[cur_positional_option_idx_];
        if (!canAcceptNewOccurrence(opt)) {
            cur_positional_option_idx_++;
            cur_positional_option_idx_ = find_next_positional(cur_positional_option_idx_);        
            if(cur_positional_option_idx_ == remaining_options_.size()) {
                throw TooManyPositionalOptions("");
            }
            opt = remaining_options_[cur_positional_option_idx_];
        }
        opt->accept(matcher);
        if (matcher.match) {
            auto res = std::dynamic_pointer_cast<AbstractPositionalOption>(opt);
            assert(res);
            return res;
        };
        return nullptr;
    }

    std::shared_ptr<AbstractOption> eatNextNamedOption(ArgGrammarParser& args, SingleOptionMatcher& matcher) {
        for (auto opt : remaining_options_) {
            opt->accept(matcher);
            if (matcher.match) {
                return opt;
            };
        }
        return nullptr;
    }

    std::shared_ptr<AbstractOption> eatNextOption(ArgGrammarParser& args, SingleOptionMatcher& matcher) {
        args.getNextOption();
        // If DOUBLE_DASH then all next options will be treated as positionals
        if (args.current_result.token_type == ArgGrammarParser::TokenTypes::DOUBLE_DASH) {
            matcher.setPositionalOnlyFlag(true);
            cur_positional_option_idx_++;
            return nullptr;
        }
        std::shared_ptr<AbstractOption> res = nullptr;
        switch (args.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
                res = eatNextNamedOption(args, matcher);
                if (res == 0) {
                    throw UnknownNamedOption("");
                }
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
            case ArgGrammarParser::TokenTypes::VALUE:
                res = eatNextPositionalOption(args, matcher);
                if (res == 0) {
                    throw UnexpectedValueForPositionalOption(args.current_result.value);
                }
                break;
        }
        if (res) {
            return res;
        }
        throw UnknownNamedOption(args.getRawOptionName());
        return nullptr;
    }

    void eatValueIfCan(ArgGrammarParser& args, SingleOptionMatcher& matcher, std::shared_ptr<AbstractOptionWithValue> opt) {
        // first, try to eat matcher.value
        if (matcher.value.has_value()) {
            setOptionValue(opt, matcher.value.value());
            matcher.value = std::nullopt;
            return;
        }
        if(!args.eof()) {
            args.getNextOption();
            bool arg_is_value = args.current_result.token_type == ArgGrammarParser::VALUE;
            if (arg_is_value) {
                std::optional<std::string> value_opt = args.getValueOpt();
                assert(value_opt.has_value());
                setOptionValue(opt, *value_opt);
                return;
            };
            args.ungetOption();
        }
        // Try to apply default value
        bool has_default_value = opt->baseValueSemantics().hasDefaultValue();
        if (has_default_value) {
            setDefaultValue(opt);
            return;
        }
        throw ExpectedValue(opt);
    }

    void parseNext(ArgGrammarParser& args) {
        SingleOptionMatcher matcher(args);
        auto opt = eatNextOption(args, matcher);
        if (opt) {
            if (!already_joined_.contains(opt)) {
                joinOptionsTo(matcher.unlocks, remaining_options_);
                already_joined_.insert(opt);
            }

            if (auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                eatValueIfCan(args, matcher, p);
                ensureValueListIsCompleted(p);
            };

            if (increaseOptionOccurrenceCounter(opt)) {
                // if the number of occurences of this option is exausted
                // then remove this option from the list of remaining
                // options
                // std::erase(remaining_options_, opt);
                // used_options_.push_back(opt);
            }

            if (matcher.value.has_value()) {
                throw OptionDoesntAcceptValue();
            }
        };
    }

    void parse(ArgGrammarParser args) { 
        clear();
        //SingleOptionMatcher matcher(args); 
        while (!args.eof()) {
            parseNext(args);
        }
    }

   protected:
    /// how many times the specified option has been encountered in the already
    /// parsed context
    size_t optionEncountered(std::shared_ptr<AbstractOption> opt) {
        size_t counter = 0;
        if (opts_counter_.contains(opt)) {
            counter = opts_counter_[opt];
        }
        return counter;
    }
    void joinOptionsTo(const std::vector<std::shared_ptr<AbstractOption>>& src_options,
                       std::vector<std::shared_ptr<AbstractOption>>& dst_options) {
        for (auto it : src_options) {
            if (auto p = std::dynamic_pointer_cast<OptionsGroup>(it)) {
                joinOptionsTo(p->unlocks(),
                              dst_options);  // todo: avoid copying of a vector
            } else {
                dst_options.push_back(it);
            }
        }
    }
    bool increaseOptionOccurrenceCounter(std::shared_ptr<AbstractOption> opt) {
        size_t counter = ++opts_counter_[opt];
        if (counter == opt->maxOccurrence()) {
            return true;
        }
        if (counter > opt->maxOccurrence()) {
            if (auto p = std::dynamic_pointer_cast<OneOfPositional>(opt)) {
                throw OnlyOneChoiseIsAllowed(p);  // todo: make separate checker for OneOfPositional
            }
            throw MaxOptionOccurrenceIsExceeded(opt);
        }
        return false;
    }
    void ensureValueListIsCompleted(std::shared_ptr<AbstractOptionWithValue> opt) {
        size_t actual = 0;
        if(storage.contains(opt)) {
            actual = storage[opt].lastOccurrenceSize();                
        }
        if (opt->nValuesRole() == AbstractOptionWithValue::NValuesRole::EXACT) {
            size_t expected = opt->nValues();
            if (actual < expected) {
                if(actual == 0) {
                    throw ExpectedValue(opt);
                }
                throw TooFewValuesForOption(opt);  // todo print message
            }
        } else {
            if (actual == 0) {
                throw ExpectedValue(opt);
            }
        }
    }
    void setDefaultValue(std::shared_ptr<AbstractOptionWithValue> opt) {
        std::any v = opt->baseValueSemantics().setToDefault();
        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
        joinOptionsTo(unlocked_by_value, remaining_options_);
        storage.addValue(opt, "", v);
    }

    void setImplicitValue(std::shared_ptr<AbstractOptionWithValue> opt) {
        std::any v = opt->baseValueSemantics().setToImplicit();
        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
        joinOptionsTo(unlocked_by_value, remaining_options_);
        storage.addValue(opt, "", v);
    }

    void setOptionValue(std::shared_ptr<AbstractOptionWithValue> opt, const std::string& value) {
        bool first = true;
        std::vector<std::string> tokens = mysplit(value);
        switch (opt->nValuesRole()) {
            case AbstractOptionWithValue::NValuesRole::EXACT:
                if (opt->nValues() != tokens.size()) {
                    throw ExpectedExactNumberOfValues(opt);
                }
            case AbstractOptionWithValue::NValuesRole::UPTO:
                if (tokens.size() > opt->nValues()) {
                    throw TooManyValuesForOption(opt);
                }
            case AbstractOptionWithValue::NValuesRole::INFINITE:
                /* nothing to do*/
                break;
        }
        for (auto token : tokens) {
            std::any val = opt->baseValueSemantics().semanticParse(token);
            std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
            joinOptionsTo(unlocked_by_value, remaining_options_);
            if (first) {
                first = false;
                storage.addValue(opt, token, val);
            } else {
                storage.addValueToCurrentOccurence(opt, token, val);
            }
        }
    }

    public:
    bool canAcceptNewOccurrence(std::shared_ptr<AbstractOption> opt) {
        size_t actual_count = 0;
        if (opts_counter_.contains(opt)) {
            actual_count = opts_counter_[opt];
        }
        size_t max_count = opt->maxOccurrence();
        bool can_accept = (actual_count < max_count);
        return can_accept;
    }
};

class Parser2 : public BaseParser {
   public:
    Parser2(std::shared_ptr<AbstractOption> options) : BaseParser{options} {}

    bool parse(ArgGrammarParser args) {
        BaseParser::parse(args);
        applyImplicitValues();
        checkUnusedRequiredOptions();
        return true;
    }

   private:
    bool isOptionSpecifiedOrImplied(std::shared_ptr<AbstractOption> opt) {
        if(optionEncountered(opt) > 0) {
            return true;
        }
        if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
            if(opts_with_implicit_value_.contains(p)) {
                return true;
            }
        }
        return false;
    };
    void checkUnusedRequiredOptions() {
        // check that all required options are used
        bool too_few_pos_opts{false};
        bool required_opt_is_not_set{false};
        for (auto opt : remaining_options_) {
            if (opt->required() && !isOptionSpecifiedOrImplied(opt)) {
                auto q = std::dynamic_pointer_cast<AbstractPositionalOption>(opt);
                if (!q) {
                    too_few_pos_opts = true;
                }
                required_opt_is_not_set = true;
            }
        }
        if(required_opt_is_not_set) {
            throw RequiredOptionIsNotSet(nullptr);
        }
        if(too_few_pos_opts) {
            /// TODO print how many pos options are expected
            throw TooFewPositionalOptions();
        }
    }

    void applyImplicitValues() {
        // applies default values for options that were not encountered
        for (auto opt : remaining_options_) {
            if (optionEncountered(opt) > 0) {
                continue;
            }
            if (auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                if (p->baseValueSemantics().hasImplicitValue()) {
                    setImplicitValue(p);
                    opts_with_implicit_value_.insert(p);
                    // todo: if implicit value unlocks some options these options should be proccessed
                } else {
                    if (opt->required()) {
                        throw RequiredOptionIsNotSet(opt);
                    }
                }
            };
        }
    }
};

#endif