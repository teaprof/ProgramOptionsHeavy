#ifndef __PROGRAM_OPTIONS_PRINTER_H__
#define __PROGRAM_OPTIONS_PRINTER_H__

#include <Help/TextExtractors.h>
#include <Parsers/Parser.h>
#include <Printers/PrettyPrinter.h>

namespace program_options_heavy
{

namespace printers
{

class ProgramOptionsPrinter
{
  public:
    std::shared_ptr<Section> print(Parser &parser)
    {
        auto res = std::make_shared<Section>();
        auto usage = std::make_shared<Section>();
        usage->title = "Usage";
        usage->add_paragraph("\t" + shortHelp(parser));

        auto description = std::make_shared<Section>();
        description->title = "Detailed description:";
        description->add_paragraph(parser.program_description);

        auto details = std::make_shared<Section>();
        details->title = "Details:";
        for (auto &group : parser.groups())
        {
            details->items.push_back(print(*group));
        }

        res->items.push_back(usage);
        res->items.push_back(description);
        res->items.push_back(details);
        return res;
    }
    std::string shortHelp(Parser &parser) const
    {
        std::stringstream str;
        str << parser.exename << " ";
        for (auto group : parser.groups())
        {
            str << "[" << group->groupName() << "] ";
        }
        return str.str();
    }
    static std::shared_ptr<Section> print(OptionsGroup &grp)
    {
        auto res = std::make_shared<Section>();
        res->title = grp.groupName();
        res->add_paragraph(grp.description());
        std::stringstream options_list;
        for(const auto& opt : grp.options->unlocks) {
            OptionTextExtractor prn(grp.help());
            opt->accept(prn);
            options_list<<prn.str()<<"\n";
        }
        res->add_paragraph(options_list.str());
        return res;
    }
    std::set<std::string> options_groups_printed_already_;
};

} /* namespace printers */

} /* namespace program_options_heavy */

#endif // __PROGRAM_OPTIONS_PRINTER_H__