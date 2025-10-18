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

        std::vector<std::string> complete() {
            auto completion_line = getCompletionLine();
            auto words = splitCompletionLine(completion_line);
            return complete(words);
        }

    private:
        std::shared_ptr<SubcommandsParser> parser_;

        struct Subcommand {
            std::string name;
            std::vector<std::string> params;
        };        

        std::vector<std::string> complete(std::vector<std::string_view> words) {
            auto all_variants = getAllVariants();
            if(words.empty()) {
                //completion line should contain at least executable name
                return {};
            }
            if(!parser_->exename.empty() && words[1] != parser_->exename) {
                // name of the executable doesn't correspond to the first arg
                return {};
            }
            if(words.size() == 1) {
                //list all possible subcommands
                std::vector<std::string> res;
                for(const auto& subcmd: all_variants) {
                    res.push_back(subcmd.name);
                }
                return res;
            }
            const auto& selected_subcmd_str = words[1];
            
            std::vector<Subcommand>::iterator subcommand = std::find_if(all_variants.begin(), all_variants.end(), [&selected_subcmd_str](auto it)
            {
                return it.name == selected_subcmd_str;
            });
            if(subcommand == all_variants.end()) {
                // unknown subcommand
                return {};
            }
            for(size_t n = 2; n < words.size(); n++) {
                const auto& curparam = words[n];
                auto it = std::find(subcommand->params.begin(), subcommand->params.end(), curparam);
                if(it == subcommand->params.end())  {
                    //incorrect param found
                    return {};
                }
                subcommand->params.erase(it);
            }
            return subcommand->params;
        }

        std::vector<Subcommand> getAllVariants() {
            std::vector<Subcommand> res;
            for(auto it : parser_->getSubcommands()) {
                std::shared_ptr<ProgramOptionsParser> parser = it.second;
                Subcommand subcmd;
                subcmd.name = it.first;
                for(auto grp : parser->groups()) {
                    for(auto opt: grp->visible.options()) {
                        subcmd.params.push_back(opt->long_name());
                    }
                }
                res.push_back(std::move(subcmd));

            }
            return res;

        }

        std::string_view trim(std::string_view str) {
            size_t trim_from_start = str.find_first_not_of(' ');
            size_t trim_from_end = str.find_last_not_of(' ');
            if(trim_from_start != std::string_view::npos) {
                assert(trim_from_end != std::string_view::npos);
                size_t len = trim_from_end - trim_from_start + 1;
                return str.substr(trim_from_start, len);
            }
            return str.substr(0, 0); //empty string view
        }

        std::vector<std::string_view> splitCompletionLine(const std::optional<std::string>& line) {
            if(!line.has_value())
                return {};
            std::vector<std::string_view> res;
            std::string_view source = line.value();
            size_t start = 0;
            size_t finish;            
            do {
                finish = source.find(' ', start+1);
                auto substr = source.substr(start, finish - start);
                substr = trim(substr);
                if(substr.length() > 0) {
                    res.push_back(substr);
                }
                start = finish + 1;
            } while(finish != std::string::npos);
            return res;
        }

        std::optional<std::string> getCompletionLine() {
            if(const char* cstr = getenv("COMP_LINE")) {
                return std::string(cstr);
            } else {
                return std::nullopt;
            }
        }

};

} /* program_options_heavy */

#endif