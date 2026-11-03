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
#include <boost/log/trivial.hpp>
#include <vector>

#include "Exceptions.h"
#include "Option.h"
#include "OptionMatcher.h"
#include "Printer.h"
#include "ValueSemantics.h"
#include "ValueStorage.h"

namespace program_options_heavy {

class OptionsOccurrenceCounter {
    public:

    void clear() {
        //TODO: increase debug level for commented debug messages and uncomment them
        //BOOST_LOG_TRIVIAL(debug) << "OptionsOccurrenceCounter::clear(): clearing counter map (size=" << opts_counter_.size() << ")";
        opts_counter_.clear();
    }
    /// how many times the specified option has been encountered in the already
    /// parsed context
    size_t optionEncountered(std::shared_ptr<AbstractOption> opt) {
        size_t counter = 0;
        if (opts_counter_.contains(opt)) {
            counter = opts_counter_[opt];
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionsOccurrenceCounter::optionEncountered(): \"" << ShortPrinter::optionDisplayName(opt) << "\", counter=" << counter << "";
        return counter;
    }
    bool increaseOptionOccurrenceCounter(std::shared_ptr<AbstractOption> opt) {        
        size_t counter = ++opts_counter_[opt];
        BOOST_LOG_TRIVIAL(debug) << "OptionsOccurrenceCounter::increaseOptionOccurrenceCounter(): \"" << ShortPrinter::optionDisplayName(opt) << "\", counter=" << counter << ", max=" << opt->maxOccurrence() << "";
        if (counter == opt->maxOccurrence()) {
            return true;
        }
        if (counter > opt->maxOccurrence()) {
            if (auto p = std::dynamic_pointer_cast<OneOfPositional>(opt)) {
                BOOST_LOG_TRIVIAL(debug) << "OptionsOccurrenceCounter::increaseOptionOccurrenceCounter(): : opt\"" << ShortPrinter::optionDisplayName(p) << "\", throwing OnlyOneChoiseIsAllowed";
                throw OnlyOneChoiseIsAllowed(p);  // todo: make separate checker for OneOfPositional
            }
            BOOST_LOG_TRIVIAL(debug) << "OptionsOccurrenceCounter::increaseOptionOccurrenceCounter(): \"" << ShortPrinter::optionDisplayName(opt) << "\", throwing MaxOptionOccurrenceIsExceeded";
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
        BOOST_LOG_TRIVIAL(debug) << "OptionsOccurrenceCounter::canAcceptNewOccurrence(): \"" << ShortPrinter::optionDisplayName(opt) << "\", actual=" << actual_count << ", max=" << max_count << ", can_accept=" << can_accept << "";
        return can_accept;
    }

    std::map<std::shared_ptr<AbstractOption>, size_t> opts_counter_;
};

class OptionsValueStorage {
    public:
    KeyValueStorage storage;

    void clear() {
        //BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::clear()";
        storage.clear();
    }

    void initializeOptionsStorages(std::shared_ptr<AbstractOption> root) {
        //BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::initializeOptionsStorages()";
        //storage.initializeOptionsStorages(root);
    }

    virtual void onDefaultValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) { /* nothing to do */}
    virtual void onImplicitValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) { /* nothing to do */}
    virtual void onValueApplied(std::shared_ptr<AbstractOptionWithValue> opt) { /* nothing to do */}

    void setDefaultValue(std::shared_ptr<AbstractOptionWithValue> opt) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setDefaultValue(): \"" << ShortPrinter::optionDisplayName(opt) << "\", setting default value for option";
        std::any v = opt->baseValueSemantics().setToDefault();
        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
        onNewOptionsUnlocked(unlocked_by_value);
        storage.addValue(opt, "", v);
        onDefaultValueApplied(opt);
    }

