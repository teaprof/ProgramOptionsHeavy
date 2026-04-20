#ifndef PARSERS_PARSERWITHSUBCOMMANDS_H
#define PARSERS_PARSERWITHSUBCOMMANDS_H

#include <Parsers/AbstractOptionsParser.h>
#include <Parsers/OptionsGroup.h>
#include <Parsers/Parser.h>

namespace program_options_heavy {

class ProgramSubcommandsPrinter;

class ParserWithSubcommands : public AbstractOptionsParser {
   public:
    using ValueT = std::shared_ptr<Parser>;
    using SubcommandsT = std::map<std::string, ValueT>;

    ParserWithSubcommands(const std::string& exename = "") : AbstractOptionsParser(exename) {}
    ParserWithSubcommands(int argc, const char* argv[]) : AbstractOptionsParser(argc, argv) {}
    SubcommandsT getSubcommands() { return subcommands_; }
    std::shared_ptr<Parser> pushBack(const std::string& subcommand_name, std::shared_ptr<Parser> val) {
        auto res = subcommands_.emplace(subcommand_name, val);
        if (!res.second) {
            throw std::runtime_error(
                "The specified subcommand_name is already "
                "present in SubcommandsParser");
        }
        subcommands_order_.push_back(subcommands_.find(subcommand_name));
        return res.first->second;
    }
    std::shared_ptr<Parser> operator[](const std::string& subcommand_name) {
        auto pos = subcommands_.find(subcommand_name);
        if (pos == subcommands_.end()) {
            pos = subcommands_.emplace(subcommand_name, std::make_shared<Parser>(exename)).first;
            subcommands_order_.push_back(subcommands_.find(subcommand_name));
        }
        return pos->second;
    }
    std::shared_ptr<Parser> at(const std::string& subcommand_name) {
        auto pos = subcommands_.find(subcommand_name);
        assert(pos != subcommands_.end());
        return pos->second;
    }
    std::shared_ptr<Parser> defaultSubcommand() { return at(default_subcommand_name_); }
    void setDefaultSubcommand(const std::string& subcommand_name, bool hide) {
        assert(subcommands_.contains(subcommand_name));
        default_subcommand_name_ = subcommand_name;
        is_default_subcommand_enabled_ = true;
        hide_default_subcommand_name_ = hide;  // if true the default subcommand name will be replaced by
                                               // empty string in the help message
    }
    const std::string& defaultSubcommandName() { return default_subcommand_name_; }
    std::shared_ptr<Parser> selectedSubcommand() { return selected_subcommand_->second; }
    const std::string& selectedSubcommandName()  // TODO move to ParseResults class
    {
        return selected_subcommand_->first;
    }
    bool parse(int argc, const char* argv[]) override {
        assert(!subcommands_.empty());
        auto top_level_options = std::make_shared<OneOfPositional>();
        for (auto it : subcommands_) {
            auto command = std::make_shared<LiteralString>(it.first);
            for (auto grp : it.second->groups()) {
                command->addUnlock(grp->options);
            }
            top_level_options->addAlternative(command);
        }
        /*        bool fallback_to_default = true;
                if (argc >= 2)
                {
                    const char *first_arg = argv[1];
                    selected_subcommand_ = subcommands_.find(first_arg);
                    if (selected_subcommand_ != subcommands_.end())
                    {
                        argc--;
                        argv++;
                        fallback_to_default = false;
                    };
                };
                if (fallback_to_default)
                {
                    // Subcommand is unknown or no subcommand is specified,
           select the
                    // default subcommand
                    if (is_default_subcommand_enabled_)
                    {
                        selected_subcommand_ =
           subcommands_.find(default_subcommand_name_);
           assert(selected_subcommand_
           != subcommands_.end());
                    }
                    else
                    {
                        throw std::runtime_error("Invalid program arguments");
                    }
                };
                selected_subcommand_->second->parse(argc, argv);
                activated = true;*/
        Matcher matcher(top_level_options);
        std::vector<std::string> args;
        for (int n = 1; n < argc; n++) {  // skip the name of executable
            args.push_back(argv[n]);
        }
        activated = true;  // TODO wtf?
        ArgGrammarParser parser(args);
        return matcher.parse(parser);
    }
    void validate() override {}
    void update(const boost::program_options::variables_map& vm) override {}
    std::vector<SubcommandsT::iterator>& subcommandsOrder() { return subcommands_order_; }
    bool hideDefaultSubcommandName() const { return hide_default_subcommand_name_; }

    bool activated{false};  // becomes true when parse function succeeded
   private:
    SubcommandsT subcommands_;
    std::vector<SubcommandsT::iterator> subcommands_order_;  // order of subcommands_ for printing purpose
    SubcommandsT::iterator selected_subcommand_;
    std::string default_subcommand_name_{"default"};
    bool hide_default_subcommand_name_{false};
    bool is_default_subcommand_enabled_{false};
    friend class ProgramSubcommandsPrinter;
};

} /* namespace program_options_heavy */

#endif  // PARSERS_PARSERWITHSUBCOMMANDS_H