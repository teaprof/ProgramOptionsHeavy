#include<ProgramOptionsHeavy.h>

using program_options_heavy::Parser;
using program_options_heavy::HelpOptions;
using program_options_heavy::MultithreadOptions;
using program_options_heavy::printers::ProgramOptionsPrinter;
using program_options_heavy::printers::PrettyPrinter;


int main(int argc, const char* argv[]) {
    Parser parser(argc, argv);
    auto help_options = std::make_shared<HelpOptions>();
    auto multithreading_options = std::make_shared<MultithreadOptions>();
    parser.addGroup(help_options);
    parser.addGroup(multithreading_options);

    ProgramOptionsPrinter printer;
    auto dom = printer.print(parser);

    PrettyPrinter printer;
    dom->accept(printer);
    return 0;
}
