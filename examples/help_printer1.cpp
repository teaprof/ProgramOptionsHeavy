#include <ProgramOptionsHeavy.h>

using program_options_heavy::CommonOptions;
using program_options_heavy::MultithreadOptions;
using program_options_heavy::DynamicParser;
using program_options_heavy::printers::PrettyPrinter;
using program_options_heavy::printers::MarkdownPrinter;
using program_options_heavy::printers::ProgramOptionsFormatter;

int main(int argc, const char* argv[]) {
    DynamicParser parser(argc, argv);
    auto common_options = std::make_shared<CommonOptions>();
    auto multithreading_options = std::make_shared<MultithreadOptions>();
    parser.addGroup(common_options);
    parser.addGroup(multithreading_options);

    ProgramOptionsFormatter formatter;
    auto dom = formatter.print(parser);

    //PrettyPrinter printer;
    MarkdownPrinter printer;
    dom->accept(printer);
    return 0;
}
