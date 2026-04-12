#ifndef PRINTERS_PROGRAMSUBCOMMANDSPRINTER_H
#define PRINTERS_PROGRAMSUBCOMMANDSPRINTER_H

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
            usage->addParagraph("\t" + shortHelp(parser, subcmd));
        }

        auto description = std::make_shared<Section>();
        description->title = "Description:";
        description->addParagraph(parser.program_description);
        for (auto &subcmd : parser.subcommandsOrder())
        {
            description->addParagraph("\t" + subcommandDescription(parser, subcmd));
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
    static std::string shortHelp(ParserWithSubcommands &parser, ParserWithSubcommands::SubcommandsT::iterator it) 
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
    static std::string subcommandDescription(ParserWithSubcommands &parser, ParserWithSubcommands::SubcommandsT::iterator it) 
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
            if (!options_groups_printed_already.contains(it->groupName()))
            {
                res.push_back(print(*it));
                options_groups_printed_already.insert(it->groupName());
            }
        }
        return res;
    }
    static std::shared_ptr<Section> print(OptionsGroup &grp) 
    {
        return ProgramOptionsPrinter::print(grp);
    }
    std::set<std::string> options_groups_printed_already;
};

} /* namespace printers */

} /* namespace program_options_heavy */

#endif // PRINTERS_PROGRAMSUBCOMMANDSPRINTER_H