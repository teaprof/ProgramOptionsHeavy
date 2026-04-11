#ifndef PARSERS_HELPSUBCOMMAND_H
#define PARSERS_HELPSUBCOMMAND_H

#include <Parsers/BasicOptions.h>
#include <Parsers/Parser.h>

namespace program_options_heavy
{

class HelpSubcommand : public Parser
{
  public:
    HelpSubcommand() : Parser()
    {
        help_options = std::make_shared<program_options_heavy::HelpOptions>();
        addGroup(help_options);
        program_description = "--help - produce this help";
    }
    std::shared_ptr<program_options_heavy::HelpOptions> help_options;

  private:
};

} /* namespace program_options_heavy */
#endif // PARSERS_HELPSUBCOMMAND_H