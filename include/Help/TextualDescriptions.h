#ifndef __HELP_TEXTUAL_DESCRIPTIONS_H__
#define __HELP_TEXTUAL_DESCRIPTIONS_H__

#include <string>
#include <vector>

class ParameterDescription {
    public:
        std::string keys;
        std::string description;
};

class GroupDescription {
    public:
        std::string name;
        std::string brief;
        std::string detailed;
        std::vector<ParameterDescription> parameter_descriptions;
};

class RunVariantDescription {
    public:
        std::string exename;
        std::string brief;
        std::string detailed;
        std::vector<std::string> options;
};

class RunVariantsDescription {
    public:
        std::vector<RunVariantDescription> variants;
};

class ProgramDescription {
    public:
        std::string brief;
        std::string detailed;                
};

class EntireDescription {
    public:
        ProgramDescription program_description;
        RunVariantsDescription run_variants;
        std::vector<GroupDescription> group_descriptions;
        std::vector<ParameterDescription> parameter_descriptions;
};

#endif