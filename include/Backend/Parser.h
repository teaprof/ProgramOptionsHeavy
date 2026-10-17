#ifndef BACKEND_MATCHER_H
#define BACKEND_MATCHER_H

//#include <Checker/Checker.h>
#include <Lexer/Lexer.h>

#include <cassert>
#include <iostream>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string_view>
#include <vector>

#include "Exceptions.h"
#include "Option.h"
#include "OptionMatcher.h"
#include "ValueSemantics.h"
#include "ValueStorage.h"

#define PARSER_DEBUG

class OptionsOccurrenceCounter {
    public:

    void clear() {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsOccurrenceCounter::clear() - clearing counter map (size=" << opts_counter_.size() << ")\n";
#endif
        opts_counter_.clear();
    }
    /// how many times the specified option has been encountered in the already
    /// parsed context
    size_t optionEncountered(std::shared_ptr<AbstractOption> opt) {
        size_t counter = 0;
        if (opts_counter_.contains(opt)) {
            counter = opts_counter_[opt];
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsOccurrenceCounter::optionEncountered() - counter=" << counter << "\n";
#endif
        return counter;
    }
    bool increaseOptionOccurrenceCounter(std::shared_ptr<AbstractOption> opt) {
        size_t counter = ++opts_counter_[opt];
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsOccurrenceCounter::increaseOptionOccurrenceCounter() - counter=" << counter << ", max=" << opt->maxOccurrence() << "\n";
#endif
        if (counter == opt->maxOccurrence()) {
            return true;
        }
        if (counter > opt->maxOccurrence()) {
            if (auto p = std::dynamic_pointer_cast<OneOfPositional>(opt)) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsOccurrenceCounter::increaseOptionOccurrenceCounter() - throwing OnlyOneChoiseIsAllowed\n";
#endif
                throw OnlyOneChoiseIsAllowed(p);  // todo: make separate checker for OneOfPositional
            }
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsOccurrenceCounter::increaseOptionOccurrenceCounter() - throwing MaxOptionOccurrenceIsExceeded\n";
#endif
            throw MaxOptionOccurrenceIsExceeded(opt);
        }
        return false;
    }

    bool canAcceptNewOccurrence(std::shared_ptr<AbstractOption> opt) {
        size_t actual_count = 0;
        if (opts_counter_.contains(opt)) {
            actual_count = opts_counter_[opt];
        }
        size_t max_count = opt->maxOccurrence();
        bool can_accept = (actual_count < max_count);
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsOccurrenceCounter::canAcceptNewOccurrence() - actual=" << actual_count << ", max=" << max_count << ", can_accept=" << can_accept << "\n";
#endif
        return can_accept;
    }

    std::map<std::shared_ptr<AbstractOption>, size_t> opts_counter_;
};

class OptionsValueStorage {
    public:
    KeyValueStorage storage;

    void clear() {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsValueStorage::clear()\n";
#endif
        storage.clear();
    }

    void initializeOptionsStorages(std::shared_ptr<AbstractOption> root) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsValueStorage::initializeOptionsStorages()\n";
#endif
        //storage.initializeOptionsStorages(root);
    }

    virtual void onDefaultValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) { /* nothing to do */}
    virtual void onImplicitValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) { /* nothing to do */}
    virtual void onValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) { /* nothing to do */}

    void setDefaultValue(std::shared_ptr<AbstractOptionWithValue> opt) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsValueStorage::setDefaultValue() - setting default value for option\n";
#endif
        std::any v = opt->baseValueSemantics().setToDefault();
        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
        onNewOptionsUnlocked(unlocked_by_value);
        storage.addValue(opt, "", v);
        onDefaultValueApplied(opt);
    }

    void setImplicitValue(std::shared_ptr<AbstractOptionWithValue> opt) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsValueStorage::setImplicitValue() - setting implicit value for option\n";
