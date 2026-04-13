#ifndef PARSERS_ABSTRACTOPTIONSPARSER_H
#define PARSERS_ABSTRACTOPTIONSPARSER_H

#include <boost/program_options.hpp>
#include <filesystem>

namespace program_options_heavy {

class AbstractOptionsParser {
   public:
    AbstractOptionsParser(const std::string& exename) : exename{exename} {}
    AbstractOptionsParser(int argc, const char* argv[]) {
        if (argc > 0) {
            std::filesystem::path path(argv[0]);
            exename = path.filename();
        }
    }

    virtual bool parse(int argc, const char* argv[]) = 0;
    virtual void validate() = 0;
    virtual void update(const boost::program_options::variables_map& vm) = 0;

    std::string exename;
    std::string program_description;
};

} /* namespace program_options_heavy */

#endif  // PARSERS_ABSTRACTOPTIONSPARSER_H