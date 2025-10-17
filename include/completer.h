#ifndef __COMPLETER_H__
#define __COMPLETER_H__

#include <Parsers/SubcommandsParser.h>

#include <string>
#include <optional>
#include <vector>
#include <string_view>
#include <ranges>
#include <cstdlib>

namespace program_options_heavy {

inline std::vector<std::string> complete(const std::string &program, const std::string &curargument,
                                         const std::string &prevargument)
{
    return {"run", "collect"};
}


class Completer {
    public:
        Completer(std::shared_ptr<SubcommandsParser>& parser) : parser_{parser} {}

    private:
        std::shared_ptr<SubcommandsParser> parser_;

        std::vector<std::string> splitCompletionLine(const std::optional<std::string>& line) {
            if(!line.has_value())
                return {};
            std::vector<std::string> res;
            std::string_view source = line.value();
            size_t start = 0;
            size_t finish = source.find(' ', start+1);
            while(finish != std::string::npos) {
                std::string substr{source.substr(start, finish - start)};
                substr.
                

            }
            return res;
        }

        std::optional<std::string> getCompletionLine() {
            if(const char* cstr = getenv("COMP_LINE")) {
                return std::string(cstr);
            } else {
                return std::nullopt;
            }
        }

}

} /* program_options_heavy */

#endif