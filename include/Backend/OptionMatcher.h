#ifndef OPTION_MATCHER_H
#define OPTION_MATCHER_H

#include <Backend/Printer.h>
#include <Lexer/Lexer.h>
#include <Lexer/TokenPrinter.h>

#include <boost/log/trivial.hpp>

#include "Option.h"

namespace program_options_heavy {

class OptionMatcher : public AbstractOptionVisitor {
   private:
    ArgGrammarParser& grammar_parser_;

   public:
    bool match;
    std::optional<std::string> value;
    std::vector<std::shared_ptr<AbstractOption>> unlocks;
    size_t selected_alternative{0};
    OptionMatcher(ArgGrammarParser& args) : grammar_parser_{args} {}

    void visit(std::shared_ptr<AbstractOption> opt) override {
        // should not never visit abstract object
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractOption): token=\"" << grammar_parser_.current_result << "\", visiting abstract option (should not happen)";
        assert(false);
    }
    void visit(std::shared_ptr<AbstractPositionalOptionWithValue> opt) override {
        //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractPositionalOptionWithValue): displayName=\"" << opt->displayName() << "\", entering";
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
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractPositionalOptionWithValue): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", MATCHED, value=" << (value.has_value() ? value.value() : std::string("<empty>"));
    }
    void visit(std::shared_ptr<LiteralString> opt) override {
        //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(LiteralString): displayName=\"" << opt->displayName() << "\", literal=\"" << opt->str() << "\", entering";
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
            BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(LiteralString): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", MATCHED";
        } else {
            BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(LiteralString): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", no match";
        };
    }
    void visit(std::shared_ptr<NamedOption> opt) override {
        //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(NamedOption): displayName=\"" << opt->displayName() << "\", entering";
        unlocks.clear();
        match = false;
        switch (grammar_parser_.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
                match = opt->longName().has_value() && opt->longName().value() == grammar_parser_.current_result.long_option_name;
                //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(NamedOption): displayName=\"" << opt->displayName() << "\", long option case, match=" << match;
                break;
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
                match =
                    opt->shortName().has_value() && opt->shortName().value() == grammar_parser_.current_result.short_option_name;
                //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(NamedOption): displayName=\"" << opt->displayName() << "\", short option case, match=" << match;
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
            case ArgGrammarParser::TokenTypes::VALUE:
                break;
        }
        if (match) {            
            value = std::nullopt;
            unlocks = opt->unlocks();  // todo: avoid copying of a vector
            BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(NamedOption): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", MATCHED";
        } else {
            BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(NamedOption): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", no match";
        }
    }
    void visit(std::shared_ptr<AbstractNamedOptionWithValue> opt) override {
        //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractNamedOptionWithValue): displayName=\"" << opt->displayName() << "\", entering";
        unlocks.clear();
        match = false;
        switch (grammar_parser_.current_result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
                match = opt->longName().has_value() && opt->longName().value() == grammar_parser_.current_result.long_option_name;
                //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractNamedOptionWithValue): displayName=\"" << opt->displayName() << "\", long option case, match=" << match;
                break;
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
                match =
                    opt->shortName().has_value() && opt->shortName().value() == grammar_parser_.current_result.short_option_name;
                //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractNamedOptionWithValue): displayName=\"" << opt->displayName() << "\", short option case, match=" << match;
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
            case ArgGrammarParser::TokenTypes::VALUE:
                /* nothing to do*/;
        }
        if (match) {
            value = grammar_parser_.getValueOpt();
            unlocks = opt->unlocks();  // todo: avoid copying of a vector
            BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractNamedOptionWithValue): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", MATCHED, value=" << (value.has_value() ? value.value() : std::string("<empty>"));
        } else {
            BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractNamedOptionWithValue): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", no match";
        }
    }
    void visit(std::shared_ptr<AbstractPositionalOption> opt) override {
        // should not never visit abstract object
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(AbstractPositionalOption): token=\"" << grammar_parser_.current_result << "\", visiting abstract positional option (should not happen)";
        assert(false);
    }
    void visit(std::shared_ptr<OptionsGroup> opt) override {
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OptionsGroup): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", entering (unreachable)";
        assert(false);
        match = true;
        value = std::nullopt;
        unlocks = opt->unlocks();  // todo: avoid copying of a vector
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OptionsGroup): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", forced match (assert disabled)";
    }
    void visit(std::shared_ptr<OneOfPositional> opt) override {
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfPositional) \"" << opt->displayName() << "\", alternatives=" << Printer::alternativesToString(opt) << ", token=\"" << grammar_parser_.current_result << "\", entering";
        match = false;
        unlocks.clear();
        selected_alternative = 0;
        for (size_t n = 0; n < opt->alternativesSize(); n++) {
            auto alt = opt->alternative(n);
            //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfPositional): displayName=\"" << opt->displayName() << "\", alternatives=" << opt->alternativesSize() << ", entering";
            alt->accept(*this);
            if (match) {
                selected_alternative = n;
                value = std::nullopt;
                unlocks = opt->alternative(n)->unlocks();  // todo: avoid copying of a vector
                BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfPositional): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", MATCHED alternative " << n;
                return;
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfPositional): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", no match";
    }
    void visit(std::shared_ptr<OneOfNamed> opt) override {
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfNamed): displayName=\"" << opt->displayName() << ", alternatives=" << Printer::alternativesToString(opt) << ", token=\"" << grammar_parser_.current_result << "\", entering";
        match = false;
        unlocks.clear();
        selected_alternative = 0;
        for (size_t n = 0; n < opt->alternativesSize(); n++) {
            auto alt = opt->alternative(n);
            //BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfNamed) checking alternative " << n << " : "<< alt->displayName();
            alt->accept(*this);
            if (match) {
                value = std::nullopt;
                selected_alternative = n;
                unlocks = opt->alternative(n)->unlocks();  // todo: avoid copying of a vector
                BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfNamed): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", MATCHED alternative " << n;
                return;
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "OptionMatcher::visit(OneOfNamed): token=\"" << grammar_parser_.current_result << "\", displayName=\"" << opt->displayName() << "\", no alternative MATCHED";
    }

    void setPositionalOnlyFlag(bool value) { grammar_parser_.match_only_positional = value; }
};

} /* namespace program_options_heavy */

#endif