#endif
        std::any v = opt->baseValueSemantics().setToImplicit();
        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
        onNewOptionsUnlocked(unlocked_by_value);
        storage.addValue(opt, "", v);
        onImplicitValueApplied(opt);
    }

    void setOptionValue(std::shared_ptr<AbstractOptionWithValue> opt, const std::string& value) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsValueStorage::setOptionValue() - value=\"" << value << "\", tokens_count=" << mysplit(value).size() << "\n";
#endif
        bool first = true;
        std::vector<std::string> tokens = mysplit(value);
        switch (opt->nValuesRole()) {
            case AbstractOptionWithValue::NValuesRole::EXACT:
                if (opt->nValues() != tokens.size()) {
#ifdef PARSER_DEBUG
                    std::cerr << "[Parser::DEBUG] OptionsValueStorage::setOptionValue() - throwing ExpectedExactNumberOfValues\n";
#endif
                    throw ExpectedExactNumberOfValues(opt);
                }
            case AbstractOptionWithValue::NValuesRole::UPTO:
                if (tokens.size() > opt->nValues()) {
#ifdef PARSER_DEBUG
                    std::cerr << "[Parser::DEBUG] OptionsValueStorage::setOptionValue() - throwing TooManyValuesForOption\n";
#endif
                    throw TooManyValuesForOption(opt);
                }
            case AbstractOptionWithValue::NValuesRole::INFINITE:
                /* nothing to do*/
                break;
        }
        for (auto token : tokens) {
            std::any val = opt->baseValueSemantics().semanticParse(token);
            std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
            onNewOptionsUnlocked(unlocked_by_value);
            if (first) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsValueStorage::setOptionValue() - adding first value token=\"" << token << "\"\n";
#endif
                first = false;
                storage.addValue(opt, token, val);
            } else {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsValueStorage::setOptionValue() - adding additional value token=\"" << token << "\"\n";
#endif
                storage.addValueToCurrentOccurence(opt, token, val);
            }
        }
        onValueApplied(opt);
    }
    void setSelectedAlternative(std::shared_ptr<OneOfPositional> opt, size_t selected_idx) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsValueStorage::setSelectedAlternative(OneOfPositional) - selected_idx=" << selected_idx << "\n";
#endif
        storage.setSelectedAlternativeIndex(opt, selected_idx);
    }
    void setSelectedAlternative(std::shared_ptr<OneOfNamed> opt, size_t selected_idx) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsValueStorage::setSelectedAlternative(OneOfNamed) - selected_idx=" << selected_idx << "\n";
#endif
        storage.setSelectedAlternativeIndex(opt, selected_idx);
    }
    virtual void onNewOptionsUnlocked(const std::vector<std::shared_ptr<AbstractOption>>& src_options) = 0;
};

class OptionsEater : public OptionsOccurrenceCounter, public OptionsValueStorage {
   protected:
    std::vector<std::shared_ptr<AbstractOption>> remaining_options_;
    std::set<std::shared_ptr<AbstractOption>> already_joined_;
    std::shared_ptr<AbstractOption> options_;
    size_t cur_positional_option_idx_{0};

   public:

    OptionsEater(std::shared_ptr<AbstractOption> options) : options_{options} {
        // Checker checker;
        // options->accept(checker);
    }

    void clear() {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::clear() - starting clear\n";
#endif
        OptionsOccurrenceCounter::clear();
        OptionsValueStorage::clear();
        remaining_options_.clear();
        already_joined_.clear();
        cur_positional_option_idx_ = 0;
        onNewOptionsUnlocked({options_});

        initializeOptionsStorages(options_);
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::clear() - completed, remaining_options_.size()=" << remaining_options_.size() << "\n";
#endif
    }

