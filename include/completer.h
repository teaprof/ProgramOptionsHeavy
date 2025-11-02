#ifndef __COMPLETER_H__
#define __COMPLETER_H__

#include <Parsers/ParserWithSubcommands.h>

#include <string>
#include <optional>
#include <vector>
#include <string_view>
#include <ranges>
#include <cstdlib>

namespace program_options_heavy {

inline std::vector<std::string> getCompletionVariants(const std::string &program, const std::string &curargument,
                                         const std::string &prevargument)
{
    return {"run", "collect"};
}


class Completer {
    /// TODOLIST:
    /// 1. Process the values passed for options
    /// 2. Take into account that any option can have different names
    public:
        Completer(std::shared_ptr<ParserWithSubcommands>& parser) : parser_{parser} {
            all_variants_ = getAllVariants();
            for(const auto& it : all_variants_)
                all_command_names.push_back(it.first);
        }

        std::vector<std::string> getCompletionVariants() {            
            auto completion_line = getLineForCompletion();
            if(completion_line.has_value())
                return getCompletionVariants(completion_line.value());
            return {};
        }

        std::vector<std::string> getCompletionVariants(const std::string& completion_line) {
            auto words = split(completion_line);
            return getCompletionVariants(words);
        }

    private:
        std::shared_ptr<ParserWithSubcommands> parser_;

        struct Command {
            std::string name;
            std::vector<std::vector<std::string>> options;
            std::vector<std::string> canonical_options; // same size as options
        };        
        std::map<std::string, Command> all_variants_;
        std::vector<std::string> all_command_names;

        struct MatchResults {
            std::optional<std::size_t> whole_match_idx;
            std::vector<std::string> variants;
            std::vector<std::size_t> variants_idx;
        };

        MatchResults getCompletionVariants(const std::string_view& word, const std::vector<std::string>& variants) {
            MatchResults res;
            for(size_t n = 0; n < variants.size(); n++) {
                if(variants[n].find(word) == 0) {
                    res.variants.push_back(variants[n]);
                    res.variants_idx.push_back(n);
                    if(variants[n] == word) {
                        res.whole_match_idx = n;
                    }
                }
            }
            return res;
        }

        MatchResults getCompletionVariants(const std::string_view& word, const std::vector<std::vector<std::string>>& variants) {
            MatchResults res;
            for(size_t n = 0; n < variants.size(); n++) {
                MatchResults tmp = getCompletionVariants(word, variants[n]);                
                if(tmp.variants.size() > 0) {
                    res.variants.push_back(tmp.variants.front());
                    res.variants_idx.push_back(n);
                }
                if(tmp.whole_match_idx.has_value()) {
                    res.whole_match_idx = n;
                }
            }
            return res;
        }

        std::tuple<std::string_view, std::string_view, std::vector<std::string_view>> byRoles(const std::vector<std::string_view>& words) {
            std::string_view exec_name;
            std::string_view command_name;
            std::vector<std::string_view> options;
            size_t processed = 0;
            if(!parser_->exename.empty() && words.size() > 0) {
                exec_name = words[0];
                processed++;
            }
            if(words.size() > processed && words[processed].find('-') != 0) {
                command_name = words[processed];
                processed++;
            }
            for(size_t n = processed; n < words.size(); n++) {
                options.push_back(words[n]);
            }
            return {exec_name, command_name, options};
        }

        std::vector<std::string> getCompletionVariants(const std::vector<std::string_view>& words) {
            auto [exec_name, command_name, options] = byRoles(words);

            // match the name of the executable
            std::vector<std::string> exe_variants{parser_->exename};
            if(!parser_->exename.empty()) {                                
                MatchResults exe_match_results = getCompletionVariants(exec_name, exe_variants);
                if(exe_match_results.whole_match_idx.has_value() == false)
                    return exe_match_results.variants;
            }            

            // match the name of the command
            MatchResults command_match_results = getCompletionVariants(command_name, all_command_names);
            if(command_match_results.whole_match_idx.has_value() == false)
                return command_match_results.variants;
            
            const std::string& subcommand = all_command_names[command_match_results.whole_match_idx.value()];

            std::vector<std::vector<std::string>> rest_options = all_variants_[subcommand].options;
            std::vector<std::string> canonical_rest_options = all_variants_[subcommand].canonical_options;
            // match command options
            if(options.size() > 0) {
                bool last_option_full_match = false;

                // remove all used options
                std::vector<size_t> remove_idx;
                for(size_t idx = 0; idx < options.size(); idx++) {
                    MatchResults match_results = getCompletionVariants(options[idx], rest_options);
                    if(match_results.whole_match_idx.has_value()) {
                        size_t rest_idx = match_results.whole_match_idx.value();
                        remove_idx.push_back(rest_idx);
                        if(idx + 1 == options.size()) {
                            last_option_full_match = true;
                        }
                    };
                }
                for(auto it = remove_idx.rbegin(); it != remove_idx.rend(); it++) {
                    rest_options.erase(rest_options.begin() + *it);
                    canonical_rest_options.erase(canonical_rest_options.begin() + *it);
                }
                // last option should be processed separately
                if(!last_option_full_match) {
                    const std::string_view& last_option = options.back();
                    MatchResults match_results = getCompletionVariants(last_option, rest_options);
                    return match_results.variants;
                }
            }
            return canonical_rest_options;
        }

        std::map<std::string, Command> getAllVariants() {
            std::map<std::string, Command> res;
            for(auto it : parser_->getSubcommands()) {
                std::shared_ptr<ProgramOptionsParser> parser = it.second;
                Command cmd;
                cmd.name = it.first;
                for(auto grp : parser->groups()) {
                    for(auto opt: grp->visible.options()) {
                        auto& oopt = *opt;
                        cmd.options.push_back(getOptionNames(oopt));
                        cmd.canonical_options.push_back(getCanonicalName(oopt));
                    }
                }
                res[cmd.name] = std::move(cmd);

            }
            return res;
        }


        std::string getCanonicalName(const boost::program_options::option_description& opt) {
            const std::vector<std::string>& all_names = getOptionNames(opt);
            assert(!all_names.empty());
            return all_names.front();
        }
        std::vector<std::string> getOptionNames(const boost::program_options::option_description& opt) {
            /// \todo: move to OptionsGroup
            std::vector<std::string> res;
            const std::pair<const std::string*, size_t> long_names = opt.long_names();
            for(size_t n = 0; n < long_names.second; n++) {
                std::string str = std::string("--") + *(long_names.first + n);
                res.push_back(str);
            }
            //extract short name:
            std::string short_name = opt.canonical_display_name(boost::program_options::command_line_style::allow_dash_for_short);
            if(!short_name.empty()) {
                if(short_name.size() == 1) {
                    short_name = std::string("-") + short_name;
                }
                res.push_back(short_name);
            } else {
                res.push_back("empty");
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

        std::vector<std::string_view> split(std::string_view line) {
            std::vector<std::string_view> res;            
            size_t start = 0;
            size_t finish;            
            do {
                finish = line.find(' ', start+1);
                auto substr = line.substr(start, finish - start);
                substr = trim(substr);
                if(substr.length() > 0) {
                    res.push_back(substr);
                }
                start = finish + 1;
            } while(finish != std::string::npos);
            return res;
        }

        std::optional<std::string> getLineForCompletion() {
            if(const char* cstr = getenv("COMP_LINE")) {
                return std::string(cstr);
            } else {
                return std::nullopt;
            }
        }

};

} /* program_options_heavy */

#endif