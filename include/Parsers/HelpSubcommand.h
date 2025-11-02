#ifndef __HELP_SUBCOMMAND_H__
#define __HELP_SUBCOMMAND_H__

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
#endif //__HELP_SUBCOMMAND_H__