    void eatValueIfCan(ArgGrammarParser& args, OptionMatcher& matcher, std::shared_ptr<AbstractOptionWithValue> opt) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatValueIfCan() - entering\n";
#endif
        // first, try to eat matcher.value
        if (matcher.value.has_value()) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsEater::eatValueIfCan() - using matcher value=\"" << matcher.value.value() << "\"\n";
#endif
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
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::eatValueIfCan() - got value from args=\"" << *value_opt << "\"\n";
#endif
                // TODO: if value can't be accepted then we should set option value to implicit value
                setOptionValue(opt, *value_opt);
                return;
            };
            args.ungetOption();
        }
        // Try to apply implicit value
        bool has_implicit_value = opt->baseValueSemantics().hasImplicitValue();
        if (has_implicit_value) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsEater::eatValueIfCan() - applying implicit value\n";
#endif
            setImplicitValue(opt);
            return;
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatValueIfCan() - throwing ExpectedValue\n";
#endif
        throw ExpectedValue(opt);
    }

    std::shared_ptr<AbstractPositionalOption> eatNextPositionalOption(ArgGrammarParser& args, OptionMatcher& matcher) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatNextPositionalOption() - entering, cur_positional_option_idx_=" << cur_positional_option_idx_ << ", remaining_options_.size()=" << remaining_options_.size() << "\n";
#endif
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
        // TODO: error: do not scroll to the next positional option until the current option can accept new occurrences
        if(cur_positional_option_idx_ == remaining_options_.size()) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsEater::eatNextPositionalOption() - throwing TooManyPositionalOptions (all remaining exhausted)\n";
#endif
            throw TooManyPositionalOptions("");
        }
        cur_positional_option_idx_ = find_next_positional(cur_positional_option_idx_);        
        if(cur_positional_option_idx_ == remaining_options_.size()) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsEater::eatNextPositionalOption() - throwing TooManyPositionalOptions (no positional found)\n";
#endif
            throw TooManyPositionalOptions("");
        }
        auto opt = remaining_options_[cur_positional_option_idx_];
        if (!canAcceptNewOccurrence(opt)) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsEater::eatNextPositionalOption() - current positional exhausted, advancing\n";
#endif
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
            if(auto oneOf = std::dynamic_pointer_cast<OneOfPositional>(opt)) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::eatNextPositionalOption() - matched OneOfPositional, selected_alternative=" << matcher.selected_alternative << "\n";
#endif
                setSelectedAlternative(oneOf, matcher.selected_alternative);
            }
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsEater::eatNextPositionalOption() - matched\n";
#endif
            return res;
        };
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatNextPositionalOption() - no match\n";
#endif
        return nullptr;
    }

    std::shared_ptr<AbstractOption> eatNextNamedOption(ArgGrammarParser& args, OptionMatcher& matcher) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatNextNamedOption() - entering, remaining_options_.size()=" << remaining_options_.size() << "\n";
#endif
        for (auto opt : remaining_options_) { // TODO: should we check if opt is AbstractNamedOption?
            opt->accept(matcher);
            if (matcher.match) {
                if(auto oneOf = std::dynamic_pointer_cast<OneOfNamed>(opt)) {
                    setSelectedAlternative(oneOf, matcher.selected_alternative);
                }
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::eatNextNamedOption() - matched\n";
#endif
                return opt;
            };
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatNextNamedOption() - no match found\n";
#endif
        return nullptr;
    }

    std::shared_ptr<AbstractOption> eatNextOption(ArgGrammarParser& args, OptionMatcher& matcher) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatNextOption() - entering\n";
#endif
        args.getNextOption();
        // If DOUBLE_DASH then all next options will be treated as positionals
        if (args.current_result.token_type == ArgGrammarParser::TokenTypes::DOUBLE_DASH) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] OptionsEater::eatNextOption() - DOUBLE_DASH detected, setting positional-only mode\n";
