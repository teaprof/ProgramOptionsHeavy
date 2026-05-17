#ifndef OPTION_MATCHER_H
#define OPTION_MATCHER_H

#include <Lexer/Lexer.h>

#include "Option.h"


class OptionMatcher : public AbstractOptionVisitor {
   private:
    ArgGrammarParser& grammar_parser_;

   public:
    bool match;
    std::optional<std::string> value;
    std::vector<std::shared_ptr<AbstractOption>> unlocks;
    OptionMatcher(ArgGrammarParser& args) : grammar_parser_{args} {}

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

#endif