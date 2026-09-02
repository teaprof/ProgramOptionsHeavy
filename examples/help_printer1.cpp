#include <ProgramOptionsHeavy.h>

using program_options_heavy::HelpOptions;
using program_options_heavy::MultithreadOptions;
using program_options_heavy::DynamicParser;
using program_options_heavy::printers::PrettyPrinter;
using program_options_heavy::printers::ProgramOptionsFormatter;

int main(int argc, const char* argv[]) {
    DynamicParser parser(argc, argv);
    auto help_options = std::make_shared<HelpOptions>();
    auto multithreading_options = std::make_shared<MultithreadOptions>();
    parser.addGroup(help_options);
    parser.addGroup(multithreading_options);

    ProgramOptionsFormatter formatter;
    auto dom = formatter.print(parser);

    PrettyPrinter printer;
    dom->accept(printer);
    return 0;
}
