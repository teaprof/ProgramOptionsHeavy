#include "Printers/Document.h"
#include "Printers/PrettyPrinter.h"
#include "Printers/MarkdownPrinter.h"
using namespace program_options_heavy::printers;

const std::string longString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum et \
dui eu quam commodo porta. Vestibulum libero ante, pellentesque quis mi vel, placerat placerat augue.  \
Donec varius commodo luctus. Vivamus vel nunc orci. Nulla facilisi. Morbi orci augue, bibendum ac ipsum nec, \
mattis convallis ipsum. In lacinia, ex quis finibus porta, odio lacus hendrerit nulla, ultricies tristique \
risus nisl efficitur sem. Nullam eget arcu pulvinar, molestie tellus et, lobortis nulla. Donec dictum tortor \
at tortor pellentesque, rhoncus consectetur arcu blandit. Vestibulum sit amet mi at risus auctor sodales. In \
semper turpis non nulla laoreet, vel rhoncus ante aliquam. Maecenas non lacinia mauris. Mauris vulputate lacus \
diam, id dapibus ipsum consequat sed.";

const std::string shortString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum et \
dui eu quam commodo porta.";

const std::string singleLineString = "Lorem ipsum dolor sit amet";


int main() {
    Section s1;
    s1.title = "The first section: text";
    s1.addParagraph(singleLineString);
    s1.addParagraph(shortString);
    s1.addParagraph(longString);
    
    Section s2;
    s2.title = "The second section: tables";
    auto table1 = std::make_shared<Table>();
    table1->addRow();
    table1->addCellToRow("Moscow");
    table1->addCellToRow("3930");
    table1->addCellToRow("39320");
    table1->addRow();
    table1->addCellToRow("SPb");
    table1->addCellToRow("39320");
    table1->addCellToRow("3930");
    table1->addRow();
    table1->addCellToRow("Dolgoprudny");
    table1->addCellToRow("392320");
    table1->addCellToRow("-");
    auto s21 = std::make_shared<Section>();
    s21->title = "Simple table";
    s21->addTable(table1);
    s2.items.push_back(s21);

    auto table2 = std::make_shared<Table>();
    table2->addRow();
    table2->addCellToRow("Moscow");
    table2->addCellToRow("39320");
    table2->addCellToRow(singleLineString);
    table2->addRow();
    table2->addCellToRow("SPb");
    table2->addCellToRow("39320");
    table2->addCellToRow(shortString);
    table2->addRow();
    table2->addCellToRow("Dolgoprudny");
    table2->addCellToRow("39320");
    table2->addCellToRow(longString);
    auto s22 = std::make_shared<Section>();
    s22->title = "Table with long strings";
    s22->addTable(table2);
    s2.items.push_back(s22);

    Section s3;
    s3.title = "The 3rd section: unordered list";    
    auto ul1 = std::make_shared<UnorderedList>();
    ul1->items.push_back("item 1");
    ul1->items.push_back("item 2");
    ul1->items.push_back("item 3");
    auto s31 = std::make_shared<Section>();
    s31->title = "Simple list";
    s31->addParagraph(shortString);
    s31->addUnorderedList(ul1);
    s3.items.push_back(s31);

    
    auto ul2 = std::make_shared<UnorderedList>();
    ul2->items.push_back(shortString);
    ul2->items.push_back(longString);
    ul2->items.push_back(shortString);
    auto s32 = std::make_shared<Section>();
    s32->title = "List with long text";
    s32->addParagraph(shortString);
    s32->addUnorderedList(ul2);
    s3.items.push_back(s32);


    //PrettyPrinter printer;
    MarkdownPrinter printer;
    printer.visit(s1);
    printer.visit(s2);
    printer.visit(s3);

    
    
    return 0;
}