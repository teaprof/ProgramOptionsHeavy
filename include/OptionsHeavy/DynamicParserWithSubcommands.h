    #ifndef PARSERS_PARSERWITHSUBCOMMANDS_H
#define PARSERS_PARSERWITHSUBCOMMANDS_H

#include <OptionsHeavy/AbstractDynamicOptionsParser.h>
#include <OptionsHeavy/basic/HeavyOption.h>

namespace program_options_heavy {

class ProgramSubcommandsPrinter;

class ParserWithSubcommands : public AbstractDynamicOptionsParser {
   public:
    using ValueT = std::shared_ptr<HeavyOptionsGroups>;
    using SubcommandsT = std::map<std::string, ValueT>;

    ParserWithSubcommands(const std::string& exename = "") : AbstractDynamicOptionsParser(exename) {}
    ParserWithSubcommands(int argc, const char* argv[]) : AbstractDynamicOptionsParser(argc, argv) {}
    SubcommandsT getSubcommands() { return subcommands_; }
    std::shared_ptr<HeavyOptionsGroups> pushBack(const std::string& subcommand_name, std::shared_ptr<HeavyOptionsGroups> val) {
        auto res = subcommands_.emplace(subcommand_name, val);
        if (!res.second) {
            throw std::runtime_error(
                "The specified subcommand_name is already "
                "present in SubcommandsParser");
        }
        subcommands_order_.push_back(subcommands_.find(subcommand_name));
        return res.first->second;
    }
    std::shared_ptr<HeavyOptionsGroups> operator[](const std::string& subcommand_name) {
        auto pos = subcommands_.find(subcommand_name);
        if (pos == subcommands_.end()) {
            pos = subcommands_.emplace(subcommand_name, std::make_shared<HeavyOptionsGroups>(exename)).first;
            subcommands_order_.push_back(subcommands_.find(subcommand_name));
        }
        return pos->second;
    }
    std::shared_ptr<HeavyOptionsGroups> at(const std::string& subcommand_name) {
        auto pos = subcommands_.find(subcommand_name);
        assert(pos != subcommands_.end());
        return pos->second;
    }
    void setFreeOptionsGroup(const std::shared_ptr<HeavyOptionsGroup> grp) {
        this->free_options_group_ = grp;
    }
    //std::shared_ptr<HeavyOptionsGroups> defaultSubcommand() { return at(default_subcommand_name_); }
    /*void setDefaultSubcommand(const std::string& subcommand_name, bool hide) {
        assert(subcommands_.contains(subcommand_name));
        default_subcommand_name_ = subcommand_name;
        is_default_subcommand_enabled_ = true;
        hide_default_subcommand_name_ = hide;  // if true the default subcommand name will be replaced by
                                               // empty string in the help message
    }*/
    //const std::string& defaultSubcommandName() { return default_subcommand_name_; }    
    std::shared_ptr<HeavyOptionsGroups> selectedSubcommand() { return selected_subcommand_->second; }
    const std::string& selectedSubcommandName()  // TODO move to ParseResults class
    {
        return selected_subcommand_->first;
    }
    bool parse(int argc, const char* argv[]) override {
        assert(!subcommands_.empty());
        auto top_level_options = std::make_shared<OptionsGroup>();
        auto top_level_subcommands = std::make_shared<OneOfPositional>();
        for (auto it : subcommands_) {
            auto command = std::make_shared<LiteralString>(it.first);
            for (auto grp : it.second->groups()) {
                command->addUnlock(grp->options);
            }
            top_level_subcommands->addAlternative(command);
        }
        top_level_options->addUnlock(top_level_subcommands);
        top_level_options->addUnlock(free_options_group_->options);
        Parser parser(top_level_options);
        std::vector<std::string> args;
        for (int n = 1; n < argc; n++) {  // skip the name of executable
            args.push_back(argv[n]);
        }
        //activated = true; 
        ArgGrammarParser grammar_parser(args);
        bool res = parser.parse(grammar_parser);
        if(res) {
            std::optional<size_t> selected_idx = parser.storage.selectedAlternativeIndex(top_level_subcommands);
            if(selected_idx.has_value()) {
                assert(selected_idx.value() < subcommands_order_.size());
                selected_subcommand_ = subcommands_order_[selected_idx.value()];
            };
        }
        return res;
    }
    void validate() override {}
    void update(const boost::program_options::variables_map& vm) override {}
    std::vector<SubcommandsT::iterator>& subcommandsOrder() { return subcommands_order_; }
    //bool hideDefaultSubcommandName() const { return hide_default_subcommand_name_; }

    //bool activated{false};  // becomes true when parse function succeeded
   private:
    SubcommandsT subcommands_;
    std::vector<SubcommandsT::iterator> subcommands_order_;  // order of subcommands_ for printing purpose
    std::shared_ptr<HeavyOptionsGroup> free_options_group_; // free options are available anywhere and should not be unlocked by subcommands
    SubcommandsT::iterator selected_subcommand_;
    //std::string default_subcommand_name_{"default"};
    //bool hide_default_subcommand_name_{false};
    bool is_default_subcommand_enabled_{false};
    friend class ProgramSubcommandsPrinter; // TODO: remove out of here
};

} /* namespace program_options_heavy */

#endif  // PARSERS_PARSERWITHSUBCOMMANDS_H