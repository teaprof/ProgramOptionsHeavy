/* Prints the current_result of ArgGrammarParser depending on its token_type.
*/
#ifndef TOKEN_PRINTER_H
#define TOKEN_PRINTER_H

#include <Lexer/Lexer.h>

#include <iostream>

namespace program_options_heavy {

/// Prints the current_result of ArgGrammarParser to an output stream.
/// The printed content depends on the value of current_result.token_type.
class TokenPrinter {
   public:
    /// Prints the given token result to the given stream
    static std::ostream& print(std::ostream& out, const ArgGrammarParser::Result& result) {
        switch (result.token_type) {
            case ArgGrammarParser::TokenTypes::LONG_OPTION:
                out << "--" << result.long_option_name;
                break;
            case ArgGrammarParser::TokenTypes::LONG_OPTION_EQ_VALUE:
                out << "--" << result.long_option_name << "=" << result.value;
                break;
            case ArgGrammarParser::TokenTypes::SHORT_OPTION:
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_WITHOUT_VALUE:
                out << "-" << result.short_option_name;
                break;
            case ArgGrammarParser::TokenTypes::SHORT_OPTION_EQ_VALUE:
                out << "-" << result.short_option_name << "=" << result.value;
                break;
            case ArgGrammarParser::TokenTypes::DOUBLE_DASH:
                out << "--";
                break;
            case ArgGrammarParser::TokenTypes::VALUE:
                out << result.value;
                break;
        }
        return out;
    }
    /// Prints the given token result to std::cout
    static void print(const ArgGrammarParser::Result& result) { print(std::cout, result) << std::endl; }
    /// Prints the current_result of the given parser to std::cout
    static void print(const ArgGrammarParser& parser) { print(parser.current_result); }
};

} /* namespace program_options_heavy */

/// Prints the given token result to the given stream, respecting its token_type.
/// Makes the printer usable in log statements:
/// BOOST_LOG_TRIVIAL(debug) << "token=\"" << parser.current_result << "\"";
inline std::ostream& operator<<(std::ostream& out, const ArgGrammarParser::Result& result) {
    return program_options_heavy::TokenPrinter::print(out, result);
}

#endif