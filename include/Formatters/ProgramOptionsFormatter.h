#ifndef PRINTERS_PROGRAMOPTIONSPRINTER_H
#define PRINTERS_PROGRAMOPTIONSPRINTER_H

#include <Help/TextExtractors.h>
#include <OptionsHeavy/DynamicParser.h>
#include <Printers/PrettyPrinter.h>

namespace program_options_heavy {

namespace printers {

class ProgramOptionsFormatter {
   public:
    std::shared_ptr<Section> print(DynamicParser& parser) const {
        auto res = std::make_shared<Section>();
        auto usage = std::make_shared<Section>();
        usage->title = "Usage";
        usage->addParagraph("\t" + shortHelp(parser));

        auto description = std::make_shared<Section>();
        description->title = "Detailed description:";
        description->addParagraph(parser.program_description);

        auto details = std::make_shared<Section>();
        details->title = "Details:";
        for (const auto& group : parser.groups()) {
            details->items.push_back(print(*group));
        }

        res->items.push_back(usage);
        res->items.push_back(description);
        res->items.push_back(details);
        return res;
    }
    static std::shared_ptr<Section> print(HeavyOptionsGroup& grp) {
        auto res = std::make_shared<Section>();
        res->title = grp.groupName();
        res->addParagraph(grp.description());
        auto table = std::make_shared<Table>();
        table->align_ = std::vector<int>{-1, -1}; // both columns align left
        std::stringstream options_list;
        for (const auto& opt : grp.options->unlocks()) {
            OptionTextExtractor prn(grp.help());
            opt->accept(prn);
            table->addRow();            
            table->addCellToRow(prn.descr.keys);
            table->addCellToRow(prn.descr.description);
        }
        res->addTable(table);
        return res;
    }
    private:
    static std::string shortHelp(DynamicParser& parser) {
        std::stringstream str;
        str << parser.exename << " ";
        for (auto group : parser.groups()) {
            str << "[" << group->groupName() << "] ";
        }
        return str.str();
    }
    std::set<std::string> options_groups_printed_already;
};

} /* namespace printers */

} /* namespace program_options_heavy */

#endif  // PRINTERS_PROGRAMOPTIONSPRINTER_H