#ifndef __PROGRAM_SUBCOMMANDS_PRINTER_H__
#define __PROGRAM_SUBCOMMANDS_PRINTER_H__

#include <Parsers/ParserWithSubcommands.h>
#include <Printers/PrettyPrinter.h>

namespace program_options_heavy
{

namespace printers
{

class ProgramSubcommandsPrinter
{
  public:
    std::shared_ptr<Section> print(ParserWithSubcommands &parser)
    {
        auto res = std::make_shared<Section>();
        auto usage = std::make_shared<Section>();
        usage->title = "Usage:";
        for (auto &subcmd : parser.subcommandsOrder())
        {
            usage->add_paragraph("\t" + shortHelp(parser, subcmd));
        }

        auto description = std::make_shared<Section>();
        description->title = "Description:";
        description->add_paragraph(parser.program_description);
        for (auto &subcmd : parser.subcommandsOrder())
        {
            description->add_paragraph("\t" + subcommandDescription(parser, subcmd));
        }

        auto details = std::make_shared<Section>();
        details->title = "Details:";
        for (auto &subcmd : parser.subcommandsOrder())
        {
            auto ptr = subcmd->second;
            for (auto &it : print(*ptr))
            {
                details->items.push_back(it);
            }
        }

        res->items.push_back(usage);
        res->items.push_back(description);
        res->items.push_back(details);
        return res;
    }
    std::string shortHelp(ParserWithSubcommands &parser, ParserWithSubcommands::subcommands_t::iterator it) const
    {
        std::stringstream str;
        str << parser.exename << " ";
        if (it->first == parser.defaultSubcommandName())
        {
            if (!parser.hideDefaultSubcommandName())
            {
                str << "[" << it->first << "] ";
            }
        }
        else
        {
            str << it->first << " ";
        }
        const std::shared_ptr<Parser> opts = it->second;
        for (auto group : opts->groups())
        {
            str << "[" << group->groupName() << "] ";
        }
        return str.str();
    }
    std::string subcommandDescription(ParserWithSubcommands &parser, ParserWithSubcommands::subcommands_t::iterator it) const
    {
        std::stringstream str;
        if (it->first != parser.defaultSubcommandName() || !parser.hideDefaultSubcommandName())
        {
            str << it->first << " - ";
        }
        str << it->second->program_description;
        return str.str();
    }
    std::vector<std::shared_ptr<Section>> print(Parser &parser)
    {
        std::vector<std::shared_ptr<Section>> res;
        for (auto it : parser.groups())
        {
            if (!options_groups_printed_already_.contains(it->groupName()))
            {
                res.push_back(print(*it));
                options_groups_printed_already_.insert(it->groupName());
            }
        }
        return res;
    }
    std::shared_ptr<Section> print(OptionsGroup &grp) const
    {
        return ProgramOptionsPrinter::print(grp);
    }
    std::set<std::string> options_groups_printed_already_;
};

} /* namespace printers */

} /* namespace program_options_heavy */

#endif // __PROGRAM_SUBCOMMANDS_PRINTER_H__