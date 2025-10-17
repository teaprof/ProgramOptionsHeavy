#ifndef __PROGRAM_OPTIONS_PARSER_H__
#define __PROGRAM_OPTIONS_PARSER_H__

#include <Parsers/AbstractOptionsParser.h>
#include <Parsers/OptionsGroup.h>

#include <iostream>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <variant>

namespace program_options_heavy
{

class ProgramOptionsParser : public AbstractOptionsParser
{
    // This class can parse the list of options and print the help message
    // Use this class for simple set of command line options like:
    // programname --arg1 --arg2 10 -zxc -v 20 input.txt output.txt
  public:
    ProgramOptionsParser(const std::string &exename = "") : AbstractOptionsParser(exename)
    {
    }
    ProgramOptionsParser(int argc, const char *argv[]) : AbstractOptionsParser(argc, argv)
    {
    }
    virtual void addGroup(std::shared_ptr<OptionsGroup> options)
    {
        if (options->positional.max_total_count() != 0)
        {
            for (auto it : groups_)
            {
                // only one group of options is allowed to have positional
                // arguments
                assert(it->positional.max_total_count() == 0);
            }
        }
        groups_.push_back(options);
    }
    bool parse(int argc, const char *argv[]) override
    {
        namespace po = boost::program_options;
        boost::program_options::options_description partial;
        boost::program_options::positional_options_description positional;
        for (auto it : groups_)
        {
            partial.add(it->partial);
            if (it->positional.max_total_count() != 0)
            {
                // only one group of options is allowed to have positional
                // arguments
                assert(positional.max_total_count() == 0);
                positional = it->positional;
            }
        }
        boost::program_options::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(partial).positional(positional).run(), vm);
        boost::program_options::notify(vm);
        for (auto it : groups_)
        {
            it->update(vm);
        }
        activated = true;
        return true;
    }
    void validate() override
    {
        for (auto it : groups_)
            it->validate();
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