    void setImplicitValue(std::shared_ptr<AbstractOptionWithValue> opt) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setImplicitValue(): \"" << ShortPrinter::optionDisplayName(opt) << "\", setting implicit value for option";
        std::any v = opt->baseValueSemantics().setToImplicit();
        std::vector<std::shared_ptr<AbstractOption>> unlocked_by_value{opt->baseValueSemantics().getUnlocks()};
        onNewOptionsUnlocked(unlocked_by_value);
        storage.addValue(opt, "", v);
        onImplicitValueApplied(opt);
    }

    void setOptionValue(std::shared_ptr<AbstractOptionWithValue> opt, const std::string& value) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setOptionValue(): \"" << ShortPrinter::optionDisplayName(opt) << "\", value=\"" << value << "\", tokens_count=" << mysplit(value).size() << "";
        bool first = true;
        std::vector<std::string> tokens = mysplit(value);
        switch (opt->nValuesRole()) {
            case AbstractOptionWithValue::NValuesRole::EXACT:
                if (opt->nValues() != tokens.size()) {
                    BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setOptionValue(): \"" << ShortPrinter::optionDisplayName(opt) << "\", throwing ExpectedExactNumberOfValues";
                    throw ExpectedExactNumberOfValues(opt);
                }
            case AbstractOptionWithValue::NValuesRole::UPTO:
                if (tokens.size() > opt->nValues()) {
                    BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setOptionValue(): \"" << ShortPrinter::optionDisplayName(opt) << "\", throwing TooManyValuesForOption";
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
                BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setOptionValue(): \"" << ShortPrinter::optionDisplayName(opt) << "\", adding first value token=\"" << token << "\"";
                first = false;
                storage.addValue(opt, token, val);
            } else {
                BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setOptionValue(): \"" << ShortPrinter::optionDisplayName(opt) << "\", adding additional value token=\"" << token << "\"";
                storage.addValueToCurrentOccurence(opt, token, val);
            }
        }
        onValueApplied(opt);
    }
    void setSelectedAlternative(std::shared_ptr<OneOfPositional> opt, size_t selected_idx) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setSelectedAlternative(OneOfPositional): \"" << ShortPrinter::optionDisplayName(opt) << "\", selected_idx=" << selected_idx << "";
        storage.setSelectedAlternativeIndex(opt, selected_idx);
    }
    void setSelectedAlternative(std::shared_ptr<OneOfNamed> opt, size_t selected_idx) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsValueStorage::setSelectedAlternative(OneOfNamed): \"" << ShortPrinter::optionDisplayName(opt) << "\", selected_idx=" << selected_idx << "";
        storage.setSelectedAlternativeIndex(opt, selected_idx);
    }
    virtual void onNewOptionsUnlocked(const std::vector<std::shared_ptr<AbstractOption>>& src_options, size_t log_nested_level = 0) = 0;
};

class OptionsEater : public OptionsOccurrenceCounter, public OptionsValueStorage {
   protected:
    std::vector<std::shared_ptr<AbstractOption>> remaining_options_;
    std::set<std::shared_ptr<AbstractOption>> already_joined_;
    std::shared_ptr<AbstractOption> options_;
    size_t cur_positional_option_idx_{0};

    bool last_processed_is_positional_{false}; // used to determine how DOUBLE DASH behave. If true, DOUBLE DASH finishes current positional option

   public:

    OptionsEater(std::shared_ptr<AbstractOption> options): options_{options} {
        // Checker checker;
        // options->accept(checker);
    }

    void clear() {
        //BOOST_LOG_TRIVIAL(debug) << "OptionsEater::clear(): starting clear";
        OptionsOccurrenceCounter::clear();
        OptionsValueStorage::clear();
        remaining_options_.clear();
        already_joined_.clear();
        cur_positional_option_idx_ = 0;
        onNewOptionsUnlocked({options_});

        initializeOptionsStorages(options_);
        //BOOST_LOG_TRIVIAL(debug) << "OptionsEater::clear(): completed";
    }

