#ifndef PRINTERS_PROGRAMOPTIONSPRINTER_H
#define PRINTERS_PROGRAMOPTIONSPRINTER_H

#include <Help/TextExtractors.h>
#include <OptionsHeavy/DynamicParser.h>
#include <Printers/PrettyPrinter.h>

namespace program_options_heavy {

namespace printers {

class ProgramOptionsFormatter {
   public:
    enum class OptionsListFormat {
        asTable,
        asList,
        decorated
    };
    OptionsListFormat optionsListFormat{OptionsListFormat::asList};
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
    std::shared_ptr<Section> print(HeavyOptionsGroup& grp) const {
        auto res = std::make_shared<Section>();
        res->title = grp.groupName();
        res->addParagraph(grp.description());
        std::shared_ptr<AbstractItem> optionList;
        switch(optionsListFormat) {
            case OptionsListFormat::asList: 
                optionList = printOptionsAsList(grp);
            break;
            case OptionsListFormat::asTable: 
                optionList = printOptionsAsTable(grp);
            break;
            case OptionsListFormat::decorated: 
                optionList = printOptionsDecorated(grp);
            break;
        }
        res->items.push_back(optionList);
        return res;
    }
    private:
    static std::shared_ptr<AbstractItem> printOptionsAsList(HeavyOptionsGroup& grp) {
        auto res = std::make_shared<UnorderedList>();
        for (const auto& opt : grp.options->unlocks()) {
            OptionTextExtractor prn(grp.help());
            opt->accept(prn);
            res->items.push_back(prn.descr.keys + "  " + prn.descr.description);
        }
        return res;
    }
    static std::shared_ptr<AbstractItem> printOptionsAsTable(HeavyOptionsGroup& grp) {
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
        return table;
    }
    static std::shared_ptr<AbstractItem> printOptionsDecorated(HeavyOptionsGroup& grp) {
        auto res = std::make_shared<Section>();
        std::stringstream options_list;
        for (const auto& opt : grp.options->unlocks()) {
            OptionTextExtractor prn(grp.help());
            opt->accept(prn);
            options_list << prn.descr.keys << "\n" << prn.descr.description << "\n";
        }
        res->addParagraph(options_list.str());
        return res;
    }

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