#endif
            matcher.setPositionalOnlyFlag(true);
            // TODO: double dash can be used to interrupt value list for the current positional option
            //cur_positional_option_idx_++;
            return nullptr;
        }
        std::shared_ptr<AbstractOption> res = nullptr;
        switch (args.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::eatNextOption() - processing named option, token=\"" << args.getRawOptionName() << "\"\n";
#endif
                res = eatNextNamedOption(args, matcher);
                if (res == 0) {
#ifdef PARSER_DEBUG
                    std::cerr << "[Parser::DEBUG] OptionsEater::eatNextOption() - throwing UnknownNamedOption\n";
#endif
                    throw UnknownNamedOption("");
                }
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
            case ArgGrammarParser::TokenTypes::VALUE:
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::eatNextOption() - processing positional option, value=\"" << args.current_result.value << "\"\n";
#endif
                res = eatNextPositionalOption(args, matcher);
                if (res == 0) {
#ifdef PARSER_DEBUG
                    std::cerr << "[Parser::DEBUG] OptionsEater::eatNextOption() - throwing UnexpectedValueForPositionalOption\n";
#endif
                    throw UnexpectedValueForPositionalOption(args.current_result.value);
                }
                break;
        }
        if (res) {
            return res;
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::eatNextOption() - throwing UnknownNamedOption (fallback)\n";
#endif
        throw UnknownNamedOption(args.getRawOptionName());
        return nullptr;
    }

    void parseNext(ArgGrammarParser& args) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::parseNext() - entering\n";
#endif
        OptionMatcher matcher(args);
        auto opt = eatNextOption(args, matcher);
        if (opt) {
            if (!already_joined_.contains(opt)) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::parseNext() - new option matched, processing unlocks\n";
#endif
                onNewOptionsUnlocked(matcher.unlocks);
                already_joined_.insert(opt);
            }

            if (auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                eatValueIfCan(args, matcher, p);
                ensureValueListIsCompleted(p);
            };

            if (increaseOptionOccurrenceCounter(opt)) { // raises exception if option is exausted
                // if the number of occurences of this option is exausted
                // then this option can be removed from the list of remaining
                // options. But in the present code we check options counters to check 
                // if the option cant be used any more. So we dont remove used options from the 
                // list of remaining options since.

                // Ideally we should check if option is exhaused after the name of the option
                // is parsed (i.e. in function eatNextPositionalOption, eatNextNamedOption) and
                // before parsing the option value
            }
            
            // TODO: callback newOptionOccurrenceFinished

            if (matcher.value.has_value()) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::parseNext() - throwing OptionDoesntAcceptValue (unconsumed matcher value)\n";
#endif
                throw OptionDoesntAcceptValue();
            }
        };
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::parseNext() - completed\n";
#endif
    }

    void parse(ArgGrammarParser args) { 
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::parse() - starting parse\n";
#endif
        clear();
        //OptionMatcher matcher(args); 
        while (!args.eof()) {
            parseNext(args);
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::parse() - parse completed\n";
#endif
    }

   protected:
    void onNewOptionsUnlocked(const std::vector<std::shared_ptr<AbstractOption>>& src_options) override {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::onNewOptionsUnlocked() - unlocking " << src_options.size() << " new options\n";
#endif
        for (auto it : src_options) {
            if (auto p = std::dynamic_pointer_cast<OptionsGroup>(it)) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::onNewOptionsUnlocked() - expanding OptionsGroup\n";
#endif
                // TODO: why separate case for OptionsGroup
                onNewOptionsUnlocked(p->unlocks());  // todo: avoid copying of a vector
            } else {
                remaining_options_.push_back(it);
            }
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::onNewOptionsUnlocked() - remaining_options_.size() now = " << remaining_options_.size() << "\n";
#endif
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
#ifdef PARSER_DEBUG
                    std::cerr << "[Parser::DEBUG] OptionsEater::ensureValueListIsCompleted() - throwing ExpectedValue\n";
#endif
                    throw ExpectedValue(opt);
                }
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::ensureValueListIsCompleted() - throwing TooFewValuesForOption, actual=" << actual << ", expected=" << expected << "\n";
#endif
                throw TooFewValuesForOption(opt);  // todo print message
            }
        } else {
            if (actual == 0) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] OptionsEater::ensureValueListIsCompleted() - throwing ExpectedValue (no EXACT, but actual=0)\n";
