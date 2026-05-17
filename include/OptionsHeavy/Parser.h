#ifndef PARSERS_PARSER_H
#define PARSERS_PARSER_H

#include <Backend/Parser.h>
#include <OptionsHeavy/AbstractOptionsParser.h>
#include <OptionsHeavy/basic/OptionsGroup.h>

#include <iostream>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <variant>

namespace program_options_heavy {

class Parser : public AbstractOptionsParser {
    // This class can parse the list of options and print the help message
    // Use this class for simple set of command line options like:
    // programname --arg1 --arg2 10 -zxc -v 20 input.txt output.txt
   public:
    Parser(const std::string& exename = "") : AbstractOptionsParser(exename) {}
    Parser(int argc, const char* argv[]) : AbstractOptionsParser(argc, argv) {}
    virtual void addGroup(std::shared_ptr<DynamicOptionsGroup> options) { groups_.push_back(options); }
    bool parse(int argc, const char* argv[]) override {
        auto options = std::make_shared<OptionsGroup>();
        for (auto grp : groups_) {
            options->addUnlock(grp->options);
        }
        std::vector<std::string> args;
        for (int n = 0; n < argc; n++) {
            args.push_back(argv[n]);
        }
        Parser2 parser(options);
        parser.parse(args);
        return true;
    }
    void validate() override {
        /*for (auto it : groups_)
            it->validate();*/
    }
    void update(const boost::program_options::variables_map& vm) override {}
    std::vector<std::shared_ptr<DynamicOptionsGroup>> groups() const { return groups_; }

   private:
    std::vector<std::shared_ptr<DynamicOptionsGroup>> groups_;
};

} /* namespace program_options_heavy */

#endif  // PARSERS_PARSER_H