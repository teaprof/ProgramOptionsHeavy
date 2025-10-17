#ifndef __ABSTRACT_OPTIONS_PARSER_H__
#define __ABSTRACT_OPTIONS_PARSER_H__

#include <boost/program_options.hpp>
#include <filesystem>

namespace program_options_heavy
{

class AbstractOptionsParser
{
  public:
    AbstractOptionsParser(const std::string &exename_) : exename{exename_}
    {
    }
    AbstractOptionsParser(int argc, const char *argv[])
    {
        if (argc > 0)
        {
            std::filesystem::path path(argv[0]);
            exename = path.filename();
        }
    }

    virtual bool parse(int argc, const char *argv[]) = 0;
    virtual void validate() = 0;
    virtual void update(const boost::program_options::variables_map &vm) = 0;

    std::string exename;
    std::string program_description;
};

} /* namespace program_options_heavy */

#endif // __ABSTRACT_OPTIONS_PARSER_H__