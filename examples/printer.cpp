#include <OptionsEasy/OptionsEasy.h>
#include <Backend/Printer.h>

int main() {
    auto common_options = std::make_shared<OptionsGroup2>();
    common_options->addUnlock(std::make_shared<NamedOption>("--named"))
        ->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--valued"));
    auto run_options = std::make_shared<OptionsGroup2>();
    run_options->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--dimension"));
    auto gather_options = std::make_shared<OptionsGroup2>();
    gather_options->addUnlock(std::make_shared<PositionalOptionWithValue<std::string>>());

    auto run_command = std::make_shared<LiteralString>("run")->addUnlock(run_options)->addUnlock(common_options);
    auto gather_command = std::make_shared<LiteralString>("gather")->addUnlock(gather_options)->addUnlock(common_options);

    auto top_level_options = std::make_shared<OneOfPositional>();
    top_level_options->addAlternative2(run_command);
    top_level_options->addAlternative2(gather_command);

    Printer prn;
    top_level_options->accept(prn);
    return 0;
}