#endif
                throw ExpectedValue(opt);
            }
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] OptionsEater::ensureValueListIsCompleted() - OK, actual=" << actual << "\n";
#endif
    }
};

class Parser : public OptionsEater {
   public:
    Parser(std::shared_ptr<AbstractOption> options) : OptionsEater{options} {}

    bool parse(ArgGrammarParser args) {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] Parser::parse() - starting parse\n";
#endif
        OptionsEater::parse(args);
        applyDefaultValues();
        checkUnusedRequiredOptions();
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] Parser::parse() - parse completed successfully\n";
#endif
        return true;
    }

   private:
    std::set<std::shared_ptr<AbstractOptionWithValue>> opts_with_default_value_;
    bool isOptionSpecifiedOrImplied(std::shared_ptr<AbstractOption> opt) {
        if(optionEncountered(opt) > 0) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] Parser::isOptionSpecifiedOrImplied() - option was encountered, returning true\n";
#endif
            return true;
        }
        if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
            if(opts_with_default_value_.contains(p)) {
#ifdef PARSER_DEBUG
                std::cerr << "[Parser::DEBUG] Parser::isOptionSpecifiedOrImplied() - opt has default value, returning true\n";
#endif
                return true;
            }
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] Parser::isOptionSpecifiedOrImplied() - returning false\n";
#endif
        return false;
    };
    void checkUnusedRequiredOptions() {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] Parser::checkUnusedRequiredOptions() - checking " << remaining_options_.size() << " remaining options\n";
#endif
        // check that all required options are used
        bool too_few_pos_opts{false};
        bool required_opt_is_not_set{false};
        size_t missing_count{0};
        for (auto opt : remaining_options_) {
            if (opt->required() && !isOptionSpecifiedOrImplied(opt)) {
                auto q = std::dynamic_pointer_cast<AbstractPositionalOption>(opt);
                if (!q) {
                    too_few_pos_opts = true;
                }
                required_opt_is_not_set = true;
                missing_count++;
            }
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] Parser::checkUnusedRequiredOptions() - missing_count=" << missing_count << ", too_few_pos_opts=" << too_few_pos_opts << "\n";
#endif
        if(required_opt_is_not_set) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] Parser::checkUnusedRequiredOptions() - throwing RequiredOptionIsNotSet\n";
#endif
            throw RequiredOptionIsNotSet(nullptr);
        }
        if(too_few_pos_opts) {
#ifdef PARSER_DEBUG
            std::cerr << "[Parser::DEBUG] Parser::checkUnusedRequiredOptions() - throwing TooFewPositionalOptions\n";
#endif
            /// TODO print how many pos options are expected
            throw TooFewPositionalOptions();
        }
    }

    void applyDefaultValues() {
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] Parser::applyDefaultValues() - processing " << remaining_options_.size() << " remaining options\n";
#endif
        // applies default values for options that were not encountered
        for (auto opt : remaining_options_) {
            if (optionEncountered(opt) > 0) {
                continue;
            }
            if (auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                if (p->baseValueSemantics().hasDefaultValue()) {
#ifdef PARSER_DEBUG
                    std::cerr << "[Parser::DEBUG] Parser::applyDefaultValues() - applying default value\n";
#endif
                    setDefaultValue(p);
                    opts_with_default_value_.insert(p);
                    // todo: if default value unlocks some options these options should be proccessed
                } else {
                    if (opt->required()) {
#ifdef PARSER_DEBUG
                        std::cerr << "[Parser::DEBUG] Parser::applyDefaultValues() - throwing RequiredOptionIsNotSet\n";
#endif
                        throw RequiredOptionIsNotSet(opt);
                    }
                }
            };
        }
#ifdef PARSER_DEBUG
        std::cerr << "[Parser::DEBUG] Parser::applyDefaultValues() - done, " << opts_with_default_value_.size() << " defaults applied\n";
#endif
    }
};

#endif