    void eatValueIfCan(ArgGrammarParser& args, OptionMatcher& matcher, std::shared_ptr<AbstractOptionWithValue> opt) {
        //BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatValueIfCan(): \"" << ShortPrinter::optionDisplayName(opt) << "\", entering";
        // first, try to eat matcher.value
        if (matcher.value.has_value()) {
            BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatValueIfCan(): \"" << ShortPrinter::optionDisplayName(opt) << "\", using matcher value=\"" << matcher.value.value() << "\"";
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
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatValueIfCan(): \"" << ShortPrinter::optionDisplayName(opt) << "\", got value from args=\"" << *value_opt << "\"";
                // TODO: if value can't be accepted then we should set option value to implicit value
                setOptionValue(opt, *value_opt);
                return;
            };
            args.ungetOption();
        }
        // Try to apply implicit value
        bool has_implicit_value = opt->baseValueSemantics().hasImplicitValue();
        if (has_implicit_value) {
            BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatValueIfCan(): \"" << ShortPrinter::optionDisplayName(opt) << "\", applying implicit value";
            setImplicitValue(opt);
            return;
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatValueIfCan(): \"" << ShortPrinter::optionDisplayName(opt) << "\", throwing ExpectedValue";
        throw ExpectedValue(opt);
    }

    std::shared_ptr<AbstractPositionalOption> eatNextPositionalOption(ArgGrammarParser& args, OptionMatcher& matcher) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextPositionalOption(): entering, cur_positional_option_idx_=" << cur_positional_option_idx_ << ", remaining_options_.size()=" << remaining_options_.size() << "";
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
            BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextPositionalOption(): throwing TooManyPositionalOptions (all remaining exhausted)";
            throw TooManyPositionalOptions("");
        }
        cur_positional_option_idx_ = find_next_positional(cur_positional_option_idx_);        
        if(cur_positional_option_idx_ == remaining_options_.size()) {
            BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextPositionalOption(): throwing TooManyPositionalOptions (no positional found)";
            throw TooManyPositionalOptions("");
        }
        auto opt = remaining_options_[cur_positional_option_idx_];
        if (!canAcceptNewOccurrence(opt)) {
            BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextPositionalOption(): \"" << ShortPrinter::optionDisplayName(opt) << "\", current positional exhausted, advancing";
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
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextPositionalOption(): \"" << ShortPrinter::optionDisplayName(oneOf) << "\", matched OneOfPositional, selected_alternative=" << matcher.selected_alternative << "";
                setSelectedAlternative(oneOf, matcher.selected_alternative);
            }
            BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextPositionalOption(): \"" << ShortPrinter::optionDisplayName(res) << "\", matched";
            return res;
        };
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextPositionalOption(): \"" << ShortPrinter::optionDisplayName(opt) << "\", no match";
        return nullptr;
    }

    std::shared_ptr<AbstractOption> eatNextNamedOption(ArgGrammarParser& args, OptionMatcher& matcher) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextNamedOption(): entering, remaining_options_.size()=" << remaining_options_.size() << "";
        for (auto opt : remaining_options_) { // TODO: should we check if opt is AbstractNamedOption?
            opt->accept(matcher);
            if (matcher.match) {
                if(auto one_of = std::dynamic_pointer_cast<OneOfNamed>(opt)) {
                    setSelectedAlternative(one_of, matcher.selected_alternative);
                }
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextNamedOption(): \"" << ShortPrinter::optionDisplayName(opt) << "\", matched";
                return opt;
            };
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextNamedOption(): no match found";
        return nullptr;
    }

    std::shared_ptr<AbstractOption> eatNextOption(ArgGrammarParser& args, OptionMatcher& matcher) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): entering";
        args.getNextOption();
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): parsing \""<<args.arg(args.current_result.index)<<"\"";
        // If DOUBLE_DASH then all next options will be treated as positionals
        if (args.current_result.token_type == ArgGrammarParser::TokenTypes::DOUBLE_DASH) {            
            if(last_processed_is_positional_)  {
                //double dash can be used to interrupt value list for the current positional option
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): DOUBLE_DASH detected, finish current positional option and ready to parse next positional";
                cur_positional_option_idx_++;
            } else {
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): DOUBLE_DASH detected, setting positional-only mode";
                matcher.setPositionalOnlyFlag(true);
            }
            return nullptr;
        }
        std::shared_ptr<AbstractOption> res = nullptr;
        switch (args.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): processing named option, token=\"" << args.getRawOptionName() << "\"";
                res = eatNextNamedOption(args, matcher);
                if (res == 0) {
                    BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): throwing UnknownNamedOption";
                    throw UnknownNamedOption("");
                }
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
            case ArgGrammarParser::TokenTypes::VALUE:
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): processing positional option, value=\"" << args.current_result.value << "\"";
                res = eatNextPositionalOption(args, matcher);
                last_processed_is_positional_ = true;
                if (res == 0) {
                    BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): throwing UnexpectedValueForPositionalOption";
                    throw UnexpectedValueForPositionalOption(args.current_result.value);
                }
                break;
        }
        if (res) {
            return res;
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::eatNextOption(): throwing UnknownNamedOption (fallback)";
        throw UnknownNamedOption(args.getRawOptionName());
        return nullptr;
    }

    void parseNext(ArgGrammarParser& args) {
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::parseNext(): entering";
        OptionMatcher matcher(args);
        auto opt = eatNextOption(args, matcher);
        if (opt) {
            if (!already_joined_.contains(opt)) {
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::parseNext(): \"" << ShortPrinter::optionDisplayName(opt) << "\", new option matched, processing unlocks";
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
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::parseNext(): throwing OptionDoesntAcceptValue (unconsumed matcher value)";
                throw OptionDoesntAcceptValue();
            }
        };
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::parseNext(): completed";
    }

    void parse(ArgGrammarParser args) { 
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::parse(): starting parse";
        clear();
        //OptionMatcher matcher(args); 
        while (!args.eof()) {
            parseNext(args);
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::parse(): parse completed";
    }

   protected:
    void onNewOptionsUnlocked(const std::vector<std::shared_ptr<AbstractOption>>& src_options, size_t log_nested_level = 0) override {
        std::string str(log_nested_level, ' ');
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::onNewOptionsUnlocked(): "<<str<<" unlocking " << src_options.size() << " new options";
        for (auto it : src_options) {
            if (auto p = std::dynamic_pointer_cast<OptionsGroup>(it)) {
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::onNewOptionsUnlocked(): "<<str<<" expanding OptionsGroup:  \"" << ShortPrinter::optionDisplayName(p) << "\"";
                // TODO: why separate case for OptionsGroup
                onNewOptionsUnlocked(p->unlocks(), log_nested_level + 1);// todo: avoid copying of a vector
            } else {
                // TODO: if it is a positional option than print the place at which it is added
                // TODO: store separately positional and named options
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::onNewOptionsUnlocked(): "<<str<<" adding displayName = \'" << ShortPrinter::optionDisplayName(it) << "\"";
                remaining_options_.push_back(it);
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::onNewOptionsUnlocked(): "<<str<<" finished, remaining_options_.size() now = " << remaining_options_.size() << "";
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
                    BOOST_LOG_TRIVIAL(debug) << "OptionsEater::ensureValueListIsCompleted(): \"" << ShortPrinter::optionDisplayName(opt) << "\", throwing ExpectedValue";
                    throw ExpectedValue(opt);
                }
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::ensureValueListIsCompleted(): \"" << ShortPrinter::optionDisplayName(opt) << "\", throwing TooFewValuesForOption, actual=" << actual << ", expected=" << expected << "";
                throw TooFewValuesForOption(opt);  // todo print message
            }
        } else {
            if (actual == 0) {
                BOOST_LOG_TRIVIAL(debug) << "OptionsEater::ensureValueListIsCompleted(): \"" << ShortPrinter::optionDisplayName(opt) << "\", throwing ExpectedValue (no EXACT, but actual=0)";
                throw ExpectedValue(opt);
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionsEater::ensureValueListIsCompleted(): \"" << ShortPrinter::optionDisplayName(opt) << "\", OK, actual=" << actual << "";
    }
};

class Parser : public OptionsEater {
   public:
    Parser(std::shared_ptr<AbstractOption> options): OptionsEater{options} {}

    bool parse(ArgGrammarParser args) {
        BOOST_LOG_TRIVIAL(debug) << "Parser::parse(): starting parse";
        OptionsEater::parse(args);
        applyDefaultValues();
        checkUnusedRequiredOptions();
        BOOST_LOG_TRIVIAL(debug) << "Parser::parse(): parse completed successfully";
        return true;
    }

   private:
    std::set<std::shared_ptr<AbstractOptionWithValue>> opts_with_default_value_;
    bool isOptionSpecifiedOrImplied(std::shared_ptr<AbstractOption> opt) {
        if(optionEncountered(opt) > 0) {
            BOOST_LOG_TRIVIAL(debug) << "Parser::isOptionSpecifiedOrImplied(): \"" << ShortPrinter::optionDisplayName(opt) << "\", option was encountered, returning true";
            return true;
        }
        if(auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
            if(opts_with_default_value_.contains(p)) {
                BOOST_LOG_TRIVIAL(debug) << "Parser::isOptionSpecifiedOrImplied(): \"" << ShortPrinter::optionDisplayName(opt) << "\", opt has default value, returning true";
                return true;
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "Parser::isOptionSpecifiedOrImplied(): \"" << ShortPrinter::optionDisplayName(opt) << "\", returning false";
        return false;
    };
    void checkUnusedRequiredOptions() {
        BOOST_LOG_TRIVIAL(debug) << "Parser::checkUnusedRequiredOptions(): checking " << remaining_options_.size() << " remaining options";
        // check that all required options are used
        bool too_few_pos_opts{false};
        bool required_opt_is_not_set{false};
        size_t missing_count{0};
        for (auto opt : remaining_options_) {
            if (opt->isRequired() && !isOptionSpecifiedOrImplied(opt)) {
                auto q = std::dynamic_pointer_cast<AbstractPositionalOption>(opt);
                if (!q) {
                    too_few_pos_opts = true;
                }
                required_opt_is_not_set = true;
                missing_count++;
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "Parser::checkUnusedRequiredOptions(): missing_count=" << missing_count << ", too_few_pos_opts=" << too_few_pos_opts << "";
        if(required_opt_is_not_set) {
            BOOST_LOG_TRIVIAL(debug) << "Parser::checkUnusedRequiredOptions(): throwing RequiredOptionIsNotSet";
            throw RequiredOptionIsNotSet(nullptr);
        }
        if(too_few_pos_opts) {
            BOOST_LOG_TRIVIAL(debug) << "Parser::checkUnusedRequiredOptions(): throwing TooFewPositionalOptions";
            /// TODO print how many pos options are expected
            throw TooFewPositionalOptions();
        }
    }

    void applyDefaultValues() {
        BOOST_LOG_TRIVIAL(debug) << "Parser::applyDefaultValues(): processing " << remaining_options_.size() << " remaining options";
        // applies default values for options that were not encountered
        for (auto opt : remaining_options_) {
            if (optionEncountered(opt) > 0) {
                continue;
            }
            if (auto p = std::dynamic_pointer_cast<AbstractOptionWithValue>(opt)) {
                if (p->baseValueSemantics().hasDefaultValue()) {
                    BOOST_LOG_TRIVIAL(debug) << "Parser::applyDefaultValues(): opt= \"" << ShortPrinter::optionDisplayName(p) << "\", applying default value";
                    setDefaultValue(p);
                    opts_with_default_value_.insert(p);
                    // todo: if default value unlocks some options these options should be proccessed
                } else {
                    if (opt->isRequired()) {
                        BOOST_LOG_TRIVIAL(debug) << "Parser::applyDefaultValues(): opt= \"" << ShortPrinter::optionDisplayName(p) << ", throwing RequiredOptionIsNotSet";
                        throw RequiredOptionIsNotSet(opt);
                    }
                }
            };
        }
        BOOST_LOG_TRIVIAL(debug) << "Parser::applyDefaultValues(): done, " << opts_with_default_value_.size() << " defaults applied";
    }
};

} /* namespace program_options_heavy */
#endif