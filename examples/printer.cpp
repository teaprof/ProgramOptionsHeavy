#include <Backend/Printer.h>
#include <Backend/Facade.h>

int main() {
    auto common_options = std::make_shared<OptionsGroup2>();
    common_options->
        addUnlock(std::make_shared<NamedOption>("--named"))->
        addUnlock(std::make_shared<NamedOptionWithValue<int>>("--valued"));
    auto run_options = std::make_shared<OptionsGroup2>();
    run_options->addUnlock(std::make_shared<NamedOptionWithValue<int>>("--dimension"));
    auto gather_options = std::make_shared<OptionsGroup2>();
    gather_options->addUnlock(std::make_shared<PositionalOptionWithValue<std::string>>());

    auto runCommand = std::make_shared<LiteralString>("run")->addUnlock(run_options)->addUnlock(common_options);
    auto gatherCommand = std::make_shared<LiteralString>("gather")->addUnlock(gather_options)->addUnlock(common_options);
    
    auto top_level_options = std::make_shared<OneOf>();
    top_level_options->addAlternative(runCommand);
    top_level_options->addAlternative(gatherCommand);

    Printer prn;
    top_level_options->accept(prn);
    return 0;
}

