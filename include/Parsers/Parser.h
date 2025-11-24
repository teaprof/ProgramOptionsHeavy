#ifndef __PROGRAM_OPTIONS_PARSER_H__
#define __PROGRAM_OPTIONS_PARSER_H__

#include <Parsers/AbstractOptionsParser.h>
#include <Parsers/OptionsGroup.h>
#include <Backend/Matcher.h>

#include <iostream>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <variant>

namespace program_options_heavy
{

class Parser : public AbstractOptionsParser
{
    // This class can parse the list of options and print the help message
    // Use this class for simple set of command line options like:
    // programname --arg1 --arg2 10 -zxc -v 20 input.txt output.txt
  public:
    Parser(const std::string &exename = "") : AbstractOptionsParser(exename)
    {
    }
    Parser(int argc, const char *argv[]) : AbstractOptionsParser(argc, argv)
    {
    }
    virtual void addGroup(std::shared_ptr<OptionsGroup> options)
    {
        groups_.push_back(options);
    }
    bool parse(int argc, const char *argv[]) override
    {
        auto options = std::make_shared<OptionsGroup2>();
        for(auto grp : groups_) {
            options->addUnlock(grp->options);
        }
        std::vector<std::string> args;
        for(int n = 0; n < argc; n++) {
            args.push_back(argv[n]);
        }
        Matcher matcher(options);
        matcher.parse(args);
        return true;
    }
    void validate() override
    {
        /*for (auto it : groups_)
            it->validate();*/
    }
    void update(const boost::program_options::variables_map &vm) override
    {
    }
    const std::vector<std::shared_ptr<OptionsGroup>> groups() const
    {
        return groups_;
    }

    bool activated{false}; // becomes true when parse function succeeded
  private:
    std::vector<std::shared_ptr<OptionsGroup>> groups_;
};

} /* namespace program_options_heavy */

#endif // __PROGRAM_OPTIONS_PARSER_H__